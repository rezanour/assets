//=============================================================================
// libloader_obj.c - OBJ model file
// Reza Nourai, 2016
//=============================================================================

#include "..\include\libloader.h"
#include "libloader_obj.h"
#include "libloader_util.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

bool libload_obj(const char* filename, libload_obj_model_t* model)
{
    bool result = false;
    size_t buffer_size = 0;
    char* buffer = 0;
    char* buffer_end = 0;
    char* line = 0;
    char* line_end = 0;
    libload_float3_t* verts = 0;
    libload_float3_t* vert_normals = 0;
    libload_float2_t* vert_texcoords = 0;
    uint8_t* positions_base = 0;
    uint8_t* normals_base = 0;
    uint8_t* texcoords_base = 0;
    size_t num_verts = 0;
    size_t num_vert_normals = 0;
    size_t num_vert_texcoords = 0;
    char group_name[256] = { 0 };
    char smooth_group[5] = { 0 };
    size_t max_vertices = model->num_vertices;

    model->num_vertices = 0;

    // input validation
    if (model->positions && model->positions_stride == 0)
      goto cleanup;

    if (model->normals && model->normals_stride == 0)
      goto cleanup;

    if (model->texcoords && model->texcoords_stride == 0)
      goto cleanup;

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

    memset(verts, 0, buffer_size);
    memset(vert_normals, 0, buffer_size);
    memset(vert_texcoords, 0, buffer_size);

    positions_base = (uint8_t*)model->positions;
    normals_base = (uint8_t*)model->normals;
    texcoords_base = (uint8_t*)model->texcoords;

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
            sscanf_s(line + 2, "%s", group_name, 256);
        }
        else if (_strnicmp(line, "usemtl ", 7) == 0) // use material
        {
        }
        else if (_strnicmp(line, "s ", 2) == 0) // smooth shading group
        {
            sscanf_s(line + 2, "%s", smooth_group, 5);
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
              // enough room left?
              if (model->num_vertices + 3 > max_vertices)
                break;

              if (model->positions) // caller wants positions
              {
                *(libload_float3_t*)(positions_base + model->positions_stride * model->num_vertices) = verts[v[0] - 1];
                *(libload_float3_t*)(positions_base + model->positions_stride * (model->num_vertices + 1)) = verts[v[1] - 1];
                *(libload_float3_t*)(positions_base + model->positions_stride * (model->num_vertices + 2)) = verts[v[2] - 1];
              }
              if (model->normals)
              {
                *(libload_float3_t*)(normals_base + model->normals_stride * model->num_vertices) = vert_normals[vn[0] - 1];
                *(libload_float3_t*)(normals_base + model->normals_stride * (model->num_vertices + 1)) = vert_normals[vn[1] - 1];
                *(libload_float3_t*)(normals_base + model->normals_stride * (model->num_vertices + 2)) = vert_normals[vn[2] - 1];
              }
              if (model->texcoords)
              {
                *(libload_float2_t*)(texcoords_base + model->texcoords_stride * model->num_vertices) = vert_texcoords[vt[0] - 1];
                *(libload_float2_t*)(texcoords_base + model->texcoords_stride * (model->num_vertices + 1)) = vert_texcoords[vt[1] - 1];
                *(libload_float2_t*)(texcoords_base + model->texcoords_stride * (model->num_vertices + 2)) = vert_texcoords[vt[2] - 1];
              }

              model->num_vertices += 3;
            }
            else if (num_fields == 12) // quad
            {
              // enough room left?
              if (model->num_vertices + 6 > max_vertices)
                break;

              if (model->positions)
              {
                *(libload_float3_t*)(positions_base + model->positions_stride * model->num_vertices) = verts[v[0] - 1];
                *(libload_float3_t*)(positions_base + model->positions_stride * (model->num_vertices + 1)) = verts[v[1] - 1];
                *(libload_float3_t*)(positions_base + model->positions_stride * (model->num_vertices + 2)) = verts[v[2] - 1];
                *(libload_float3_t*)(positions_base + model->positions_stride * (model->num_vertices + 3)) = verts[v[0] - 1];
                *(libload_float3_t*)(positions_base + model->positions_stride * (model->num_vertices + 4)) = verts[v[2] - 1];
                *(libload_float3_t*)(positions_base + model->positions_stride * (model->num_vertices + 5)) = verts[v[3] - 1];
              }
              if (model->normals)
              {
                *(libload_float3_t*)(normals_base + model->normals_stride * model->num_vertices) = vert_normals[vn[0] - 1];
                *(libload_float3_t*)(normals_base + model->normals_stride * (model->num_vertices + 1)) = vert_normals[vn[1] - 1];
                *(libload_float3_t*)(normals_base + model->normals_stride * (model->num_vertices + 2)) = vert_normals[vn[2] - 1];
                *(libload_float3_t*)(normals_base + model->normals_stride * (model->num_vertices + 3)) = vert_normals[vn[0] - 1];
                *(libload_float3_t*)(normals_base + model->normals_stride * (model->num_vertices + 4)) = vert_normals[vn[2] - 1];
                *(libload_float3_t*)(normals_base + model->normals_stride * (model->num_vertices + 5)) = vert_normals[vn[3] - 1];
              }
              if (model->texcoords)
              {
                *(libload_float2_t*)(texcoords_base + model->texcoords_stride * model->num_vertices) = vert_texcoords[vt[0] - 1];
                *(libload_float2_t*)(texcoords_base + model->texcoords_stride * (model->num_vertices + 1)) = vert_texcoords[vt[1] - 1];
                *(libload_float2_t*)(texcoords_base + model->texcoords_stride * (model->num_vertices + 2)) = vert_texcoords[vt[2] - 1];
                *(libload_float2_t*)(texcoords_base + model->texcoords_stride * (model->num_vertices + 3)) = vert_texcoords[vt[0] - 1];
                *(libload_float2_t*)(texcoords_base + model->texcoords_stride * (model->num_vertices + 4)) = vert_texcoords[vt[2] - 1];
                *(libload_float2_t*)(texcoords_base + model->texcoords_stride * (model->num_vertices + 5)) = vert_texcoords[vt[3] - 1];
              }

              model->num_vertices += 6;
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

    result = true;

cleanup:
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
