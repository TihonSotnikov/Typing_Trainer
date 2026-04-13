cmake_minimum_required(VERSION 3.21)
project(BlindTypingTrainer)

# set(X_VCPKG_APPLOCAL_DEPS OFF CACHE BOOL "")
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
qt_standard_project_setup()

add_subdirectory(source/main_window)

qt_add_executable(BlindTypingTrainer source/main.cpp)
target_link_libraries(BlindTypingTrainer PRIVATE main_window Qt6::Widgets)

qt_generate_deploy_app_script(
    TARGET BlindTypingTrainer
    OUTPUT_SCRIPT deploy_script.cmake
)
add_custom_command(TARGET BlindTypingTrainer POST_BUILD
    COMMAND ${CMAKE_COMMAND} 
            -D "DEPLOY_TARGET=$<TARGET_FILE:BlindTypingTrainer>"
            -D "QT_DEPLOY_PREFIX=$<TARGET_FILE_DIR:BlindTypingTrainer>"
            -D "QT_DEPLOY_BIN_DIR=."
            -P "${deploy_script.cmake}"
    COMMENT "Running deployment for BlindTypingTrainer..."
    VERBATIM
)