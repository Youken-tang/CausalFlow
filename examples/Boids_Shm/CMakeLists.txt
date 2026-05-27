#Boids
# Youken 2025.09.10
project(MagsimExample_Boids_Shm)
set(EXEC_NAME Boids_Shm)

file(GLOB DIR_${EXEC_NAME}_SRC
        CONFIGURE_DEPENDS
        "src/*.cpp"
        "main.cpp"
)
add_executable(${EXEC_NAME} ${DIR_${EXEC_NAME}_SRC})

target_include_directories(${EXEC_NAME} PRIVATE "./include")

target_link_libraries(${EXEC_NAME}
        Mgsim
)

# 链接 gperftools（如果启用）
if(GPERFTOOLS_ENABLE)
    target_link_libraries(${EXEC_NAME} ${TCMALLOC_LIB})
endif()

# 设置输出路径和 RPATH
set_target_properties(${EXEC_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${EXEC_NAME}
)
message(STATUS "MagsimExample_${EXEC_NAME} bin output: ${CMAKE_BINARY_DIR}/bin/${EXEC_NAME}")

file(GLOB ${EXEC_NAME}_DATA_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/sim_run.cfg"
        "${CMAKE_CURRENT_SOURCE_DIR}/FOM.xml"
)

if(${EXEC_NAME}_DATA_FILES)
    foreach(DATA_FILE ${${EXEC_NAME}_DATA_FILES})
        get_filename_component(FILE_NAME ${DATA_FILE} NAME)
        configure_file(${DATA_FILE} ${CMAKE_BINARY_DIR}/bin/${EXEC_NAME}/${FILE_NAME} COPYONLY)
    endforeach()
endif()

#CTEST
if(BUILD_TESTING)
    add_test(NAME ${EXEC_NAME}_test COMMAND ${EXEC_NAME})
    set_tests_properties(${EXEC_NAME}_test PROPERTIES
#            TIMEOUT 10
#            PROCESSORS 4
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${EXEC_NAME}
            REQUIRED_FILES ${CMAKE_BINARY_DIR}/bin/${EXEC_NAME}/sim_run.cfg
            REQUIRED_FILES ${CMAKE_BINARY_DIR}/bin/${EXEC_NAME}/FOM.xml
    )
endif()