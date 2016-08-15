cbuffer constants
{
  float4x4 LocalToWorld;
  float4x4 WorldToView;
  float4x4 ViewToProjection;
};

void main(
  in float3 position : POSITION,
  in float3 normal : NORMAL,
  in float3 tangent : TANGENT,
  in float3 bitangent : BITANGENT,
  in float2 texcoord : TEXCOORD,
  out float4 out_position : SV_POSITION,
  out float3 out_normal : NORMAL,
  out float3 out_tangent : TANGENT,
  out float3 out_bitangent : BITANGENT,
  out float2 out_texcoord : TEXCOORD)
{
  out_position = mul(ViewToProjection,
                   mul(WorldToView,
                     mul(LocalToWorld, float4(position, 1))));

  // only works if local to world is orthonormal
  out_normal = normalize(mul((float3x3)LocalToWorld, normal));
  out_tangent = normalize(mul((float3x3)LocalToWorld, tangent));
  out_bitangent = normalize(mul((float3x3)LocalToWorld, bitangent));

  out_texcoord = texcoord;
}