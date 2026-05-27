#include "Simulator.h"
#include "BirdShm.h"

#include <iostream>
#include <boost/chrono.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <cassert>
#include <string>
#include <algorithm>  

using namespace std;

double Length;
double Width;
double Height;

using Bird = BirdShm;
  
// 定义日志类  
class Logger {  
private:  
    std::ofstream logFile;  
    std::string logFileName;  
  
public:  
    // 构造函数，初始化日志文件名  
    Logger(const std::string& fileName) : logFileName(fileName) {  
        logFile.open(logFileName, std::ios::out);  
        if (!logFile.is_open()) {  
            std::cerr << "无法打开日志文件 " << logFileName << std::endl;  
        }  
    }  
  
    // 析构函数，关闭日志文件  
    ~Logger() {  
        if (logFile.is_open()) {  
            logFile.close();  
        }  
    }  
  
    // 写入日志信息  
    void writeLog(const std::string& message) {  
        if (logFile.is_open()) {  
            //std::time_t currentTime = std::time(nullptr);  
            //std::tm* localTime = std::localtime(&currentTime);  
            //char timeStr[256];  
            //std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localTime);  
            logFile << message ;  
        }  
    }  
}; 

template<typename T>
class BoidsSimulator : public Simulator
{
protected:
	Logger logger; 
	long numBirds; 
	

public:
	BoidsSimulator():logger("Trace of Birds.csv") {}
	virtual long ParseScenario();
	virtual ~BoidsSimulator() {}

	void createLPsState()
	{
		for( auto i = 0; i < getNumofLPs(); i++ )
		{
			T* intralpst = new T();
			T* odd = new T();
			T* even = new T();

			addLPSharedState(i, intralpst, odd, even);
		}
	}

    //string genMsgfrominsimdata(string insimdata, SimMsg*& msg) { return "Hello World"; }
    //virtual string genMsgTypefrominsimdata(string insimdata) = 0;
    void collect_statistics(SimTime glbts);
	void setNumBirds(long num){ numBirds = num; }
};

template<typename T>
long BoidsSimulator<T>::ParseScenario() 
{

	for( auto i = 0; i < numBirds; i++ )
	{
		Vector3D pos(drand48()*Length, drand48()*Width, drand48()*Height);
		Vector3D vec( drand48()*3 - 1.5, drand48()*3 - 1.5, drand48()*3 - 1.5 );
		
		Bird* pbird = new Bird(pos, vec, 20, 15);
		pbird->SetEntityID(i);
		
		add_simentity(pbird, 0);
	}
	
	return numBirds;
}

template<typename T>
void BoidsSimulator<T>::collect_statistics(SimTime glbts)
{
	auto fraction = glbts.GetTime() - std::floor( glbts.GetTime() );

	if( abs( fraction - 0.1) > 0.001 )
		return;

	logger.writeLog( std::to_string(glbts.GetTime()) );
	logger.writeLog( string(",") );
	for( auto id = 0; id < getNumofEntities(); id++ )
	{
		SimEntity* ent = get_simentity(id);
		auto pb = dynamic_cast<Bird*>(ent);

		logger.writeLog( std::to_string( pb->getPosition().x ) );
		logger.writeLog( string(",") );
		logger.writeLog( std::to_string( pb->getPosition().y ) );
		logger.writeLog( string(",") );
	}
	logger.writeLog("\n");
}

/*void musim::free_event(void*, int)
{
}*/

namespace bpo = boost::program_options;

int main(int ac, char *av[])
{
	srand(time(0));

	// 解析命令行参数
	bpo::options_description desc("boids simulation args!");
	desc.add_options()
	("help", "show help info")
	("num", bpo::value<long>()->default_value(10000), "the number of birds!")
	("length", bpo::value<double>()->default_value(1000), "the length of the space!")
	("width", bpo::value<double>()->default_value(1000), "the width of the space!")
	("height", bpo::value<double>()->default_value(50), "the height of the space!");

	bpo::variables_map vm;
	store( parse_command_line(ac, av, desc), vm );
	notify(vm);

	try{
		if( vm.count("help") ){
			cout << desc << endl;
			return 0;
		}
		if ( vm.count("num") )
		{
			cout << "num = " << vm["num"].as<long>() << std::endl;
		}else if ( vm.count("length") )
		{
			cout << "length = " << vm["length"].as<double>() << std::endl;
		}else if ( vm.count("width") )
		{
			cout << "width = " << vm["width"].as<double>() << std::endl;
		}else if ( vm.count("height") )
		{
			cout << "height = " << vm["height"].as<double>() << std::endl;
		}
	}catch (const std::exception &ex){
		std::cerr << ex.what() << std::endl;
	}

	// 初始化仿真器
	BoidsSimulator<BoidsState> *psim = new BoidsSimulator<BoidsState>();
	
	// 基于命令行参数设置飞鸟数量、飞行空间的长、宽、高
	psim->setNumBirds(vm["num"].as<long>());
	Length = vm["length"].as<double>();
	Width = vm["width"].as<double>();
	Height = vm["height"].as<double>();

	psim->sim_pre_init(ac, av);
	psim->createLPsState();	// 为每个LP生成相应的存储区。
	psim->sim_init();
	psim->sim_run();
	psim->stop();

	return 0;
}



//!< 将共享存储区域加入到仿真中
	/*for( auto i = 0; i < psim->getNumofLPs(); i++ )
	{
		BoidsState* intralpst = new BoidsState();
		BoidsState* odd = new BoidsState();
		BoidsState* even = new BoidsState();

		psim->addLPSharedState(i, intralpst, odd, even);
	}*/

