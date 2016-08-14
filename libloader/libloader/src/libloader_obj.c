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

bool libload_obj(const char* filename, /*inout*/ libload_model_t* model)
{
    bool result = false;
    char* buffer = 0;
    char* buffer_end = 0;
    char* line = 0;
    char* line_end = 0;
    size_t buffer_size = 0;
    float* verts = 0;
    float* vert_normals = 0;
    float* vert_texcoords = 0;
    size_t num_verts = 0;
    size_t num_vert_normals = 0;
    size_t num_vert_texcoords = 0;
    char group_name[256] = { 0 };
    char smooth_group[5] = { 0 };
    size_t max_vertices = model->num_vertices;

    if (model->positions && model->positions_stride == 0)
      goto cleanup;

    if (model->normals && model->normals_stride == 0)
      goto cleanup;

    if (model->texcoords && model->texcoords_stride == 0)
      goto cleanup;

    buffer = read_text_file(filename, &buffer_size);
    if (!buffer)
      goto cleanup;

    buffer_end = buffer + buffer_size;

    // allocate buffers for holding values conservatively (as if the whole file is just this)
    verts = (float*)malloc(buffer_size / sizeof(float));
    if (!verts)
        goto cleanup;

    vert_normals = (float*)malloc(buffer_size / sizeof(float));
    if (!vert_normals)
        goto cleanup;

    vert_texcoords = (float*)malloc(buffer_size / sizeof(float));
    if (!vert_texcoords)
        goto cleanup;

    line = buffer;
    model->num_vertices = 0;
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
            num_verts += sscanf_s(line + 2, "%f %f %f", &verts[num_verts], &verts[num_verts + 1], &verts[num_verts + 2]);
        }
        else if (_strnicmp(line, "vn ", 3) == 0) // vertex normals
        {
            num_vert_normals += sscanf_s(line + 3, "%f %f %f", &vert_normals[num_vert_normals], &vert_normals[num_vert_normals + 1], &vert_normals[num_vert_normals + 2]);
        }
        else if (_strnicmp(line, "vt ", 3) == 0) // vertex tex coords
        {
            num_vert_texcoords += sscanf_s(line + 3, "%f %f %f", &vert_texcoords[num_vert_texcoords], &vert_texcoords[num_vert_texcoords + 1], &vert_texcoords[num_vert_texcoords + 2]);
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
            int num_fields = sscanf_s(line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                &v[0], &vt[0], &vn[0],
                &v[1], &vt[1], &vn[1],
                &v[2], &vt[2], &vn[2],
                &v[3], &vt[3], &vn[3]);

            if (num_fields < 10)
            {
              // enough room left?
              if (model->num_vertices + 3 > max_vertices)
                break;

              // single triangle
              if (model->positions)
              {
                float* positions = (float*)((uint8_t*)model->positions + model->positions_stride * model->num_vertices);
                positions[0] = verts[v[0]];
                positions[1] = verts[v[1]];
                positions[2] = verts[v[2]];
              }
              if (model->texcoords)
              {
                float* texcoords = (float*)((uint8_t*)model->texcoords + model->texcoords_stride * model->num_vertices);
                texcoords[0] = vert_texcoords[vt[0]];
                texcoords[1] = vert_texcoords[vt[1]];
                texcoords[2] = vert_texcoords[vt[2]];
              }
              if (model->normals)
              {
                float* normals = (float*)((uint8_t*)model->normals + model->normals_stride * model->num_vertices);
                normals[0] = vert_normals[vn[0]];
                normals[1] = vert_normals[vn[1]];
                normals[2] = vert_normals[vn[2]];
              }

              model->num_vertices += 3;
            }
            else
            {
              // enough room left?
              if (model->num_vertices + 6 > max_vertices)
                break;

              // quad
              if (model->positions)
              {
                float* positions = (float*)((uint8_t*)model->positions + model->positions_stride * model->num_vertices);
                positions[0] = verts[v[0]];
                positions[1] = verts[v[1]];
                positions[2] = verts[v[2]];
                positions[3] = verts[v[0]];
                positions[4] = verts[v[2]];
                positions[5] = verts[v[3]];
              }
              if (model->texcoords)
              {
                float* texcoords = (float*)((uint8_t*)model->texcoords + model->texcoords_stride * model->num_vertices);
                texcoords[0] = vert_texcoords[vt[0]];
                texcoords[1] = vert_texcoords[vt[1]];
                texcoords[2] = vert_texcoords[vt[2]];
                texcoords[3] = vert_texcoords[vt[0]];
                texcoords[4] = vert_texcoords[vt[2]];
                texcoords[5] = vert_texcoords[vt[3]];
              }
              if (model->normals)
              {
                float* normals = (float*)((uint8_t*)model->normals + model->normals_stride * model->num_vertices);
                normals[0] = vert_normals[vn[0]];
                normals[1] = vert_normals[vn[1]];
                normals[2] = vert_normals[vn[2]];
                normals[3] = vert_normals[vn[0]];
                normals[4] = vert_normals[vn[2]];
                normals[5] = vert_normals[vn[3]];
              }

              model->num_vertices += 6;
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
