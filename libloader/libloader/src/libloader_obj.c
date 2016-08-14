//=============================================================================
// libloader_obj.c - OBJ model file
// Reza Nourai, 2016
//=============================================================================

#include "..\include\libloader.h"
#include "libloader_util.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

bool libload_obj_load(const char* filename, libload_obj_model_t** out_model)
{
  bool result = false;
  uint32_t buffer_size = 0;
  char* buffer = 0;
  char* buffer_end = 0;
  char* line = 0;
  char* line_end = 0;
  libload_float3_t* verts = 0;
  libload_float3_t* vert_normals = 0;
  libload_float2_t* vert_texcoords = 0;
  uint32_t num_verts = 0;
  uint32_t num_vert_normals = 0;
  uint32_t num_vert_texcoords = 0;
  libload_obj_model_t* model = 0;
  libload_obj_model_part_t* current_part = 0;
  keyvalue_pair_t* vertex_map = 0;
  uint32_t map_size = 0;
  uint32_t map_max = 0;

  // read the entire file into memory
  buffer = read_text_file(filename, &buffer_size);
  if (!buffer)
    goto cleanup;

  buffer_end = buffer + buffer_size;

  // allocate buffers for holding values conservatively
  // as if the whole file is just this type of value
  verts = (libload_float3_t*)malloc(buffer_size);
  if (!verts)
    goto cleanup;

  vert_normals = (libload_float3_t*)malloc(buffer_size);
  if (!vert_normals)
    goto cleanup;

  vert_texcoords = (libload_float2_t*)malloc(buffer_size);
  if (!vert_texcoords)
    goto cleanup;

  // allocate the model struct
  model = (libload_obj_model_t*)malloc(sizeof(libload_obj_model_t));
  if (!model)
    goto cleanup;

  memset(model, 0, sizeof(libload_obj_model_t));

  model->vertices = (libload_obj_vertex_t*)malloc(buffer_size);
  if (!model->vertices)
    goto cleanup;

  model->indices = (uint32_t*)malloc(buffer_size);
  if (!model->indices)
    goto cleanup;

  model->parts = (libload_obj_model_part_t*)malloc(buffer_size);
  if (!model->parts)
    goto cleanup;

  // allocate the vertex map for binning (to build minimal verts & good index list)
  vertex_map = (keyvalue_pair_t*)malloc(buffer_size);
  if (!vertex_map)
    goto cleanup;

  map_max = buffer_size / sizeof(keyvalue_pair_t);

  // start at top of buffer, and start parsing one line at a time
  line = buffer;
  while (line < buffer_end)
  {
    // read one line in by finding the newline & turning into \0
    line_end = line;
    while (line_end < buffer_end && *line_end != '\n')
      ++line_end;

    if (line_end < buffer_end)
      *line_end = '\0';

    // handle line
    if (*line == '#') // comment
    {
    }
    else if (_strnicmp(line, "mtllib ", 7) == 0) // material library
    {
    }
    else if (_strnicmp(line, "v ", 2) == 0) // vertex
    {
      if (sscanf_s(line + 2, "%f %f %f",
        &verts[num_verts].x,
        &verts[num_verts].y,
        &verts[num_verts].z) != 3)
        goto cleanup;

      ++num_verts;
    }
    else if (_strnicmp(line, "vn ", 3) == 0) // vertex normals
    {
      if (sscanf_s(line + 3, "%f %f %f",
        &vert_normals[num_vert_normals].x,
        &vert_normals[num_vert_normals].y,
        &vert_normals[num_vert_normals].z) != 3)
        goto cleanup;

      ++num_vert_normals;
    }
    else if (_strnicmp(line, "vt ", 3) == 0) // vertex tex coords
    {
      if (sscanf_s(line + 3, "%f %f",
        &vert_texcoords[num_vert_texcoords].x,
        &vert_texcoords[num_vert_texcoords].y) != 2)
        goto cleanup;

      ++num_vert_texcoords;
    }
    else if (_strnicmp(line, "g ", 2) == 0) // new group
    {
      if (current_part)
        current_part->num_indices = model->num_indices - current_part->base_index;

      current_part = &model->parts[model->num_parts++];
      current_part->base_index = model->num_indices;
      sscanf_s(line + 2, "%s", current_part->name, (uint32_t)(sizeof(current_part->name) / sizeof(current_part->name[0])));
    }
    else if (_strnicmp(line, "usemtl ", 7) == 0) // use material
    {
    }
    else if (_strnicmp(line, "s ", 2) == 0) // smooth shading group
    {
      // not used when vertex normals present
    }
    else if (_strnicmp(line, "f ", 2) == 0) // face definition
    {
      int v[4], vn[4], vt[4];
      int num_fields = sscanf_s(line + 2,
        "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
        &v[0], &vt[0], &vn[0],
        &v[1], &vt[1], &vn[1],
        &v[2], &vt[2], &vn[2],
        &v[3], &vt[3], &vn[3]);

      if (num_fields == 9)  // single triangle
      {
        for (int i = 0; i < 3; ++i)
        {
          assert(((v[i] - 1) & 0xFFFFF) == (v[i] - 1)); // make sure value doesn't exceed key mask
          assert(((vn[i] - 1) & 0xFFFFF) == (vn[i] - 1)); // make sure value doesn't exceed key mask
          assert(((vt[i] - 1) & 0xFFFFF) == (vt[i] - 1)); // make sure value doesn't exceed key mask

          uint32_t index = 0;
          uint64_t key =
            ((uint64_t)((v[i] - 1) & 0xFFFFF) << 40) |
            ((uint64_t)((vn[i] - 1) & 0xFFFFF) << 20) |
            ((uint64_t)((vt[i] - 1) & 0xFFFFF));

          if (!keyvalue_find(vertex_map, map_size, key, &index))
          {
            // insert new vertex
            model->vertices[model->num_vertices].position = verts[v[i] - 1];
            model->vertices[model->num_vertices].normal = vert_normals[vn[i] - 1];
            model->vertices[model->num_vertices].texcoord = vert_texcoords[vt[i] - 1];
            model->indices[model->num_indices] = model->num_vertices;
            keyvalue_insert(vertex_map, &map_size, map_max, key, model->num_vertices);
            ++model->num_vertices;
            ++model->num_indices;
          }
          else
          {
            model->indices[model->num_indices++] = index;
          }
        }
      }
      else if (num_fields == 12) // quad
      {
        uint32_t order[] = { 0, 1, 2, 0, 2, 3 };
        for (int i = 0; i < 6; ++i)
        {
          int j = order[i];
          assert(((v[j] - 1) & 0xFFFFF) == (v[j] - 1)); // make sure value doesn't exceed key mask
          assert(((vn[j] - 1) & 0xFFFFF) == (vn[j] - 1)); // make sure value doesn't exceed key mask
          assert(((vt[j] - 1) & 0xFFFFF) == (vt[j] - 1)); // make sure value doesn't exceed key mask

          uint32_t index = 0;
          uint64_t key =
            ((uint64_t)((v[j] - 1) & 0xFFFFF) << 40) |
            ((uint64_t)((vn[j] - 1) & 0xFFFFF) << 20) |
            ((uint64_t)((vt[j] - 1) & 0xFFFFF));

          if (!keyvalue_find(vertex_map, map_size, key, &index))
          {
            // insert new vertex
            model->vertices[model->num_vertices].position = verts[v[j] - 1];
            model->vertices[model->num_vertices].normal = vert_normals[vn[j] - 1];
            model->vertices[model->num_vertices].texcoord = vert_texcoords[vt[j] - 1];
            model->indices[model->num_indices] = model->num_vertices;
            keyvalue_insert(vertex_map, &map_size, map_max, key, model->num_vertices);
            ++model->num_vertices;
            ++model->num_indices;
          }
          else
          {
            model->indices[model->num_indices++] = index;
          }
        }
      }
      else
      {
        // can have larger polys than quad?
        assert(false);
        goto cleanup;
      }
    }

    line = line_end + 1;
  }

  *out_model = model;
  model = 0;
  result = true;

cleanup:
  libload_obj_free(model);

  if (vertex_map)
    free(vertex_map);
  if (vert_texcoords)
    free(vert_texcoords);
  if (vert_normals)
    free(vert_normals);
  if (verts)
    free(verts);
  if (buffer)
    free(buffer);

  return result;
}

void libload_obj_free(libload_obj_model_t* model)
{
  if (model)
  {
    if (model->vertices)
      free(model->vertices);
    if (model->indices)
      free(model->indices);
    if (model->parts)
      free(model->parts);
    free(model);
  }
}
