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
  uint32_t num_vertices;
  float* positions;
  float* normals;
  float* texcoords;
  uint32_t positions_stride;
  uint32_t normals_stride;
  uint32_t texcoords_stride;
} libload_model_t;

// load OBJ model.
// On input, model should contain valid pointers to streams the caller wants
// filled. For example, if 'float* positions' is non-null, positions will be
// filled in for each vertex. Also, on input, num_verts should be set to
// the maximum number of verts the streams can hold.
bool libload_obj(const char* filename, /*inout*/ libload_model_t* model);

#ifdef __cplusplus
} // extern "C"
#endif
