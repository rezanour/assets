//=============================================================================
// libloader.h - simple C asset loading library
// Reza Nourai, 2016
//=============================================================================
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    size_t num_triangles;
    float* positions;
    float* normals;
    float* texcoords;
} libload_model_t;

// load OBJ model.
// On input, model should contain valid pointers to streams the caller wants
// filled. For example, if 'float* positions' is non-null, positions will be
// filled in for each vertex.
// Also, on input, num_verts stores the maximum number of verts the streams
// can hold.
bool libload_obj(const char* filename, /*inout*/ libload_model_t* model);

#ifdef __cplusplus
} // extern "C"
#endif
