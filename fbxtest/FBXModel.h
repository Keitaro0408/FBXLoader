#ifndef _FBXMODEL_
#define _FBXMODEL_
#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dx11.h>	
#include <vector>


struct UserMaterial
{
	DirectX::XMFLOAT4 Diffuse;  //�g�U���ˌ�
	DirectX::XMFLOAT4 Ambient;  //����
	DirectX::XMFLOAT4 Specular; //���ʔ��ˌ�
	DirectX::XMFLOAT4 Emissive; //���ˌ�
	float			  Power;	//�}�e���A���̃X�y�L�����F�w��
};

// ���_��񂪊i�[�����\����
struct UserVertex
{
	DirectX::XMFLOAT3 Vec;		//���_�f�[�^
	DirectX::XMFLOAT3 Normal;	//���_�f�[�^
	float			  tu;		//�e�N�X�`�����Wx
	float			  tv;		//�e�N�X�`�����Wy
};

struct UserTexture
{
	const char* TextureName;
	ID3D11ShaderResourceView* pTexture;
};

struct FBXModelData
{
	int							PolygonCount;		// �|���S����
	int							PrimitiveCount;		// �O�p�|���S���̐�
	int							ControlPointCount;	// �R���g���[���|�C���g�̐�
	UserVertex*					pVertex;			// ���_�f�[�^
	std::vector<UserTexture*>	pTextureData;		// �e�N�X�`�����
	UserMaterial				Material;			// �}�e���A�����
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