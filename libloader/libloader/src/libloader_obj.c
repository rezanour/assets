//=============================================================================
// libloader_obj.c - OBJ model file
// Reza Nourai, 2016
//=============================================================================

#include "..\include\libloader.h"
#include "libloader_obj.h"

#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

bool libload_obj(const char* filename, /*inout*/ libload_model_t* model)
{
    bool result = false;
    FILE* file = 0;
    errno_t err = 0;
    fpos_t pos = 0;
    char* buffer = 0;
    char* buffer_end = 0;
    char* line = 0;
    char* line_end = 0;
    size_t bytes_read = 0;
    float* verts = 0;
    float* vert_normals = 0;
    float* vert_texcoords = 0;
    size_t num_verts = 0;
    size_t num_vert_normals = 0;
    size_t num_vert_texcoords = 0;
    char group_name[256] = { 0 };
    char smooth_group[5] = { 0 };
    size_t max_triangles = model->num_triangles;

    err = fopen_s(&file, filename, "rt");
    if (err != 0)
        goto cleanup;

    if (fseek(file, 0, SEEK_END) != 0)
        goto cleanup;

    if (fgetpos(file, &pos) != 0)
        goto cleanup;

    if (fseek(file, 0, SEEK_SET) != 0)
        goto cleanup;

    buffer = (char*)malloc(pos);
    if (!buffer)
        goto cleanup;

    buffer_end = buffer + pos;

    bytes_read = fread_s(buffer, pos, 1, pos, file);
    if (bytes_read == 0)
        goto cleanup;

    // don't need the file handle anymore
    fclose(file);
    file = 0;

    // allocate buffers for holding values conservatively (as if the whole file is just this)
    verts = (float*)malloc(pos / sizeof(float));
    if (!verts)
        goto cleanup;

    vert_normals = (float*)malloc(pos / sizeof(float));
    if (!vert_normals)
        goto cleanup;

    vert_texcoords = (float*)malloc(pos / sizeof(float));
    if (!vert_texcoords)
        goto cleanup;

    line = buffer;
    model->num_triangles = 0;
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
                // single triangle
                if (model->positions)
                {
                    model->positions[model->num_triangles * 3] = verts[v[0]];
                    model->positions[model->num_triangles * 3 + 1] = verts[v[1]];
                    model->positions[model->num_triangles * 3 + 2] = verts[v[2]];
                }
                if (model->texcoords)
                {
                    model->texcoords[model->num_triangles * 3] = vert_texcoords[vt[0]];
                    model->texcoords[model->num_triangles * 3 + 1] = vert_texcoords[vt[1]];
                    model->texcoords[model->num_triangles * 3 + 2] = vert_texcoords[vt[2]];
                }
                if (model->normals)
                {
                    model->normals[model->num_triangles * 3] = vert_normals[vn[0]];
                    model->normals[model->num_triangles * 3 + 1] = vert_normals[vn[1]];
                    model->normals[model->num_triangles * 3 + 2] = vert_normals[vn[2]];
                }

                ++model->num_triangles;
                if (model->num_triangles == max_triangles)
                    break;
            }
            else
            {
                // quad
                if (model->positions)
                {
                    model->positions[model->num_triangles * 3] = verts[v[0]];
                    model->positions[model->num_triangles * 3 + 1] = verts[v[1]];
                    model->positions[model->num_triangles * 3 + 2] = verts[v[2]];

                    if (model->num_triangles + 1 < max_triangles)
                    {
                        model->positions[model->num_triangles * 3 + 3] = verts[v[0]];
                        model->positions[model->num_triangles * 3 + 4] = verts[v[2]];
                        model->positions[model->num_triangles * 3 + 5] = verts[v[3]];
                    }
                }
                if (model->texcoords)
                {
                    model->texcoords[model->num_triangles * 3] = vert_texcoords[vt[0]];
                    model->texcoords[model->num_triangles * 3 + 1] = vert_texcoords[vt[1]];
                    model->texcoords[model->num_triangles * 3 + 2] = vert_texcoords[vt[2]];

                    if (model->num_triangles + 1 < max_triangles)
                    {
                        model->texcoords[model->num_triangles * 3 + 3] = vert_texcoords[vt[0]];
                        model->texcoords[model->num_triangles * 3 + 4] = vert_texcoords[vt[2]];
                        model->texcoords[model->num_triangles * 3 + 5] = vert_texcoords[vt[3]];
                    }
                }
                if (model->normals)
                {
                    model->normals[model->num_triangles * 3] = vert_normals[vn[0]];
                    model->normals[model->num_triangles * 3 + 1] = vert_normals[vn[1]];
                    model->normals[model->num_triangles * 3 + 2] = vert_normals[vn[2]];

                    if (model->num_triangles + 1 < max_triangles)
                    {
                        model->normals[model->num_triangles * 3 + 3] = vert_normals[vn[0]];
                        model->normals[model->num_triangles * 3 + 4] = vert_normals[vn[2]];
                        model->normals[model->num_triangles * 3 + 5] = vert_normals[vn[3]];
                    }
                }

                ++model->num_triangles;
                if (model->num_triangles == max_triangles)
                    break;

                ++model->num_triangles;
                if (model->num_triangles == max_triangles)
                    break;
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

    if (file)
        fclose(file);

    return result;
}
