#pragma once

#if ENABLE_UNIT_TESTS
    #define SET_EMSCRIPTEN_CALLBACK(func, ...)
#else
    #define SET_EMSCRIPTEN_CALLBACK(func, ...) func(__VA_ARGS__)
#endif
