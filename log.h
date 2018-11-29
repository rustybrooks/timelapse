#pragma once

#ifdef _WIN32
#define debug(x, ...) fprintf(stderr, x, __VA_ARGS__); fflush(stderr)
#else
#define debug(x, args...) fprintf(stderr, x, ## args); fflush(stderr)
#endif
