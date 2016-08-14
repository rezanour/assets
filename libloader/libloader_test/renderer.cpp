#include <Windows.h>
#include <assert.h>
#include "renderer.h"

// Shaders
#include "Quad_vs.h"
#include "Quad_ps.h"

#pragma comment(lib, "d3d11.lib")

//=============================================================================
// BaseRenderer
//=============================================================================

HRESULT BaseRenderer::BaseInitialize(HWND hwnd)
{
  RECT rc{};
  GetClientRect(hwnd, &rc);

  DXGI_SWAP_CHAIN_DESC scd{};
  scd.BufferCount = 2;
  scd.BufferDesc.Width = rc.right - rc.left;
  scd.BufferDesc.Height = rc.bottom - rc.top;
  scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  scd.OutputWindow = hwnd;
  scd.SampleDesc.Count = 1;
  scd.Windowed = TRUE;

  D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
  UINT flags = 0;
#ifdef _DEBUG
  flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
    flags, &feature_level, 1, D3D11_SDK_VERSION, &scd, &swapchain_, &device_, nullptr, &context_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  hr = swapchain_->GetBuffer(0, IID_PPV_ARGS(&backbuffer_));
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  hr = device_->CreateRenderTargetView(backbuffer_.Get(), nullptr, &backbuffer_rtv_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  D3D11_TEXTURE2D_DESC td{};
  backbuffer_->GetDesc(&td);

  td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  td.Format = DXGI_FORMAT_D32_FLOAT;

  hr = device_->CreateTexture2D(&td, nullptr, &depthbuffer_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  hr = device_->CreateDepthStencilView(depthbuffer_.Get(), nullptr, &depthbuffer_dsv_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  context_->OMSetRenderTargets(1, backbuffer_rtv_.GetAddressOf(), depthbuffer_dsv_.Get());

  D3D11_VIEWPORT vp{};
  vp.Width = (float)scd.BufferDesc.Width;
  vp.Height = (float)scd.BufferDesc.Height;
  vp.MaxDepth = 1.f;
  context_->RSSetViewports(1, &vp);

  return hr;
}

HRESULT BaseRenderer::Render(int swap_interval)
{
  static const float clear_color[] = { 0.f, 0.f, 0.5f, 1.f };
  context_->ClearRenderTargetView(backbuffer_rtv_.Get(), clear_color);
  context_->ClearDepthStencilView(depthbuffer_dsv_.Get(), D3D11_CLEAR_DEPTH, 1.f, 0);

  OnRender();

  return swapchain_->Present(swap_interval, 0);
}

//=============================================================================
// QuadRenderer
//=============================================================================

HRESULT QuadRenderer::Initialize(HWND hwnd, uint32_t width, uint32_t height, DXGI_FORMAT format, const uint32_t* pixels)
{
  SetWindowPos(hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE);

  HRESULT hr = BaseInitialize(hwnd);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = device_->CreateVertexShader(Quad_vs, sizeof(Quad_vs), nullptr, &vertexshader_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  hr = device_->CreatePixelShader(Quad_ps, sizeof(Quad_ps), nullptr, &pixelshader_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  D3D11_INPUT_ELEMENT_DESC elems[2]{};
  elems[0].Format = DXGI_FORMAT_R32G32_FLOAT;
  elems[0].SemanticName = "POSITION";
  elems[1].AlignedByteOffset = sizeof(float) * 2;
  elems[1].Format = DXGI_FORMAT_R32G32_FLOAT;
  elems[1].SemanticName = "TEXCOORD";
  hr = device_->CreateInputLayout(elems, _countof(elems), Quad_vs, sizeof(Quad_vs), &inputlayout_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  struct vertex_data
  {
    float ndc_x, ndc_y;
    float u, v;
  };

  vertex_data vertices[6] =
  {
    { -1, 1, 0, 0 },
    { 1, 1, 1, 0 },
    { 1, -1, 1, 1 },
    { -1, 1, 0, 0 },
    { 1, -1, 1, 1 },
    { -1, -1, 0, 1 },
  };

  D3D11_BUFFER_DESC bd{};
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.ByteWidth = sizeof(vertices);
  bd.StructureByteStride = sizeof(vertex_data);

  D3D11_SUBRESOURCE_DATA init{};
  init.pSysMem = vertices;
  init.SysMemPitch = sizeof(vertices);
  init.SysMemSlicePitch = init.SysMemPitch;

  hr = device_->CreateBuffer(&bd, &init, &vertexbuffer_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  D3D11_TEXTURE2D_DESC td{};
  td.ArraySize = 1;
  td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  td.Format = format;
  td.Width = width;
  td.Height = height;
  td.SampleDesc.Count = 1;
  td.MipLevels = 1;

  init.pSysMem = pixels;
  init.SysMemPitch = sizeof(uint32_t) * width;
  init.SysMemSlicePitch = init.SysMemPitch * height;

  hr = device_->CreateTexture2D(&td, &init, &texture_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  hr = device_->CreateShaderResourceView(texture_.Get(), nullptr, &texture_srv_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  D3D11_SAMPLER_DESC sd{};
  sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  hr = device_->CreateSamplerState(&sd, &sampler_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  uint32_t strides[] = { sizeof(vertex_data) };
  uint32_t offsets[] = { 0 };

  context_->IASetInputLayout(inputlayout_.Get());
  context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context_->IASetVertexBuffers(0, 1, vertexbuffer_.GetAddressOf(), strides, offsets);
  context_->VSSetShader(vertexshader_.Get(), nullptr, 0);
  context_->PSSetShader(pixelshader_.Get(), nullptr, 0);
  context_->PSSetShaderResources(0, 1, texture_srv_.GetAddressOf());
  context_->PSSetSamplers(0, 1, sampler_.GetAddressOf());

  return S_OK;
}

void QuadRenderer::OnRender()
{
  context_->Draw(6, 0);
}

//=============================================================================
// ModelRenderer
//=============================================================================

HRESULT ModelRenderer::Initialize(HWND hwnd)
{
  HRESULT hr = BaseInitialize(hwnd);
  if (FAILED(hr))
  {
    return hr;
  }

  return hr;
}

void ModelRenderer::OnRender()
{

}
