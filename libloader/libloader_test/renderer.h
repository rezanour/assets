#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <stdint.h>
#include <DirectXMath.h>
#include <vector>

class QuadRenderer;
class ModelRenderer;

struct SimpleVertex3D
{
  float x, y, z;    // position
  float nx, ny, nz; // normal
  float u, v;       // texcoord
};

class BaseRenderer
{
public:
  virtual ~BaseRenderer() {}

  virtual void HandleInput() {}
  HRESULT Render(int swap_interval);

protected:
  BaseRenderer() {}

  HRESULT BaseInitialize(HWND hwnd);
  virtual void OnRender() = 0;

private:
  BaseRenderer(const BaseRenderer&) = delete;
  BaseRenderer& operator= (const BaseRenderer&) = delete;

protected:
  Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain_;
  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer_;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> depthbuffer_;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backbuffer_rtv_;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthbuffer_dsv_;
};

class QuadRenderer : public BaseRenderer
{
public:
  QuadRenderer() {}
  virtual ~QuadRenderer() {}

  HRESULT Initialize(HWND hwnd, uint32_t width, uint32_t height, DXGI_FORMAT format, const uint32_t* pixels);

private:
  virtual void OnRender() override;

  QuadRenderer(const QuadRenderer&) = delete;
  QuadRenderer& operator= (const QuadRenderer&) = delete;

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> vertexbuffer_;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> inputlayout_;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexshader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelshader_;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_srv_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_;
};

class ModelRenderer : public BaseRenderer
{
public:
  ModelRenderer() {}
  virtual ~ModelRenderer() {}

  HRESULT Initialize(HWND hwnd, uint32_t num_vertices, const SimpleVertex3D* vertices);
  HRESULT Initialize(HWND hwnd, uint32_t num_vertices, const SimpleVertex3D* vertices, uint32_t num_indices, const uint32_t* indices);

  HRESULT CreateImage(uint32_t width, uint32_t height, DXGI_FORMAT format, const uint32_t* pixels, uint32_t* out_image_handle);
  void AddModel(uint32_t base_index, uint32_t num_indices, uint32_t image_handle);

  virtual void HandleInput() override;

private:
  virtual void OnRender() override;

  ModelRenderer(const ModelRenderer&) = delete;
  ModelRenderer& operator= (const ModelRenderer&) = delete;

private:
  struct constant_data
  {
    DirectX::XMFLOAT4X4 LocalToWorld;
    DirectX::XMFLOAT4X4 WorldToView;
    DirectX::XMFLOAT4X4 ViewToProjection;
  };

  struct image_data
  {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
  };

  struct model_data
  {
    uint32_t base_index;  // base_vertex if not using indexbuffer
    uint32_t num_indices; // num_vertices if not using indexbuffer
    uint32_t image_handle;
  };

  Microsoft::WRL::ComPtr<ID3D11Buffer> vertexbuffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> indexbuffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> constantbuffer_;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> inputlayout_;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexshader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelshader_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_;
  Microsoft::WRL::ComPtr<ID3D11BlendState> blendstate_;
  uint32_t num_vertices_ = 0;
  uint32_t num_indices_ = 0;
  std::vector<image_data> images_;
  std::vector<model_data> models_;

  constant_data constants_ = {};
  DirectX::XMFLOAT3 camera_position_ = DirectX::XMFLOAT3(0, 0, -500);
  DirectX::XMFLOAT4 camera_orientation_ = DirectX::XMFLOAT4(0, 0, 0, 1);
};
