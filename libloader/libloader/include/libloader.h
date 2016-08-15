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
  libload_float3_t tangent;
  libload_float3_t bitangent;
  libload_float2_t texcoord;
} libload_obj_vertex_t;

typedef struct
{
  char name[64];
  char material_name[64];
  uint32_t base_index;
  uint32_t num_indices;
} libload_obj_model_part_t;

typedef struct
{
  char material_file[512];

  uint32_t num_parts;
  uint32_t num_vertices;
  uint32_t num_indices;

  libload_obj_model_part_t* parts;
  libload_obj_vertex_t* vertices;
  uint32_t* indices;
} libload_obj_model_t;

bool libload_obj_load(const char* filename, libload_obj_model_t** out_model);
bool libload_obj_compute_tangent_space(libload_obj_model_t* model);
void libload_obj_free(libload_obj_model_t* model);

typedef struct
{
  char name[64];        // name
  float Ns;             // specular exponent
  float Ni;             // optical density
  float d;              // dissolve (aka. alpha)
  float Tr;             // Unused
  libload_float3_t Tf;  // transmissive
  int illum_model;      // illumination model
  libload_float3_t Ka;  // ambient
  libload_float3_t Kd;  // diffuse
  libload_float3_t Ks;  // specular
  libload_float3_t Ke;  // emmisive
  char map_Ka[512];     // ambient map
  char map_Kd[512];     // diffuse map
  char map_d[512];      // dissolve map
  char map_bump[512];   // bump map
  char bump[512];       // bump map
} libload_mtl_t;

// on input, inout_num_materials should contain the capacity of the out_materials array.
// on output, inout_num_materials will contain the actual number of materials filled in.
// can pass in 0 for num_materials and leave out_materials null to compute the number of materials needed.
bool libload_mtl_load(const char* filename, uint32_t* inout_num_materials, libload_mtl_t* out_materials);

#ifdef __cplusplus
} // extern "C"
#endif
