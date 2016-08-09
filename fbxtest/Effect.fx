Texture2D g_texColor: register(t0);
SamplerState g_samLinear : register(s0);
struct pixcelIn
{
	float4 pos : POSITION;
	float4 col : COLOR;
};
//グローバル
cbuffer global
{
	matrix g_WVP; //ワールドから射影までの変換行列
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float4 Normal : NORMAL;
	float2 Tex	: TEXCOORD;
};

//バーテックスシェーダー
VS_OUT VS(float4 Pos : POSITION, float4 Normal : NORMAL, float2 Tex : TEXCOORD)
{
	VS_OUT Out;
	Out.Pos = mul(Pos, g_WVP);
	Out.Normal = Normal;
	Out.Tex = Tex;
	return Out;
}

//ピクセルシェーダー
float4 PS(float4 pos : SV_POSITION) : SV_Target
{
	return pos;
}

