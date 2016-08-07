#ifndef _FBXMODEL_
#define _FBXMODEL_
#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dx11.h>	
#include <vector>


struct UserMaterial
{
	DirectX::XMFLOAT4 Diffuse;  //拡散反射光
	DirectX::XMFLOAT4 Ambient;  //環境光
	DirectX::XMFLOAT4 Specular; //鏡面反射光
	DirectX::XMFLOAT4 Emissive; //放射光
	float			  Power;	//マテリアルのスペキュラ色指数
};

// 頂点情報が格納される構造体
struct UserVertex
{
	DirectX::XMFLOAT3 Vec;		//頂点データ
	DirectX::XMFLOAT3 Normal;	//頂点データ
	float			  tu;		//テクスチャ座標x
	float			  tv;		//テクスチャ座標y
};

struct UserTexture
{
	const char* TextureName;
	ID3D11ShaderResourceView* pTexture;
};

struct FBXModelData
{
	int							PolygonCount;		// ポリゴン数
	int							PrimitiveCount;		// 三角ポリゴンの数
	int							ControlPointCount;	// コントロールポイントの数
	UserVertex*					pVertex;			// 頂点データ
	std::vector<UserTexture*>	pTextureData;		// テクスチャ情報
	UserMaterial				Material;			// マテリアル情報
};


class FBXModel
{
public:
	FBXModel();
	~FBXModel();
	std::vector<FBXModelData*>	m_pFbxModelData;
	void Draw();
private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* pDeviceContext;

};
#endif