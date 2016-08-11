Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0); 

struct pixcelIn
{
	float4 Pos : SV_POSITION;
	float4 Col : COLOR;
	float2 Tex : TEXCOORD0;
};

//�O���[�o��
cbuffer global
{
	matrix g_WVP; //���[���h����ˉe�܂ł̕ϊ��s��
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float4 Normal : NORMAL;
	float2 Tex	: TEXCOORD;
};

//�o�[�e�b�N�X�V�F�[�_�[
VS_OUT VS(float4 Pos : POSITION, float4 Normal : NORMAL, float2 Tex : TEXCOORD)
{
	VS_OUT Out;
	Out.Pos = mul(Pos, g_WVP);
	Out.Normal = Normal;
	Out.Tex = Tex;
	return Out;
}

//�s�N�Z���V�F�[�_�[
float4 PS(pixcelIn IN) : SV_Target
{
	pixcelIn OUT;

	//�e�N�X�`���[��\��
	OUT.Col = txDiffuse.Sample(samLinear, IN.Tex);

	return OUT.Col; 
}

//�s�N�Z���V�F�[�_�[
//float4 PS(float4 pos : SV_POSITION) : SV_Target
//{
//	return pos;
//}

