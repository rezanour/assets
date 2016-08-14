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

//=============================================================================
// general type definitions
//=============================================================================

typedef struct
{
  float x, y;
} libload_float2_t;

typedef struct
{
  float x, y, z;
} libload_float3_t;

//=============================================================================
// support for OBJ model files
//=============================================================================

typedef struct
{
  // in: max capacity of vertices in streams
  // out: number of vertices filled in
  uint32_t num_vertices;

  // in_opt: head of positions stream to fill in
  libload_float3_t* positions;

  // in_opt: head of normals stream to fill in
  libload_float3_t* normals;

  // in_opt: head of texcoords stream to fill in
  libload_float2_t* texcoords;

  // in: if positions is non-null, number of bytes between 2 consecutive
  // position values in the stream.
  uint32_t positions_stride;

  // in: if normals is non-null, number of bytes between 2 consecutive
  // normal values in the stream.
  uint32_t normals_stride;

  // in: if texcoords is non-null, number of bytes between 2 consecutive
  // texcoord values in the stream.
  uint32_t texcoords_stride;
} libload_obj_model_t;

bool libload_obj(const char* filename, libload_obj_model_t* model);

#ifdef __cplusplus
} // extern "C"
#endif
