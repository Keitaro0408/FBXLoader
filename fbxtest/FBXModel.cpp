#include "FBXModel.h"
#include <tchar.h>

struct SHADER_CONSTANT_BUFFER
{
	D3DXMATRIX WVP;
};

FBXModel::FBXModel(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext) :
m_pDevice(_pDevice),
m_pDeviceContext(_pDeviceContext)
{
}

FBXModel::~FBXModel()
{
	for (unsigned int i = 0; i < m_pFbxModelData.size(); i++)
	{
		for (unsigned int n = 0; n < m_pFbxModelData[i]->pTextureData.size(); n++)
		{
			if (m_pFbxModelData[i]->pTextureData[n]->pTexture == NULL)
			{

			}
			else
			{
				m_pFbxModelData[i]->pTextureData[n]->pTexture->Release();
				delete m_pFbxModelData[i]->pTextureData[n];
			}
		}

		if (m_pFbxModelData[i]->pVertex != NULL)
		{
			delete[] m_pFbxModelData[i]->pVertex;
		}
		delete m_pFbxModelData[i];
	}
}

void FBXModel::Draw()
{
	//-------------------
	// �R���X�^���g�o�b�t�@
	//-------------------
	ID3D11Buffer* pConstantBuffer;
	D3D11_BUFFER_DESC ConstantBufferDesc;
	ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ConstantBufferDesc.ByteWidth = sizeof(SHADER_CONSTANT_BUFFER);
	ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ConstantBufferDesc.MiscFlags = 0;
	ConstantBufferDesc.StructureByteStride = 0;
	ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_pDevice->CreateBuffer(&ConstantBufferDesc, NULL, &pConstantBuffer);


	//���_���C�A�E�g��`
	D3D11_INPUT_ELEMENT_DESC InElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(D3DXVECTOR3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(D3DXVECTOR3) + sizeof(D3DXVECTOR3), D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	//--------------------------------------
	// �V�F�[�_�[�̓ǂݍ��݂⃌�C�A�E�g�쐬
	//--------------------------------------
	ID3DBlob* pCompiledShader = NULL;
	ID3DBlob *pErrors = NULL;

	//-------------------
	// ���_�V�F�[�_�[�̓ǂݍ��݂ƃ��C�A�E�g�쐬
	//-------------------
	ID3D11VertexShader* pVertexShader = NULL;
	ID3D11InputLayout* pVertexShaderLayout = NULL;
	if (FAILED(D3DX11CompileFromFile(
		"Effect.fx",
		NULL,
		NULL,
		"VS",
		"vs_5_0",
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION,
		0,
		NULL,
		&pCompiledShader,
		&pErrors,
		NULL)))
	{
		MessageBox(0, "VertexShader�̃R���p�C���Ɏ��s", 0, MB_OK);
	}
	m_pDevice->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &pVertexShader);

	//���_�C���v�b�g���C�A�E�g���쐬
	m_pDevice->CreateInputLayout(
		InElementDesc,
		sizeof(InElementDesc) / sizeof(InElementDesc[0]),
		pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(),
		&pVertexShaderLayout);

	pCompiledShader->Release();

	//-------------------
	// �s�N�Z���V�F�[�_�[�̓ǂݍ���
	//-------------------
	ID3D11PixelShader* pPixelShader = NULL;
	if (FAILED(D3DX11CompileFromFile(
		"Effect.fx",
		NULL,
		NULL,
		"PS",
		"ps_5_0",
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION,
		0,
		NULL,
		&pCompiledShader,
		&pErrors,
		NULL)))
	{
		MessageBox(0, "PixelShader�̃R���p�C���Ɏ��s", 0, MB_OK);
	}
	m_pDevice->CreatePixelShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &pPixelShader);

	pCompiledShader->Release();

	int vertexNum = 0;
	UserVertex *pVertex = NULL;
	for (unsigned int i = 0; i < m_pFbxModelData.size(); i++)
	{
		vertexNum = m_pFbxModelData[i]->ControlPointCount;
		pVertex = m_pFbxModelData[i]->pVertex;
	}

	//���_�o�b�t�@�쐬
	D3D11_BUFFER_DESC BufferDesc;
	BufferDesc.ByteWidth = sizeof(UserVertex) * vertexNum;
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = sizeof(float);

	D3D11_SUBRESOURCE_DATA InitVertexData;
	InitVertexData.pSysMem = pVertex;
	ID3D11Buffer* pBuffer;
	m_pDevice->CreateBuffer(&BufferDesc, &InitVertexData, &pBuffer);

	D3DXMATRIX World,Rotate;
	D3DXMATRIX View;
	D3DXMATRIX Proj;
	static float rad = 0;
	//���[���h�g�����X�t�H�[��
	D3DXMatrixIdentity(&World);
	D3DXMatrixRotationY(&Rotate, rad);
	D3DXMatrixMultiply(&World, &World,&Rotate);
	rad += 0.01;
	// �r���[�g�����X�t�H�[��
	D3DXVECTOR3 vEyePt(0.0f, 5.0f, -500.0f);		//���_�ʒu
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);	//�����ʒu
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);		//����ʒu
	D3DXMatrixLookAtLH(&View, &vEyePt, &vLookatPt, &vUpVec);

	// �v���W�F�N�V�����g�����X�t�H�[��
	D3DXMatrixPerspectiveFovLH(&Proj, (FLOAT)D3DX_PI / 4, (FLOAT)1280 / (FLOAT)720, 0.1f, 1000.0f);

	//�V�F�[�_�[���R���e�L�X�g�ɐݒ�
	m_pDeviceContext->VSSetShader(pVertexShader, NULL, 0);
	m_pDeviceContext->PSSetShader(pPixelShader, NULL, 0);

	//�V�F�[�_�[�̃R���X�^���g�o�b�t�@�[�Ɋe��f�[�^��n��
	D3D11_MAPPED_SUBRESOURCE SubResourceData;
	SHADER_CONSTANT_BUFFER Constant;
	if (SUCCEEDED(m_pDeviceContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResourceData)))
	{
		// �ϊ��������W��n��
		Constant.WVP = World*View*Proj;
		D3DXMatrixTranspose(&Constant.WVP, &Constant.WVP);

		memcpy_s(SubResourceData.pData, SubResourceData.RowPitch, (void*)(&Constant), sizeof(Constant));
		m_pDeviceContext->Unmap(pConstantBuffer, 0);
	}

	// �R���X�^���g�o�b�t�@�[���Z�b�g
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);


	//���̒��_�o�b�t�@���R���e�L�X�g�ɐݒ�
	UINT Strides = sizeof(UserVertex);
	UINT Offsets = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Strides, &Offsets);


	//���_���C�A�E�g���R���e�L�X�g�ɐݒ�
	m_pDeviceContext->IASetInputLayout(pVertexShaderLayout);
	//�v���~�e�B�u(�|���S���̌`��)���R���e�L�X�g�ɐݒ�
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	m_pDeviceContext->Draw(vertexNum, 0);
}