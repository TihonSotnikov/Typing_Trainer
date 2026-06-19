add_library(
    backend STATIC
    typing_trainer_core.cpp
    typing_trainer_core.hpp
    ngram_statistics.cpp
    ngram_statistics.hpp
)

target_include_directories(
    backend PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
