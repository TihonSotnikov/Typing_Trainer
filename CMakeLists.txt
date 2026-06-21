cmake_minimum_required(VERSION 3.21)

set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
set(CMAKE_INSTALL_BINDIR .)
set(CMAKE_AUTORCC ON)
project(BlindTypingTrainer)

list(APPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows")
set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options(/utf-8)
endif()

find_package(nlohmann_json 3.12.0 REQUIRED)
find_package(
    Qt6 COMPONENTS Core Qml Quick Widgets QuickControls2 QuickEffects REQUIRED)
qt_standard_project_setup()

qt_policy(SET QTP0001 NEW) # Устанавливает стандартный префикс ресурса в ":/qt/qml/"
qt_policy(SET QTP0004 NEW) # Включает строгую проверку структуры каталогов QML

add_subdirectory(src)
add_subdirectory(src/frontend)
add_subdirectory(src/backend)

qt_add_executable(BlindTypingTrainer
    src/main.cpp
    app_icon.rc
    resources.qrc
)

set_source_files_properties(src/frontend/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(BlindTypingTrainer
    URI BlindTypingTrainerModule
    VERSION 1.0
    QML_FILES
        src/frontend/main.qml
        src/frontend/Theme.qml
        src/frontend/elements/XButton.qml
        src/frontend/elements/XSegmentedControl.qml
)

# Disables console with Release configuration selected.
set_target_properties(BlindTypingTrainer PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
)

target_link_libraries(
    BlindTypingTrainer PRIVATE
    frontend
    backend
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickEffects
)

# Deployment: Generates a deployment script and runs it after building the target.
# Qt6 app should have necessary .dll plugins in the same directory as the executable.
if(WIN32)
    qt_generate_deploy_qml_app_script(
        TARGET BlindTypingTrainer
        OUTPUT_SCRIPT deploy_script.cmake
    )
    
    install(TARGETS BlindTypingTrainer
        BUNDLE DESTINATION .
        RUNTIME DESTINATION .
    )
    
    # # Выполняем скрипт деплоя ТОЛЬКО во время процесса инсталляции
    install(SCRIPT ${deploy_script.cmake})

    # add_custom_command(TARGET BlindTypingTrainer POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} 
    #             -D "DEPLOY_TARGET=$<TARGET_FILE:BlindTypingTrainer>"
    #             -D "QT_DEPLOY_PREFIX=$<TARGET_FILE_DIR:BlindTypingTrainer>"
    #             -D "QT_DEPLOY_BIN_DIR=."
    #             -P "${deploy_script.cmake}"
    #     COMMENT "Running deployment for BlindTypingTrainer..."
    #     VERBATIM
    # )
endif()
