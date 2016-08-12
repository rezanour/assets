#include <Windows.h>
#include <Shlwapi.h>
#include <libloader.h>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    UNREFERENCED_PARAMETER(instance);

    LPWSTR command_line = GetCommandLine();
    size_t command_line_length = wcslen(command_line);
    LPWSTR command_line_end = command_line + command_line_length;

    LPWSTR first_param = command_line;

    // skip over the executable path itself, which is the first parameter
    if (first_param[0] == L'\"')
    {
        // quoted, so skip all the way to the next '"'
        ++first_param;
        while (first_param < command_line_end && *first_param != L'\"')
        {
            ++first_param;
        }
    }

    // skip until a blank
    while (first_param < command_line_end && *first_param != L' ')
    {
        ++first_param;
    }

    // skip until first non-blank
    while (first_param < command_line_end && *first_param == L' ')
    {
        ++first_param;
    }

    // okay, now find end of the parameter & insert null there
    LPWSTR first_param_end = first_param;
    while (first_param_end < command_line_end && *first_param_end != L' ')
    {
        ++first_param_end;
    }

    if (first_param_end < command_line_end)
    {
        *first_param_end = L'\0';
    }

    LPWSTR extension = PathFindExtension(first_param);

    if (StrCmpIW(extension, L".obj") == 0)
    {
        bool result = libload_obj(first_param);
        if (!result)
        {
            MessageBox(nullptr, L"Failed to open file.", L"Error", MB_OK | MB_ICONERROR);
        }
    }

    return 0;
}
