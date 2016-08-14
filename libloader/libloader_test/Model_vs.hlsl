cbuffer constants
{
  float4x4 LocalToWorld;
  float4x4 WorldToView;
  float4x4 ViewToProjection;
};

void main(
  in float3 position : POSITION,
  in float3 normal : NORMAL,
  in float2 texcoord : TEXCOORD,
  out float4 out_position : SV_POSITION,
  out float3 out_normal : NORMAL,
  out float2 out_texcoord : TEXCOORD)
{
  out_position = mul(ViewToProjection,
                   mul(WorldToView,
                     mul(LocalToWorld, float4(position, 1))));

  // only works if local to world is orthonormal
  out_normal = mul((float3x3)LocalToWorld, normal);

  out_texcoord = texcoord;
}