Texture2D Image;
SamplerState Sampler;

float4 main(
  in float4 position : SV_POSITION,
  in float3 normal : NORMAL,
  in float2 texcoord : TEXCOORD) : SV_TARGET
{
  static const float3 LightDir = normalize(float3(1, 1, -1));
  static const float3 LightColor = float3(1.f, 1.f, 1.f);

  float4 diff_sample = Image.Sample(Sampler, texcoord);
  return float4(saturate(dot(normal, LightDir)) * (diff_sample.rgb * LightColor), diff_sample.a);
}