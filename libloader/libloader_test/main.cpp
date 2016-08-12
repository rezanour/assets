#include <Windows.h>
#include <Shlwapi.h>
#include <libloader.h>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR command_line, int)
{
    UNREFERENCED_PARAMETER(instance);
    LPSTR extension = PathFindExtensionA(command_line);

    if (StrCmpIA(extension, ".obj") == 0)
    {
        libload_model_t model{};
        model.num_triangles = 1024;
        model.positions = (float*)malloc(sizeof(float) * model.num_triangles * 3);
        model.normals = (float*)malloc(sizeof(float) * model.num_triangles * 3);
        model.texcoords = (float*)malloc(sizeof(float) * model.num_triangles * 3);
        bool result = libload_obj(command_line, &model);
        if (!result)
        {
            MessageBox(nullptr, L"Failed to open file.", L"Error", MB_OK | MB_ICONERROR);
        }
        free(model.positions);
        free(model.normals);
        free(model.texcoords);
    }

    return 0;
}
