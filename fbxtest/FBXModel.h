#ifndef _FBXMODEL_
#define _FBXMODEL_
#include <d3d11.h>
#include <d3dx11.h>	
#include <vector>
#include <d3dx10.h>	

// �C���f�b�N�X���i�[����\����
struct UserIndex
{
	int	IndexCount;		// �C���f�b�N�X��
	WORD* IndexAry;		// �C���f�b�N�X�f�[�^
};

struct UserMaterial
{
	D3DXCOLOR Diffuse;  //�g�U���ˌ�
	D3DXCOLOR Ambient;  //����
	D3DXCOLOR Specular; //���ʔ��ˌ�
	D3DXCOLOR Emissive; //���ˌ�
	float	  Power;	//�}�e���A���̃X�y�L�����F�w��
};

// ���_��񂪊i�[�����\����
struct UserVertex
{
	D3DXVECTOR3 Vec;	//���_�f�[�^
	D3DXVECTOR3 Normal;	//���_�f�[�^
	float		tu;		//�e�N�X�`�����Wx
	float		tv;		//�e�N�X�`�����Wy
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
	UserIndex					pIndex;				// �C���f�b�N�X�n�f�[�^
	std::vector<UserTexture*>	pTextureData;		// �e�N�X�`�����
	UserMaterial				Material;			// �}�e���A�����
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