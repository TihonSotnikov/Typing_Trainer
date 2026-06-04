cmake_minimum_required(VERSION 3.21)
project(BlindTypingTrainer)

set(CMAKE_CXX_STANDARD 20)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
qt_standard_project_setup()

add_subdirectory(src/main_window)

qt_add_executable(BlindTypingTrainer src/main.cpp)

# Disables console with Release configuration selected.
set_target_properties(BlindTypingTrainer PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
)

target_link_libraries(BlindTypingTrainer PRIVATE main_window Qt6::Widgets)

# Deployment: Generates a deployment script and runs it after building the target.
# Qt6 app should have necessary .dll plugins in the same directory as the executable.
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