//=============================================================================
// libloader_util.h - Internal utility functions
// Reza Nourai, 2016
//=============================================================================
#pragma once

#define LIBLOAD_ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))

//=============================================================================
// read text file into memory buffer
//=============================================================================

char* read_text_file(const char* filename, uint32_t* out_num_bytes);

//=============================================================================
// keyvalue pair (map) with uint64 key & uint32 values
//=============================================================================

typedef struct
{
  uint64_t key;
  uint32_t value;
} keyvalue_pair_t;

bool keyvalue_find(const keyvalue_pair_t* map, uint32_t map_size, uint64_t key, uint32_t* value);
void keyvalue_insert(keyvalue_pair_t* map, uint32_t* map_size, uint32_t map_max, uint64_t key, uint32_t value);
