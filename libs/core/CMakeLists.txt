set(TONE_SOURCES "")

set(PREFIX_I "${CMAKE_CURRENT_LIST_DIR}/include/tone")
set(PREFIX_S "${CMAKE_CURRENT_LIST_DIR}/src/tone")

list(APPEND TONE_SOURCES "${PREFIX_I}/core.hpp" "${PREFIX_S}/core.cpp")
list(APPEND TONE_SOURCES "${PREFIX_I}/core/lookup.hpp" "${PREFIX_I}/core/lookup.inl")
list(APPEND TONE_SOURCES "${PREFIX_I}/core/character.hpp")
list(APPEND TONE_SOURCES "${PREFIX_I}/core/push_back_stream.hpp" "${PREFIX_S}/core/push_back_stream.cpp")
list(APPEND TONE_SOURCES "${PREFIX_I}/core/tokens.hpp" "${PREFIX_S}/core/tokens.cpp")
list(APPEND TONE_SOURCES "${PREFIX_I}/core/tokenize.hpp" "${PREFIX_S}/core/tokenize.cpp")
list(APPEND TONE_SOURCES "${PREFIX_I}/core/errors.hpp" "${PREFIX_S}/core/errors.cpp")

unset(PREFIX_I)
unset(PREFIX_S)

add_library(tone_core STATIC ${TONE_SOURCES})
target_include_directories(tone_core PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(tone_core PUBLIC fmt::fmt-header-only)
target_compile_features(tone_core PUBLIC cxx_std_20)
set_target_properties(tone_core PROPERTIES CXX_EXTENSIONS OFF)


add_subdirectory(examples)