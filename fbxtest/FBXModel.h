#ifndef _FBXMODEL_
#define _FBXMODEL_
#include <d3d11.h>
#include <d3dx11.h>	
#include <vector>
#include <d3dx10.h>	

// インデックスを格納する構造体
struct UserIndex
{
	int	IndexCount;		// インデックス数
	WORD* IndexAry;		// インデックスデータ
};

struct UserMaterial
{
	D3DXCOLOR Diffuse;  //拡散反射光
	D3DXCOLOR Ambient;  //環境光
	D3DXCOLOR Specular; //鏡面反射光
	D3DXCOLOR Emissive; //放射光
	float	  Power;	//マテリアルのスペキュラ色指数
};

// 頂点情報が格納される構造体
struct UserVertex
{
	D3DXVECTOR3 Vec;	//頂点データ
	D3DXVECTOR3 Normal;	//頂点データ
	float		tu;		//テクスチャ座標x
	float		tv;		//テクスチャ座標y
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
	UserIndex					pIndex;				// インデックス系データ
	std::vector<UserTexture*>	pTextureData;		// テクスチャ情報
	UserMaterial				Material;			// マテリアル情報
};


class FBXModel
{
public:
	FBXModel(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	~FBXModel();
	std::vector<FBXModelData*>	m_pFbxModelData;
	void Draw();
private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;

};
#endif