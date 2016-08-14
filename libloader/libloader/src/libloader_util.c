//=============================================================================
// libloader_util.c - Internal utility functions
// Reza Nourai, 2016
//=============================================================================

#include "..\include\libloader.h"
#include "libloader_util.h"

#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

char* read_text_file(const char* filename, size_t* out_num_bytes)
{
  FILE* file = 0;
  errno_t err = 0;
  fpos_t pos = 0;
  char* buffer = 0;

  *out_num_bytes = 0;

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

  if (fread_s(buffer, pos, 1, pos, file) == 0)
  {
    free(buffer);
    buffer = 0;
    goto cleanup;
  }

  *out_num_bytes = pos;

cleanup:
  if (file)
  {
    fclose(file);
    file = 0;
  }

  return buffer;
}
