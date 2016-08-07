#include "FBXModel.h"
FBXModel::FBXModel()
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

	//頂点レイアウト
	D3D11_INPUT_ELEMENT_DESC InElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

}