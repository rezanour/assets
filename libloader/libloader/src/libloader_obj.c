//=============================================================================
// libloader_obj.c - OBJ model file
// Reza Nourai, 2016
//=============================================================================

#include "..\include\libloader.h"
#include "libloader_obj.h"

#include <stdio.h>
#include <errno.h>

// load OBJ model
bool libload_obj(const wchar_t* filename)
{
    errno_t error = 0;
    FILE* file = 0;
    
    error = _wfopen_s(&file, filename, L"rb");
    if (error != 0)
    {
        return false;
    }

    fclose(file);
    return true;
}
