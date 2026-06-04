add_library(
    main_window STATIC
    main_window.cpp
    main_window.h
)

target_include_directories(main_window PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(main_window PRIVATE Qt6::Widgets)
