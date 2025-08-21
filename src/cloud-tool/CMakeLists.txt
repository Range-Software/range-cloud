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
        range-cloud-lib
        common_defines
)
