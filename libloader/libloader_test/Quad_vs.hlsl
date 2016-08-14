void main(
  in float2 position : POSITION,
  in float2 texcoord : TEXCOORD,
  out float4 out_position : SV_POSITION,
  out float2 out_texcoord : TEXCOORD)
{
  out_position = float4(position, 0, 1);
  out_texcoord = texcoord;
}