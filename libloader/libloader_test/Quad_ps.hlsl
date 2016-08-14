Texture2D Image;
SamplerState Sampler;

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
  return Image.Sample(Sampler, texcoord);
}