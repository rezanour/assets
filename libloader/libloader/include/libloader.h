//=============================================================================
// libloader.h - simple C asset loading library
// Reza Nourai, 2016
//=============================================================================
#pragma once

#include <stdint.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

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
  libload_float3_t position;
  libload_float3_t normal;
  libload_float2_t texcoord;
} libload_obj_vertex_t;

typedef struct
{
  char name[32];
  uint32_t base_index;
  uint32_t num_indices;
} libload_obj_model_part_t;

typedef struct
{
  uint32_t num_parts;
  uint32_t num_vertices;
  uint32_t num_indices;

  libload_obj_model_part_t* parts;
  libload_obj_vertex_t* vertices;
  uint32_t* indices;
} libload_obj_model_t;

bool libload_obj_load(const char* filename, libload_obj_model_t** out_model);
void libload_obj_free(libload_obj_model_t* model);

#ifdef __cplusplus
} // extern "C"
#endif
