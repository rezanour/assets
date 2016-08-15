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
#include <math.h>

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
  char groupname[64] = {0};

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

  memset(model->vertices, 0, buffer_size);

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
      sscanf_s(line + 7, "%s", model->material_file,
        (uint32_t)LIBLOAD_ARRAYSIZE(model->material_file));
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

      //vert_texcoords[num_vert_texcoords].y = 1.f - vert_texcoords[num_vert_texcoords].y;

      ++num_vert_texcoords;
    }
    else if (_strnicmp(line, "g ", 2) == 0) // new group
    {
      sscanf_s(line + 2, "%s", groupname,
        (uint32_t)LIBLOAD_ARRAYSIZE(groupname));
    }
    else if (_strnicmp(line, "usemtl ", 7) == 0) // use material
    {
      if (current_part)
        current_part->num_indices = model->num_indices - current_part->base_index;

      current_part = &model->parts[model->num_parts++];
      current_part->base_index = model->num_indices;
      strcpy_s(current_part->name, LIBLOAD_ARRAYSIZE(current_part->name), groupname);

      sscanf_s(line + 7, "%s", current_part->material_name,
        (uint32_t)LIBLOAD_ARRAYSIZE(current_part->material_name));
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

  if (current_part)
    current_part->num_indices = model->num_indices - current_part->base_index;

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

bool libload_obj_compute_tangent_space(libload_obj_model_t* model)
{
  bool result = false;
  uint32_t i = 0;
  uint32_t* num_accum = 0;

  if (!model)
    goto cleanup;

  num_accum = (uint32_t*)malloc(sizeof(uint32_t) * model->num_vertices);
  if (!num_accum)
    goto cleanup;

  memset(num_accum, 0, sizeof(uint32_t) * model->num_vertices);

  for (i = 0; i < model->num_indices; i += 3)
  {
    libload_obj_vertex_t* v0 = &model->vertices[model->indices[i]];
    libload_obj_vertex_t* v1 = &model->vertices[model->indices[i + 1]];
    libload_obj_vertex_t* v2 = &model->vertices[model->indices[i + 2]];

    libload_float3_t e0, e1;
    libload_float2_t uv0, uv1;
    float Q;
    libload_float3_t T, B;

    e0.x = v1->position.x - v0->position.x;
    e0.y = v1->position.y - v0->position.y;
    e0.z = v1->position.z - v0->position.z;

    e1.x = v2->position.x - v0->position.x;
    e1.y = v2->position.y - v0->position.y;
    e1.z = v2->position.z - v0->position.z;

    uv0.x = v1->texcoord.x - v0->texcoord.x;
    uv0.y = v1->texcoord.y - v0->texcoord.y;
    uv1.x = v2->texcoord.x - v0->texcoord.x;
    uv1.y = v2->texcoord.y - v0->texcoord.y;

    Q = 1.f / (uv0.x * uv1.y - uv0.y * uv1.x);

    T.x = Q * (uv1.y * e0.x - uv0.y * e1.x);
    T.y = Q * (uv1.y * e0.y - uv0.y * e1.y);
    T.z = Q * (uv1.y * e0.z - uv0.y * e1.z);
    B.x = Q * (uv0.x * e1.x - uv1.x * e0.x);
    B.y = Q * (uv0.x * e1.y - uv1.x * e0.y);
    B.z = Q * (uv0.x * e1.z - uv1.x * e0.z);

    v0->tangent.x += T.x; v1->tangent.x += T.x; v2->tangent.x += T.x;
    v0->tangent.y += T.y; v1->tangent.y += T.y; v2->tangent.y += T.y;
    v0->tangent.z += T.z; v1->tangent.z += T.z; v2->tangent.z += T.z;
    v0->bitangent.x += B.x; v1->bitangent.x += B.x; v2->bitangent.x += B.x;
    v0->bitangent.y += B.y; v1->bitangent.y += B.y; v2->bitangent.y += B.y;
    v0->bitangent.z += B.z; v1->bitangent.z += B.z; v2->bitangent.z += B.z;
    ++num_accum[model->indices[i]];
    ++num_accum[model->indices[i + 1]];
    ++num_accum[model->indices[i + 2]];
  }

  // average them out
  for (i = 0; i < model->num_vertices; ++i)
  {
    if (num_accum[i] > 0)
    {
      float inv_denom = 1.f / (float)num_accum[i];
      float inv_tan_len, inv_bitan_len;

      model->vertices[i].tangent.x *= inv_denom;
      model->vertices[i].tangent.y *= inv_denom;
      model->vertices[i].tangent.z *= inv_denom;
      inv_tan_len = 1.f / sqrtf(
        model->vertices[i].tangent.x * model->vertices[i].tangent.x +
        model->vertices[i].tangent.y * model->vertices[i].tangent.y +
        model->vertices[i].tangent.z * model->vertices[i].tangent.z);
      model->vertices[i].tangent.x *= inv_tan_len;
      model->vertices[i].tangent.y *= inv_tan_len;
      model->vertices[i].tangent.z *= inv_tan_len;

      model->vertices[i].bitangent.x *= inv_denom;
      model->vertices[i].bitangent.y *= inv_denom;
      model->vertices[i].bitangent.z *= inv_denom;
      inv_bitan_len = 1.f / sqrtf(
        model->vertices[i].bitangent.x * model->vertices[i].bitangent.x +
        model->vertices[i].bitangent.y * model->vertices[i].bitangent.y +
        model->vertices[i].bitangent.z * model->vertices[i].bitangent.z);
      model->vertices[i].bitangent.x *= inv_bitan_len;
      model->vertices[i].bitangent.y *= inv_bitan_len;
      model->vertices[i].bitangent.z *= inv_bitan_len;
    }

    model->vertices[i].texcoord.y = 1.f - model->vertices[i].texcoord.y;
  }

  result = true;

cleanup:
  if (num_accum)
    free(num_accum);

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

bool libload_mtl_load(const char* filename, uint32_t* inout_num_materials, libload_mtl_t* out_materials)
{
  bool result = false;
  uint32_t buffer_size = 0;
  char* buffer = 0;
  char* buffer_end = 0;
  char* line = 0;
  char* line_end = 0;
  uint32_t max_materials = 0;
  libload_mtl_t* current_material = 0;

  // validate input
  if (!inout_num_materials)
    goto cleanup;

  if (*inout_num_materials > 0 && !out_materials)
    goto cleanup;

  max_materials = *inout_num_materials;
  *inout_num_materials = 0;

  // read the entire file into memory
  buffer = read_text_file(filename, &buffer_size);
  if (!buffer)
    goto cleanup;

  buffer_end = buffer + buffer_size;

  // start at top of buffer, and start parsing one line at a time
  line = buffer;
  while (line < buffer_end)
  {
    // trim off any leading whitespace
    while ((*line == ' ' || *line == '\t') && line < buffer_end)
      ++line;

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
    else if (_strnicmp(line, "newmtl ", 7) == 0) // material library
    {
      if (out_materials)
      {
        current_material = &out_materials[*inout_num_materials];
        sscanf_s(line + 7, "%s", current_material->name,
          (uint32_t)LIBLOAD_ARRAYSIZE(current_material->name));
      }
      ++(*inout_num_materials);
    }
    else if (_strnicmp(line, "Ns ", 3) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 3, "%f", &current_material->Ns);
      }
    }
    else if (_strnicmp(line, "Ni ", 3) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 3, "%f", &current_material->Ni);
      }
    }
    else if (_strnicmp(line, "d ", 2) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 2, "%f", &current_material->d);
      }
    }
    else if (_strnicmp(line, "Tr ", 3) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 3, "%f", &current_material->Tr);
      }
    }
    else if (_strnicmp(line, "Tf ", 3) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 3, "%f %f %f",
          &current_material->Tf.x,
          &current_material->Tf.y,
          &current_material->Tf.z);
      }
    }
    else if (_strnicmp(line, "illum ", 6) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 6, "%d", &current_material->illum_model);
      }
    }
    else if (_strnicmp(line, "Ka ", 3) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 3, "%f %f %f",
          &current_material->Ka.x,
          &current_material->Ka.y,
          &current_material->Ka.z);
      }
    }
    else if (_strnicmp(line, "Kd ", 3) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 3, "%f %f %f",
          &current_material->Kd.x,
          &current_material->Kd.y,
          &current_material->Kd.z);
      }
    }
    else if (_strnicmp(line, "Ks ", 3) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 3, "%f %f %f",
          &current_material->Ks.x,
          &current_material->Ks.y,
          &current_material->Ks.z);
      }
    }
    else if (_strnicmp(line, "Ke ", 3) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 3, "%f %f %f",
          &current_material->Ke.x,
          &current_material->Ke.y,
          &current_material->Ke.z);
      }
    }
    else if (_strnicmp(line, "map_Ka ", 7) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 7, "%s",
          current_material->map_Ka,
          (uint32_t)LIBLOAD_ARRAYSIZE(current_material->map_Ka));
      }
    }
    else if (_strnicmp(line, "map_Kd ", 7) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 7, "%s",
          current_material->map_Kd,
          (uint32_t)LIBLOAD_ARRAYSIZE(current_material->map_Kd));
      }
    }
    else if (_strnicmp(line, "map_d ", 6) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 6, "%s",
          current_material->map_d,
          (uint32_t)LIBLOAD_ARRAYSIZE(current_material->map_d));
      }
    }
    else if (_strnicmp(line, "map_bump ", 9) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 9, "%s",
          current_material->map_bump,
          (uint32_t)LIBLOAD_ARRAYSIZE(current_material->map_bump));
      }
    }
    else if (_strnicmp(line, "bump ", 5) == 0) // 
    {
      if (current_material)
      {
        sscanf_s(line + 5, "%s",
          current_material->bump,
          (uint32_t)LIBLOAD_ARRAYSIZE(current_material->bump));
      }
    }

    line = line_end + 1;
  }

  result = true;

cleanup:
  if (buffer)
    free(buffer);

  return result;
}
