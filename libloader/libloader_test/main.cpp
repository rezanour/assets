#include <Windows.h>
#include <Shlwapi.h>
#include <commdlg.h>

#include <memory>
#include <vector>
#include <string>
#include <map>

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

HRESULT LoadTexture(const wchar_t* fullpath, DirectX::TexMetadata* metadata, DirectX::ScratchImage& image)
{
  if (!PathFileExists(fullpath))
    return E_FAIL;

  LPWSTR extension = PathFindExtension(fullpath);
  if (!extension)
    return E_FAIL;

  if (StrCmpI(extension, L".tga") == 0)
  {
    return DirectX::LoadFromTGAFile(fullpath, metadata, image);
  }
  else if (StrCmpI(extension, L".dds") == 0)
  {
    return DirectX::LoadFromDDSFile(fullpath, 0, metadata, image);
  }
  else
  {
    return DirectX::LoadFromWICFile(fullpath, 0, metadata, image);
  }
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

    uint32_t num_verts = 0;
    uint32_t num_indices = 0;
    LARGE_INTEGER start{}, end{}, freq{};
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    libload_obj_model_t* model = nullptr;
    bool result = libload_obj_load(filename, &model);
    if (result)
    {
      libload_obj_compute_normals(model);
      libload_obj_compute_tangent_space(model);

      num_verts = model->num_vertices;
      num_indices = model->num_indices;

      hr = model_renderer->Initialize(hwnd, model->num_vertices, (const Vertex3D*)model->vertices,
        model->num_indices, model->indices);
      if (SUCCEEDED(hr))
      {
        // load materials
        uint32_t num_materials = 0;
        std::vector<libload_mtl_t> materials;
        result = libload_mtl_load(model->material_file, &num_materials, nullptr);
        if (result)
        {
          materials.resize(num_materials);
          result = libload_mtl_load(model->material_file, &num_materials, materials.data());
          if (result)
          {
            char path[1024]{};
            strcpy_s(path, filename);
            PathRemoveFileSpecA(path);

            std::map<std::string, uint32_t> images;
            DirectX::TexMetadata metadata;
            DirectX::ScratchImage scratch;
            DirectX::TexMetadata metadata2;
            DirectX::ScratchImage scratch2;
            for (auto& material : materials)
            {
              auto it = images.find(material.name);
              if (it == images.end())
              {
                wchar_t full_path[1024]{};
                swprintf_s(full_path, L"%S\\%S", path, material.map_Kd);
                hr = LoadTexture(full_path, &metadata, scratch);
                if (SUCCEEDED(hr))
                {
                  uint32_t material_handle = 0;
                  swprintf_s(full_path, L"%S\\%S", path, material.map_bump);
                  hr = LoadTexture(full_path, &metadata2, scratch2);
                  if (SUCCEEDED(hr))
                  {
                    hr = model_renderer->CreateMaterial(
                      (uint32_t)metadata.width, (uint32_t)metadata.height, metadata.format, (const uint32_t*)scratch.GetPixels(),
                      (uint32_t)metadata2.width, (uint32_t)metadata2.height, metadata2.format, (const uint32_t*)scratch2.GetPixels(),
                      &material_handle);
                  }
                  else
                  {
                    hr = model_renderer->CreateMaterial(
                      (uint32_t)metadata.width, (uint32_t)metadata.height, metadata.format, (const uint32_t*)scratch.GetPixels(),
                      0, 0, DXGI_FORMAT_UNKNOWN, nullptr,
                      &material_handle);
                  }
                  if (SUCCEEDED(hr))
                  {
                    images[material.name] = material_handle;
                  }
                  else
                  {
                    *out_error_message = L"Failed to create material resources.";
                    break;
                  }
                }
                else
                {
                  uint32_t material_handle;
                  uint32_t color =
                    0xFF000000 |
                    ((uint32_t)((uint8_t)(material.Kd.z * 255)) << 16) |
                    ((uint32_t)((uint8_t)(material.Kd.y * 255)) << 8) |
                    ((uint32_t)((uint8_t)(material.Kd.x * 255)));
                  hr = model_renderer->CreateMaterial(
                    1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &color,
                    0, 0, DXGI_FORMAT_UNKNOWN, nullptr,
                    &material_handle);
                  if (SUCCEEDED(hr))
                  {
                    images[material.name] = material_handle;
                  }
                }
              }
            }

            if (SUCCEEDED(hr))
            {
              for (uint32_t i = 0; i < model->num_parts; ++i)
              {
                auto it = images.find(model->parts[i].material_name);
                if (it != images.end())
                  model_renderer->AddModel(model->parts[i].base_index, model->parts[i].num_indices, it->second);
              }
            }
          }
          else
          {
            hr = E_FAIL;
            *out_error_message = L"Failed to load materials.";
          }
        }
        else
        {
          uint32_t material_handle;
          uint32_t color = 0xFFFFFFFF;
          hr = model_renderer->CreateMaterial(
            1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &color,
            0, 0, DXGI_FORMAT_UNKNOWN, nullptr,
            &material_handle);
          if (SUCCEEDED(hr))
          {
            model_renderer->AddModel(0, model->num_indices, material_handle);
          }
        }
      }
      else
      {
        *out_error_message = L"Failed to create model renderer.";
      }
      libload_obj_free(model);
    }
    else
    {
      hr = E_FAIL;
      *out_error_message = L"Failed to load OBJ file.";
    }

    QueryPerformanceCounter(&end);
    wchar_t message[500]{};
    swprintf_s(message, L"Verts: %d, Indices: %d, Elapsed: %3.2fms\n", num_verts, num_indices, 1000.f * (end.QuadPart - start.QuadPart) / (float)freq.QuadPart);
    OutputDebugString(message);
  }

  return hr;
}
