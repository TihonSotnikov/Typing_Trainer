add_library(
    backend STATIC
    typing_trainer_core.cpp
    typing_trainer_core.hpp
    ngram_statistics.cpp
    ngram_statistics.hpp
    dictionaries.cpp
    dictionaries.hpp
    smart_text_generator.cpp
    smart_text_generator.hpp
)

target_include_directories(
    backend PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    backend PUBLIC
    nlohmann_json::nlohmann_json
)
