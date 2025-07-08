qt_add_executable(cloud-tool
    src/application.cpp
    src/main.cpp
    src/main_task.cpp

    src/application.h
    src/main_task.h
)

add_dependencies(cloud-tool range-base-lib range-cloud-lib)

target_link_libraries(cloud-tool
    PRIVATE
        Qt6::Core
        Qt6::Concurrent
        Qt6::Network
        Qt6::HttpServer
        common_defines
        range-base-lib
        range-cloud-lib
)
