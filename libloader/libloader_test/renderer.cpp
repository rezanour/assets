#include <Windows.h>
#include <assert.h>
#include "renderer.h"

// Shaders
#include "Quad_vs.h"
#include "Quad_ps.h"
#include "Model_vs.h"
#include "Model_ps.h"

#include <DirectXMath.h>
using namespace DirectX;

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

HRESULT ModelRenderer::Initialize(HWND hwnd, uint32_t num_vertices, const Vertex3D* vertices)
{
  HRESULT hr = BaseInitialize(hwnd);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = device_->CreateVertexShader(Model_vs, sizeof(Model_vs), nullptr, &vertexshader_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  hr = device_->CreatePixelShader(Model_ps, sizeof(Model_ps), nullptr, &pixelshader_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  D3D11_INPUT_ELEMENT_DESC elems[5]{};
  elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  elems[0].SemanticName = "POSITION";
  elems[1].AlignedByteOffset = sizeof(float) * 3;
  elems[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  elems[1].SemanticName = "NORMAL";
  elems[2].AlignedByteOffset = sizeof(float) * 6;
  elems[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  elems[2].SemanticName = "TANGENT";
  elems[3].AlignedByteOffset = sizeof(float) * 9;
  elems[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  elems[3].SemanticName = "BITANGENT";
  elems[4].AlignedByteOffset = sizeof(float) * 12;
  elems[4].Format = DXGI_FORMAT_R32G32_FLOAT;
  elems[4].SemanticName = "TEXCOORD";
  hr = device_->CreateInputLayout(elems, _countof(elems), Model_vs, sizeof(Model_vs), &inputlayout_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  D3D11_BUFFER_DESC bd{};
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.StructureByteStride = sizeof(Vertex3D);
  bd.ByteWidth = bd.StructureByteStride * num_vertices;

  D3D11_SUBRESOURCE_DATA init{};
  init.pSysMem = vertices;
  init.SysMemPitch = sizeof(Vertex3D) * num_vertices;
  init.SysMemSlicePitch = init.SysMemPitch;

  hr = device_->CreateBuffer(&bd, &init, &vertexbuffer_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  num_vertices_ = num_vertices;

  RECT rc{};
  GetClientRect(hwnd, &rc);

  XMStoreFloat4x4(&constants_.LocalToWorld, XMMatrixScaling(scale_, scale_, scale_));
  XMStoreFloat4x4(&constants_.WorldToView, XMMatrixLookAtLH(
    XMLoadFloat3(&camera_position_), XMVectorZero(), XMVectorSet(0, 1, 0, 0)));
  XMStoreFloat4x4(&constants_.ViewToProjection, XMMatrixPerspectiveFovLH(
    XMConvertToRadians(60.f), (rc.right - rc.left) / (float)(rc.bottom - rc.top), 0.1f, 100.f));

  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.StructureByteStride = sizeof(constant_data);
  bd.ByteWidth = bd.StructureByteStride;

  init.pSysMem = &constants_;
  init.SysMemPitch = sizeof(constant_data);
  init.SysMemSlicePitch = init.SysMemPitch;

  hr = device_->CreateBuffer(&bd, &init, &constantbuffer_);
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

  D3D11_BLEND_DESC blend{};
  blend.RenderTarget[0].BlendEnable = TRUE;
  blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
  blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  hr = device_->CreateBlendState(&blend, &blendstate_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  uint32_t strides[] = { sizeof(Vertex3D) };
  uint32_t offsets[] = { 0 };

  context_->IASetInputLayout(inputlayout_.Get());
  context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context_->IASetVertexBuffers(0, 1, vertexbuffer_.GetAddressOf(), strides, offsets);
  context_->VSSetShader(vertexshader_.Get(), nullptr, 0);
  context_->VSSetConstantBuffers(0, 1, constantbuffer_.GetAddressOf());
  context_->PSSetShader(pixelshader_.Get(), nullptr, 0);
  context_->PSSetSamplers(0, 1, sampler_.GetAddressOf());
  //context_->OMSetBlendState(blendstate_.Get(), nullptr, D3D11_DEFAULT_SAMPLE_MASK);

  return S_OK;
}

HRESULT ModelRenderer::Initialize(HWND hwnd, uint32_t num_vertices, const Vertex3D* vertices, uint32_t num_indices, const uint32_t* indices)
{
  HRESULT hr = Initialize(hwnd, num_vertices, vertices);
  if (FAILED(hr))
  {
    return hr;
  }

  D3D11_BUFFER_DESC bd{};
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.StructureByteStride = sizeof(uint32_t);
  bd.ByteWidth = bd.StructureByteStride * num_indices;

  D3D11_SUBRESOURCE_DATA init{};
  init.pSysMem = indices;
  init.SysMemPitch = sizeof(uint32_t) * num_indices;
  init.SysMemSlicePitch = init.SysMemPitch;

  hr = device_->CreateBuffer(&bd, &init, &indexbuffer_);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  num_indices_ = num_indices;

  context_->IASetIndexBuffer(indexbuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

  return S_OK;
}

HRESULT ModelRenderer::CreateMaterial(
  uint32_t diff_width, uint32_t diff_height, DXGI_FORMAT diff_format, const uint32_t* diffuse,
  uint32_t norm_width, uint32_t norm_height, DXGI_FORMAT norm_format, const uint32_t* normals,
  uint32_t* out_material_handle)
{
  HRESULT hr = S_OK;

  material_data mat{};

  D3D11_TEXTURE2D_DESC td{};
  td.ArraySize = 1;
  td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  td.Format = diff_format;
  td.Width = diff_width;
  td.Height = diff_height;
  td.SampleDesc.Count = 1;
  td.MipLevels = 1;

  D3D11_SUBRESOURCE_DATA init{};
  init.pSysMem = diffuse;
  init.SysMemPitch = sizeof(uint32_t) * diff_width;
  init.SysMemSlicePitch = init.SysMemPitch * diff_height;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
  hr = device_->CreateTexture2D(&td, &init, texture.ReleaseAndGetAddressOf());
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  hr = device_->CreateShaderResourceView(texture.Get(), nullptr, &mat.diffuse_srv);
  if (FAILED(hr))
  {
    assert(false);
    return hr;
  }

  // HACK: need to handle other formats
  if (normals && norm_format == DXGI_FORMAT_R8G8B8A8_UNORM)
  {
    td.Format = norm_format;
    td.Width = norm_width;
    td.Height = norm_height;

    init.pSysMem = normals;
    init.SysMemPitch = sizeof(uint32_t) * norm_width;
    init.SysMemSlicePitch = init.SysMemPitch * norm_height;

    hr = device_->CreateTexture2D(&td, &init, texture.ReleaseAndGetAddressOf());
    if (FAILED(hr))
    {
      assert(false);
      return hr;
    }

    hr = device_->CreateShaderResourceView(texture.Get(), nullptr, &mat.normals_srv);
    if (FAILED(hr))
    {
      assert(false);
      return hr;
    }

  }

  materials_.push_back(mat);
  *out_material_handle = (uint32_t)(materials_.size() - 1);

  return S_OK;
}

void ModelRenderer::AddModel(uint32_t base_index, uint32_t num_indices, uint32_t material_handle)
{
  model_data model{};
  model.base_index = base_index;
  model.num_indices = num_indices;
  model.material_index = material_handle;
  models_.push_back(model);
}

void ModelRenderer::HandleInput()
{
  // extract vectors from quaternion so we can base movement on them
  XMVECTOR quat = XMLoadFloat4(&camera_orientation_);
  XMVECTOR right = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), quat);
  XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), quat);
  XMVECTOR up = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), quat);

  // amount of movement due to input
  XMVECTOR movement = XMVectorZero();
  XMVECTOR rotation = XMQuaternionIdentity();

  float speed = 0.125f;
  if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
    speed = 5.f;

  // translation
  if (GetAsyncKeyState('W') & 0x8000)
  {
    movement += forward * speed;
  }
  if (GetAsyncKeyState('A') & 0x8000)
  {
    movement -= right * speed;
  }
  if (GetAsyncKeyState('S') & 0x8000)
  {
    movement -= forward * speed;
  }
  if (GetAsyncKeyState('D') & 0x8000)
  {
    movement += right * speed;
  }

  // rotation
  if (GetAsyncKeyState(VK_LEFT) & 0x8000)
  {
    rotation = XMQuaternionMultiply(rotation, XMQuaternionRotationAxis(up, -XMConvertToRadians(1.f)));
  }
  if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
  {
    rotation = XMQuaternionMultiply(rotation, XMQuaternionRotationAxis(up, XMConvertToRadians(1.f)));
  }
  if (GetAsyncKeyState(VK_UP) & 0x8000)
  {
    rotation = XMQuaternionMultiply(rotation, XMQuaternionRotationAxis(right, -XMConvertToRadians(1.f)));
  }
  if (GetAsyncKeyState(VK_DOWN) & 0x8000)
  {
    rotation = XMQuaternionMultiply(rotation, XMQuaternionRotationAxis(right, XMConvertToRadians(1.f)));
  }

  // Store new position based on movement
  XMStoreFloat3(&camera_position_, XMLoadFloat3(&camera_position_) + movement);

  // adjust orientation to always maintain horizon line
  quat = XMQuaternionNormalize(XMQuaternionMultiply(quat, rotation));

  forward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), quat);
  right = XMVector3Cross(XMVectorSet(0, 1, 0, 0), forward);
  up = XMVector3Cross(forward, right);

  XMMATRIX rot;
  rot.r[0] = right;
  rot.r[1] = up;
  rot.r[2] = forward;
  rot.r[3] = XMVectorSet(0, 0, 0, 1);
  XMStoreFloat4(&camera_orientation_, XMQuaternionRotationMatrix(rot));

  // scale
  if (GetAsyncKeyState('Z') & 0x8000)
    scale_ -= 0.01f;
  if (GetAsyncKeyState('X') & 0x8000)
    scale_ += 0.01f;

  XMStoreFloat4x4(&constants_.LocalToWorld, XMMatrixScaling(scale_, scale_, scale_));
}

void ModelRenderer::OnRender()
{
  XMVECTOR position = XMLoadFloat3(&camera_position_);
  XMVECTOR quat = XMLoadFloat4(&camera_orientation_);
  XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), quat);
  XMVECTOR up = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), quat);

  XMStoreFloat4x4(&constants_.WorldToView, XMMatrixLookToLH(position, forward, up));
  context_->UpdateSubresource(constantbuffer_.Get(), 0, nullptr, &constants_, sizeof(constants_), 0);

  if (indexbuffer_)
  {
    for (auto& model : models_)
    {
      auto& mat = materials_[model.material_index];
      ID3D11ShaderResourceView* srvs[] = { mat.diffuse_srv.Get(), mat.normals_srv.Get() };
      context_->PSSetShaderResources(0, 2, srvs);
      context_->DrawIndexed(model.num_indices, model.base_index, 0);
    }
  }
  else
  {
    for (auto& model : models_)
    {
      auto& mat = materials_[model.material_index];
      ID3D11ShaderResourceView* srvs[] = { mat.diffuse_srv.Get(), mat.normals_srv.Get() };
      context_->PSSetShaderResources(0, 2, srvs);
      context_->Draw(model.num_indices, model.base_index);
    }
  }
}
