//=============================================================================
// libloader.h - simple C asset loading library
// Reza Nourai, 2016
//=============================================================================
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// load OBJ model
bool libload_obj(const wchar_t* filename);

#ifdef __cplusplus
} // extern "C"
#endif
