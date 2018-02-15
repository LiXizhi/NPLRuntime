/* define WIRED_TERRAIN to render terrain as wires.*/
// #define WIRED_TERRAIN

/**
* define _USE_RAYTRACING_SUPPORT_ = 1 enables the ray tracing methods on the Terrain class, but increases
* memory usage considerably. Ray tracing is very fast and can be used for mouse picking,
* line-of-sight tests, etc. If you enable this option, then be aware that MUCH more memory will be needed.
*/
#define _USE_RAYTRACING_SUPPORT_ 0

/** vertices per fan*/
#define MAX_VERTICES_PER_FAN 10

#ifndef MAX_LINE
/**@def max number of characters in a single line of terrain configuration file*/
#define MAX_LINE	500
#endif

#define MAX_FILENAME_LENGTH		2048
