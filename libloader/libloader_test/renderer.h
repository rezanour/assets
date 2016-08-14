#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <stdint.h>

class QuadRenderer;
class ModelRenderer;

class BaseRenderer
{
public:
  virtual ~BaseRenderer();

  HRESULT Initialize(HWND hwnd);

  HRESULT Render(int swap_interval);

protected:
  BaseRenderer();
  virtual HRESULT OnInitialize() = 0;
  virtual void OnRender() = 0;

private:
  BaseRenderer(const BaseRenderer&) = delete;
  BaseRenderer& operator= (const BaseRenderer&) = delete;

protected:
  Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain_;

  // Core device resources
  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer_;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backbuffer_rtv_;
};

class QuadRenderer : public BaseRenderer
{
public:
  QuadRenderer();
  virtual ~QuadRenderer();

  HRESULT SetImageData(uint32_t width, uint32_t height, DXGI_FORMAT format, const uint32_t* pixels);

private:
  virtual HRESULT OnInitialize() override;
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
};

class ModelRenderer : public BaseRenderer
{
public:
  ModelRenderer();
  virtual ~ModelRenderer();

  HRESULT SetModelData();

private:
  virtual HRESULT OnInitialize() override;
  virtual void OnRender() override;

  ModelRenderer(const ModelRenderer&) = delete;
  ModelRenderer& operator= (const ModelRenderer&) = delete;

private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> vertexbuffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> indexbuffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> constantbuffer_;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> inputlayout_;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexshader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelshader_;
};
