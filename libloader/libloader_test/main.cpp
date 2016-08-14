#include <Windows.h>
#include <Shlwapi.h>
#include <commdlg.h>

#include <memory>
#include <vector>
#include <string>

#include <DirectXTex.h>

#include <libloader.h>
#include "renderer.h"

static HWND Initialize(HINSTANCE instance, uint32_t width, uint32_t height);
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HRESULT LoadAsset(const char* filename, HWND hwnd, std::unique_ptr<BaseRenderer>* out_renderer, std::wstring* out_error_message);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR command_line, int)
{
  HRESULT hr = S_OK;
  HWND hwnd = nullptr;
  char filename[MAX_PATH]{};
  std::unique_ptr<BaseRenderer> renderer;
  std::wstring error_message = L"Can't determine asset type from filename.";

  // If command_line contains a filename, use that.
  if (strlen(command_line) > 0)
  {
    strcpy_s(filename, command_line);
  }
  else
  {
    // No filename passed in, so prompt for file
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "All Assets (*.tga, *.dds, *.obj)\0*.tga;*.dda;*.obj\0TGA images\0*.tga\0DDS images\0*.dds\0OBJ models\0*.obj\0All Files\0*.*\0\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = _countof(filename);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameA(&ofn))
    {
      hr = E_FAIL;
      error_message = L"Failed to get file to open.";
    }
  }

  // Create the application window
  if (SUCCEEDED(hr))
  {
    hwnd = Initialize(instance, 1280, 720);
    if (!hwnd)
    {
      hr = E_FAIL;
      error_message = L"Failed to create application window.";
    }
  }

  // Load asset based on extension. Initialize the appropriate renderer.
  if (SUCCEEDED(hr))
  {
    hr = LoadAsset(filename, hwnd, &renderer, &error_message);
  }

  // Show window & run message loop
  if (SUCCEEDED(hr))
  {
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    MSG msg{};
    while (msg.message != WM_QUIT && SUCCEEDED(hr))
    {
      if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      else
      {
        renderer->HandleInput();
        hr = renderer->Render(1);
      }
    }
  }

  // Cleanup
  if (renderer)
  {
    renderer.reset();
  }

  if (hwnd)
  {
    DestroyWindow(hwnd);
    hwnd = nullptr;
  }

  // Display error message if failure
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      !error_message.empty() ? error_message.c_str() :
      (L"Error: " + std::to_wstring(hr)).c_str(),
      L"Error", MB_OK | MB_ICONERROR);
  }

  return hr;
}

HWND Initialize(HINSTANCE instance, uint32_t width, uint32_t height)
{
  WNDCLASSEX wcx{};
  wcx.cbSize = sizeof(wcx);
  wcx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wcx.hInstance = instance;
  wcx.lpfnWndProc = WndProc;
  wcx.lpszClassName = L"libloader_test";
  if (RegisterClassEx(&wcx) == INVALID_ATOM)
  {
    return nullptr;
  }

  DWORD style = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

  RECT rc{};
  rc.right = width;
  rc.bottom = height;
  AdjustWindowRect(&rc, style, FALSE);

  return CreateWindow(wcx.lpszClassName, L"libloader_test", style,
    CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
    nullptr, nullptr, instance, nullptr);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_CLOSE:
    PostQuitMessage(0);
    break;

  case WM_KEYDOWN:
    switch (wParam)
    {
    case VK_ESCAPE:
      PostQuitMessage(0);
      break;
    }
    break;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

HRESULT LoadAsset(const char* filename, HWND hwnd, std::unique_ptr<BaseRenderer>* out_renderer, std::wstring* out_error_message)
{
  HRESULT hr = S_OK;

  *out_error_message = L"";

  LPSTR extension = PathFindExtensionA(filename);
  if (StrCmpIA(extension, ".tga") == 0)
  {
    QuadRenderer* quad_renderer = new QuadRenderer;
    out_renderer->reset(quad_renderer);

    DirectX::TexMetadata metadata;
    DirectX::ScratchImage scratch;
    std::wstring filenameW(strlen(filename) + 1, L' ');
    std::transform(filename, filename + strlen(filename), filenameW.begin(),
      [](const char c) { return (wchar_t)c; });
    hr = DirectX::LoadFromTGAFile(filenameW.c_str(), &metadata, scratch);
    if (SUCCEEDED(hr))
    {
      hr = quad_renderer->Initialize(hwnd, (uint32_t)metadata.width, (uint32_t)metadata.height,
        metadata.format, (const uint32_t*)scratch.GetPixels());
      if (FAILED(hr))
      {
        *out_error_message = L"Failed to initialize quad renderer.";
      }
    }
    else
    {
      *out_error_message = L"Failed to open TGA file.";
    }
  }
  else if (StrCmpIA(extension, ".dds") == 0)
  {
    QuadRenderer* quad_renderer = new QuadRenderer;
    out_renderer->reset(quad_renderer);

    DirectX::TexMetadata metadata;
    DirectX::ScratchImage scratch;
    std::wstring filenameW(strlen(filename) + 1, L' ');
    std::transform(filename, filename + strlen(filename), filenameW.begin(),
      [](const char c) { return (wchar_t)c; });
    hr = DirectX::LoadFromDDSFile(filenameW.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratch);
    if (SUCCEEDED(hr))
    {
      hr = quad_renderer->Initialize(hwnd, (uint32_t)metadata.width, (uint32_t)metadata.height,
        metadata.format, (const uint32_t*)scratch.GetPixels());
      if (FAILED(hr))
      {
        *out_error_message = L"Failed to initialize quad renderer.";
      }
    }
    else
    {
      *out_error_message = L"Failed to open DDS file.";
    }
  }
  else if (StrCmpIA(extension, ".obj") == 0)
  {
    ModelRenderer* model_renderer = new ModelRenderer;
    out_renderer->reset(model_renderer);

    std::vector<SimpleVertex3D> vertices(1500000);
    libload_obj_model_t model{};
    model.num_vertices = (uint32_t)vertices.size();
    model.positions = (libload_float3_t*)&vertices.data()->x;
    model.normals = (libload_float3_t*)&vertices.data()->nx;
    model.texcoords = (libload_float2_t*)&vertices.data()->u;
    model.positions_stride = sizeof(SimpleVertex3D);
    model.normals_stride = sizeof(SimpleVertex3D);
    model.texcoords_stride = sizeof(SimpleVertex3D);
    bool result = libload_obj(filename, &model);
    if (result)
    {
      hr = model_renderer->Initialize(hwnd, model.num_vertices, vertices.data());
      if (FAILED(hr))
      {
        *out_error_message = L"Failed to initialize model renderer.";
      }
    }
    else
    {
      hr = E_FAIL;
      *out_error_message = L"Failed to load OBJ file.";
    }
  }

  return hr;
}
