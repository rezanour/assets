//=============================================================================
// libloader_util.c - Internal utility functions
// Reza Nourai, 2016
//=============================================================================

#include "..\include\libloader.h"
#include "libloader_util.h"

#include <stdio.h>
#include <malloc.h>
#include <assert.h>

//=============================================================================
// read text file
//=============================================================================

char* read_text_file(const char* filename, uint32_t* out_num_bytes)
{
  errno_t err = 0;
  FILE* file = 0;
  size_t len = 0;
  char* buffer = 0;

  *out_num_bytes = 0;

  err = fopen_s(&file, filename, "rb");
  if (err != 0)
    goto cleanup;

  if (fseek(file, 0, SEEK_END) != 0)
    goto cleanup;

  len = ftell(file);
  rewind(file);

  buffer = (char*)malloc(len);
  if (!buffer)
    goto cleanup;

  if (fread_s(buffer, len, 1, len, file) != len)
  {
    free(buffer);
    buffer = 0;
    goto cleanup;
  }

  *out_num_bytes = (uint32_t)len;

cleanup:
  if (file)
    fclose(file);

  return buffer;
}

//=============================================================================
// keyvalue pair
//=============================================================================

bool keyvalue_find(const keyvalue_pair_t* map, uint32_t map_size, uint64_t key, uint32_t* value)
{
  // binary search
  int low = 0, high = map_size;
  int mid = (low + high) / 2;

  while (mid > low && mid < high)
  {
    if (map[mid].key == key)
    {
      *value = map[mid].value;
      return true;
    }
    else if (map[mid].key > key)
    {
      high = mid;
    }
    else
    {
      low = mid;
    }

    mid = (low + high) / 2;
  }

  return false;
}

void keyvalue_insert(keyvalue_pair_t* map, uint32_t* map_size, uint32_t map_max, uint64_t key, uint32_t value)
{
  // binary search to find optimal insert location
  int low = 0, high = *map_size;
  int mid = (low + high) / 2;

  while (mid > low && mid < high)
  {
    if (map[mid].key == key)
    {
      // already have the key
      return;
    }
    else if (map[mid].key > key)
    {
      high = mid;
    }
    else
    {
      low = mid;
    }

    mid = (low + high) / 2;
  }

  if (*map_size == map_max)
  {
    assert(false);
    // no room
    return;
  }

  // move everything above down
  if (map[mid].key < key)
    ++mid;

  for (int i = (int)*map_size; i > mid; --i)
  {
    map[i] = map[i - 1];
  }

  map[mid].key = key;
  map[mid].value = value;

  ++(*map_size);
}
