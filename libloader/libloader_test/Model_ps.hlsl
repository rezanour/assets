Texture2D DiffuseMap : register (t0);
Texture2D NormalMap : register (t1);
SamplerState Sampler;

float4 main(
  in float4 position : SV_POSITION,
  in float3 normal : NORMAL,
  in float3 tangent : TANGENT,
  in float3 bitangent  : BITANGENT,
  in float2 texcoord : TEXCOORD) : SV_TARGET
{
  static const float3 LightDir = normalize(float3(1, 1, -1));
  static const float3 LightColor = float3(1.f, 1.f, 1.f);

  float4 diffuse = DiffuseMap.Sample(Sampler, texcoord);
  clip(diffuse.a - 0.5f);

  //if (length(tangent) > 0)
  //{
  //  float3x3 TanToWorld = float3x3(normalize(tangent), normalize(bitangent), normalize(normal));
  //  float3 normal_sample = NormalMap.Sample(Sampler, texcoord).xyz * 2 - 1;
  //  if (length(normal_sample) > 0)
  //  {
  //    normal = mul(TanToWorld, normal_sample);
  //  }
  //}

  return float4(saturate(dot(normal, LightDir)) * (diffuse.rgb * LightColor), diffuse.a);
}