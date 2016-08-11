Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0); 

struct pixcelIn
{
	float4 Pos : SV_POSITION;
	float4 Col : COLOR;
	float2 Tex : TEXCOORD0;
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
float4 PS(pixcelIn IN) : SV_Target
{
	pixcelIn OUT;

	//テクスチャーを貼る
	OUT.Col = txDiffuse.Sample(samLinear, IN.Tex);

	return OUT.Col; 
}

//ピクセルシェーダー
//float4 PS(float4 pos : SV_POSITION) : SV_Target
//{
//	return pos;
//}

