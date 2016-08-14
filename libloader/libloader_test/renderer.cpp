#include <Windows.h>
#include "renderer.h"

#pragma comment(lib, "d3d11.lib")

//=============================================================================
// BaseRenderer
//=============================================================================

BaseRenderer::BaseRenderer()
{
}

BaseRenderer::~BaseRenderer()
{
}

HRESULT BaseRenderer::Initialize(HWND hwnd)
{
  UNREFERENCED_PARAMETER(hwnd);
  return OnInitialize();
}

HRESULT BaseRenderer::Render(int swap_interval)
{
  UNREFERENCED_PARAMETER(swap_interval);
  OnRender();
  return S_OK;
}

//=============================================================================
// QuadRenderer
//=============================================================================

QuadRenderer::QuadRenderer()
{
}

QuadRenderer::~QuadRenderer()
{
}

HRESULT QuadRenderer::SetImageData(uint32_t width, uint32_t height, DXGI_FORMAT format, const uint32_t* pixels)
{
  UNREFERENCED_PARAMETER(width);
  UNREFERENCED_PARAMETER(height);
  UNREFERENCED_PARAMETER(format);
  UNREFERENCED_PARAMETER(pixels);
  return S_OK;
}

HRESULT QuadRenderer::OnInitialize()
{
  return S_OK;
}

void QuadRenderer::OnRender()
{
}

//=============================================================================
// ModelRenderer
//=============================================================================

ModelRenderer::ModelRenderer()
{

}

ModelRenderer::~ModelRenderer()
{

}

HRESULT ModelRenderer::SetModelData()
{
  return S_OK;
}

HRESULT ModelRenderer::OnInitialize()
{
  return S_OK;
}

void ModelRenderer::OnRender()
{

}
