add_library(
    frontend STATIC
    main_window.cpp
    main_window.hpp
    typing_trainer_adapter.cpp
    typing_trainer_adapter.hpp
)

target_include_directories(
    frontend PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    frontend PRIVATE
    Qt6::Widgets
    backend
)
