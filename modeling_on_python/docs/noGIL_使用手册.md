# Causal noGIL 使用手册

本发布包提供的是 Python free-threaded 版本，适用于 `python3.14t`。文件名 `causal.cpython-314t-x86_64-linux-gnu.so` 中的 `314t` 表示 Python 3.14 free-threaded ABI。

## 运行前检查

```bash
python3.14t --version
python3.14t -c "import sys; print(sys._is_gil_enabled())"
```

第二条命令期望输出 `False`。如果输出为 `True`，说明当前解释器不是 free-threaded 模式，不能用于验证 noGIL 运行。

## 导入验证

从 `causal_open_source/` 根目录运行：

```bash
export PYTHONPATH=$PWD/python:$PYTHONPATH
python3.14t - <<'PY'
import sys
import causal

print("module:", causal.__name__, causal.__version__)
print("GIL enabled:", sys._is_gil_enabled())
print("SimTime:", causal.SimTime(1.0).GetTime())
PY
```

期望 `GIL enabled` 为 `False`。该扩展模块使用 pybind11 的 noGIL 模块声明构建，导入后不应强制重新启用 GIL。

## 运行示例

```bash
cd causal_open_source
export PYTHONPATH=$PWD/python:$PYTHONPATH

cd examples/phold_python
python3.14t main.py --num 100 --strategy 0
```

## 编程注意事项

- noGIL 不等于 Python 业务代码自动线程安全。
- 多线程仿真中避免共享可变全局状态。
- 随机数建议使用 per-entity RNG，例如为每个实体创建独立的 `random.Random(seed)`。
- 多实体共享数据需要显式同步。
- Python 回调越频繁，C++/Python 边界开销越明显；热点逻辑建议尽量批处理。

## 常见问题

### ImportError 或 ABI 不匹配

确认使用的是 `python3.14t`，并且 `PYTHONPATH` 指向 `causal_open_source/python`。不要通过重命名 `.so` 绕过 ABI 标签。

### 导入后 GIL 被启用

确认解释器是 free-threaded Python，并用 `sys._is_gil_enabled()` 在导入前后分别检查。如果导入前已经是 `True`，问题来自解释器环境而不是 Causal 模块。
