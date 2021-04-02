#include <stdio.h>

#define TRACE_LOG(...) \
            do { if (TRACE) { fprintf(stderr, "DEBUG: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } } while (0)
#define DEBUG_LOG(...) \
            do { if (DEBUG) { fprintf(stderr, "DEBUG: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } } while (0)
#define ERROR_LOG(...) \
            do { fprintf(stderr, "ERROR: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#define DEBUG 1
#define TRACE 0
