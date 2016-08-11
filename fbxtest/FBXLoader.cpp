#include "FBXLoader.h"
#include "FBXModel.h"
#include <windows.h>
#include <d3dx10.h>	

FBXLoader::FBXLoader(ID3D11Device* _pDevice) :
m_pDevice(_pDevice),
m_pFBXManager(NULL),
m_pFBXScene(NULL),
m_pFBXImporter(NULL),
m_pFBXIOSettings(NULL),
m_pFBXModel(NULL)
{
	m_pFBXManager = fbxsdk::FbxManager::Create();
	if (m_pFBXManager == NULL){ MessageBox(NULL, TEXT("FbxManager�N���X�̐����Ɏ��s"), TEXT("�G���["), MB_OK); }

	m_pFBXScene = fbxsdk::FbxScene::Create(m_pFBXManager, "");
	if (m_pFBXScene == NULL){ MessageBox(NULL, TEXT("FbxScene�N���X�̐����Ɏ��s"), TEXT("�G���["), MB_OK); }

	m_pFBXImporter = fbxsdk::FbxImporter::Create(m_pFBXManager, "");
	if (m_pFBXImporter == NULL){ MessageBox(NULL, TEXT("FbxImporter�N���X�̐����Ɏ��s"), TEXT("�G���["), MB_OK); }

	m_pFBXIOSettings = fbxsdk::FbxIOSettings::Create(m_pFBXManager, IOSROOT);
	if (m_pFBXIOSettings == NULL){ MessageBox(NULL, TEXT("FbxIOSetting�N���X�̐����Ɏ��s"), TEXT("�G���["), MB_OK); }
	m_pFBXManager->SetIOSettings(m_pFBXIOSettings);
}

FBXLoader::~FBXLoader()
{

	m_pFBXScene->Destroy();
	m_pFBXIOSettings->Destroy();
	m_pFBXImporter->Destroy();
	m_pFBXManager->Destroy();
}

bool FBXLoader::FileLoad(char* _FileName)
{
	//FBX�t�@�C�����J��
	if (m_pFBXImporter->Initialize(_FileName) == false)
	{
		MessageBox(NULL, TEXT("FbxImporter��Initialize�Ɏ��s���܂���"), TEXT("�G���["), MB_OK);
		return false;
	}

	if (m_pFBXImporter->Import(m_pFBXScene) == false)
	{
		MessageBox(NULL, TEXT("FbxImporter��Import�Ɏ��s���܂���"), TEXT("�G���["), MB_OK);
		return false;
	}

	FbxGeometryConverter GeometryConverter(m_pFBXManager);
	if (GeometryConverter.Triangulate(m_pFBXScene, true) == false)
	{
		MessageBox(NULL, TEXT("�O�p�`���Ɏ��s���܂����B"), TEXT("�G���["), MB_OK);
		return false;
	}
	return true;
}

bool FBXLoader::GetModelData(FBXModel* _pFBXModel)
{
	m_pFBXModel = _pFBXModel;

	// ���[�g�m�[�h(�ŏ�ʃm�[�h)�̎擾
	fbxsdk::FbxNode* pRootNode = m_pFBXScene->GetRootNode();
	RecursiveNode(pRootNode);

	return true;
}

void FBXLoader::RecursiveNode(fbxsdk::FbxNode* pNode)
{
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		fbxsdk::FbxNode* pChild = pNode->GetChild(i);  // �q�m�[�h���擾
		RecursiveNode(pChild);
	}

	fbxsdk::FbxNodeAttribute* pAttribute = pNode->GetNodeAttribute();

	if (pAttribute != NULL)
	{
		switch (pAttribute->GetAttributeType())
		{
		case fbxsdk::FbxNodeAttribute::eMesh:
			GetMesh(pAttribute);		//Mesh���쐬
			break;
		}
	}
}

void FBXLoader::GetTextureName(
	fbxsdk::FbxSurfaceMaterial* material,
	const char* matFlag,
	std::vector<const char*>* pOutFileName,
	std::vector<fbxsdk::FbxString>* pOutUvSetName,
	int* OutCount)
{
	// �v���p�e�B�擾(Diffus��Emissive�Ȃǂ̃f�[�^�̉�)
	fbxsdk::FbxProperty Property = material->FindProperty(matFlag);

	// �v���p�e�B�ɂ��Ă��Ă郌�C���[�e�N�X�`���̐��擾
	int LayerTextureCount = Property.GetSrcObjectCount<fbxsdk::FbxLayeredTexture>();

	// ���C���[�e�N�X�`�����Ȃ���Βʏ�e�N�X�`����T��
	if (LayerTextureCount == 0)
	{
		// �v���p�e�B�ɂ��Ă��Ă�e�N�X�`���̐����擾
		int TextureCount = Property.GetSrcObjectCount<fbxsdk::FbxFileTexture>();
		for (int i = 0; i < TextureCount; ++i)
		{
			// �e�N�X�`�����J�E���g
			(*OutCount)++;
			// �e�N�X�`���擾
			fbxsdk::FbxFileTexture* fbxTexture = FbxCast<fbxsdk::FbxFileTexture>(Property.GetSrcObject<fbxsdk::FbxFileTexture>(i));
			pOutUvSetName->push_back(fbxTexture->UVSet.Get());
			pOutFileName->push_back(fbxTexture->GetRelativeFileName());
			// GetName�͐�΃p�X�擾�₯�ǁA��΃p�X���Ă̂́A���f�����쐬�����ꏊ�ւ̐�΃p�X�炵������A���f�����쐬�����l�Ɉˑ�������ۂ�(�܂�g���Ȃ�)
			// GetRelativeFileName�͑��΃p�X�擾(���΃p�X�����܂ɂ��������̂����f�����쐬�����l�ˑ�������)
		}
	}
	else
	{
		// ���C���e�N�X�`���̎擾�͍l����
		// �Ƃ������}���`�e�N�X�`�����ǂ��Ǘ����邩�l���Ă�Œ�
	}

}


void FBXLoader::GetMesh(fbxsdk::FbxNodeAttribute* _pAttribute)
{

	FBXModelData* ModelData = new FBXModelData;
	ModelData->pVertex = NULL;

	// �_�E���L���X�g
	fbxsdk::FbxMesh* pFbxMesh = (fbxsdk::FbxMesh*)_pAttribute;

	//-------------------------------------------------------------------------
	//							���_���ƃC���f�b�N�X
	//-------------------------------------------------------------------------

	// �|���S�������擾����
	int PolygonCount = pFbxMesh->GetPolygonCount();

	// ���b�V���̎O�p�`�̐��J�E���g����
	int PrimitiveCount = 0;

	// ���ׂĂ̒��_�̐����擾
	int VertexCount = pFbxMesh->GetPolygonVertexCount();

	// �C���f�b�N�X�o�b�t�@�̐������m��
	WORD* IndexAry = new WORD[VertexCount];
	int* TmpIndexAry = new int[VertexCount];

	// �R���g���[���|�C���g(�C���f�b�N�X�o�b�t�@���w���f�[�^)�̐����擾
	int ControlPointCount = pFbxMesh->GetControlPointsCount();
	ModelData->ControlPointCount = ControlPointCount;
	// �R���g���[���|�C���g�̎擾
	fbxsdk::FbxVector4* pFbxVec = pFbxMesh->GetControlPoints();

	// �R���g���[���|�C���g�̐������������Ɋm��
	D3DXVECTOR3* pVertex = new D3DXVECTOR3[ControlPointCount];



	// ���_�����Z�b�g
	for (int i = 0; i < ControlPointCount; i++)
	{
		pVertex[i].x = (float)pFbxVec[i][0];	// x
		pVertex[i].y = (float)pFbxVec[i][1];	// y
		pVertex[i].z = (float)pFbxVec[i][2];	// z
	}

	// �C���f�b�N�X�����Z�b�g
	memcpy(TmpIndexAry, pFbxMesh->GetPolygonVertices(), sizeof(int) * VertexCount);
	for (int i = 0; i < VertexCount; i++)
	{
		IndexAry[i] = TmpIndexAry[i];
	}
	delete[] TmpIndexAry;

	// ���b�V�����̎O�p�`�̐����擾
	for (int i = 0; i < PolygonCount; i++)
	{
		switch (pFbxMesh->GetPolygonSize(i))
		{
		case 3:
			PrimitiveCount += 1;
			break;
		default:
			// �O�p�`������ĂȂ�������G���[
			MessageBox(NULL, TEXT("�O�p�`������Ă��܂���"), TEXT("�G���["), MB_OK);
			break;

		}
	}





	////-------------------------------------------------------------------------
	////								 �@��
	////-------------------------------------------------------------------------

	//�@���x�N�g�����i�[����ꏊ
	D3DXVECTOR3* pNormalVec = NULL;

	// �@���Z�b�g(�@���f�[�^�̉�)�̐�
	int NormalSetCount = pFbxMesh->GetElementNormalCount();

	for (int i = 0; i < NormalSetCount; i++)
	{
		if (NormalSetCount > 1)
		{
			MessageBox(NULL, TEXT("�@���Z�b�g�͈�����Ή����ĂȂ��ł�"), TEXT("�G���["), MB_OK);
			break;
		}

		// �@���Z�b�g�̎擾
		fbxsdk::FbxGeometryElementNormal* Normal = pFbxMesh->GetElementNormal(i);

		// �}�b�s���O���[�h(�@�����ǂ������ӂ��ɒ�`����Ă邩)�擾
		FbxGeometryElement::EMappingMode MappingMode = Normal->GetMappingMode();

		// ���t�@�����X���[�h(�f�[�^���ǂ̂悤�Ɋi�[����Ă邩)�擾
		FbxGeometryElement::EReferenceMode ReferenceMode = Normal->GetReferenceMode();


		switch (MappingMode)
		{
		case FbxGeometryElement::eNone:
			MessageBox(NULL, TEXT("�}�b�s���O���[�h����`����Ă܂���"), TEXT("�G���["), MB_OK);
			break;
		case FbxGeometryElement::eByControlPoint:			// �R���g���[���|�C���g�S�Ăɏ�񂪂���
			switch (ReferenceMode)
			{
			case FbxGeometryElement::eDirect:
			{

			}
			break;
			case FbxGeometryElement::eIndexToDirect:
			{
				// ������
			}
			break;
			default:
			{
				MessageBox(NULL, TEXT("���t�@�����X���[�h�s���ł�"), TEXT("�G���["), MB_OK);
			}
			break;

			}
			break;
		case FbxGeometryElement::eByPolygonVertex:		// ���_���ׂĂɏ�񂪂���
			switch (ReferenceMode)
			{
			case FbxGeometryElement::eDirect:
			{
				// @todo �Ȃ񂩃~�X���Ƃ�
				pNormalVec = new D3DXVECTOR3[VertexCount];
				for (int i = 0; i < VertexCount; i++)
				{
					pNormalVec[i].x = float(Normal->GetDirectArray().GetAt(i)[0]);
					pNormalVec[i].y = float(Normal->GetDirectArray().GetAt(i)[1]);
					pNormalVec[i].z = float(Normal->GetDirectArray().GetAt(i)[2]);
				}
			}
			break;
			case FbxGeometryElement::eIndexToDirect:
			{
				// ������
			}
			break;
			default:
			{
				MessageBox(NULL, TEXT("���t�@�����X���[�h�s���ł�"), TEXT("�G���["), MB_OK);
			}
			break;
			}

			break;
		default:
			// �ق��̃p�^�[���͍��̂Ƃ����ق��Ăق����c
			MessageBox(NULL, TEXT("�}�b�s���O���[�h���s���ł�"), TEXT("�G���["), MB_OK);
			break;
		}
	}


	//-------------------------------------------------------------------------
	//							�e�N�X�`�����W
	//-------------------------------------------------------------------------

	std::vector<fbxsdk::FbxString>UvSetName;
	std::vector<D3DXVECTOR2*>TextureUv;
	int* UvIndexAry = NULL;

	int UVSetCount = pFbxMesh->GetElementUVCount();

	for (int i = 0; i < UVSetCount; i++)
	{
		// UV�Z�b�g�̎擾
		fbxsdk::FbxGeometryElementUV* UVSet = pFbxMesh->GetElementUV(i);

		// �}�b�s���O���[�h�̎擾
		FbxGeometryElement::EMappingMode mapping = UVSet->GetMappingMode();

		// ���t�@�����X���[�h�擾
		FbxGeometryElement::EReferenceMode reference = UVSet->GetReferenceMode();

		switch (mapping)
		{
		case FbxGeometryElement::eByControlPoint:// eByControlPoint�̃p�^�[���͂�����Ƃ܂���
			switch (reference)
			{
			case FbxGeometryElement::eDirect:
				break;
			case FbxGeometryElement::eIndexToDirect:
				break;
			default:
				break;
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
			switch (reference)
			{
			case FbxGeometryElement::eDirect:
			{
				TextureUv.push_back(new D3DXVECTOR2[VertexCount]);
				for (int n = 0; n < VertexCount; n++)
				{
					TextureUv[i][n].x = float(UVSet->GetDirectArray().GetAt(n)[0]);
					TextureUv[i][n].y = 1.0f - float(UVSet->GetDirectArray().GetAt(n)[1]);
				}

				UvSetName.push_back(UVSet->GetName());
			}
			break;
			case FbxGeometryElement::eIndexToDirect:
			{
				FbxLayerElementArrayTemplate<int>* UvIndex = &UVSet->GetIndexArray();

				/// @todo �Z�b�g��ɑΉ����ĂȂ����烊�[�N����
				TextureUv.push_back(new D3DXVECTOR2[VertexCount]);
				for (int n = 0; n < VertexCount; n++)
				{
					int index = UvIndex->GetAt(n);
					TextureUv[i][n].x = float(UVSet->GetDirectArray().GetAt(index)[0]);
					TextureUv[i][n].y = 1.0f - float(UVSet->GetDirectArray().GetAt(index)[1]);
				}

				UvSetName.push_back(UVSet->GetName());
			}
			break;
			default:
				break;
			}
			break;
		}
	}



	//-------------------------------------------------------------------------
	//							�}�e���A���ƃe�N�X�`��
	//-------------------------------------------------------------------------

	int								TextureFileCount = 0;
	std::vector<const char*>		TextureFileName;
	std::vector<fbxsdk::FbxString>	TextureUvSetName;

	UserMaterial MaterialData;
	ZeroMemory(&MaterialData, sizeof(MaterialData));


	// Node�ɖ߂�
	fbxsdk::FbxNode* Node = pFbxMesh->GetNode();

	// Material�̐����擾����
	int MaterialCount = Node->GetMaterialCount();

	for (int i = 0; i < MaterialCount; i++)
	{
		if (MaterialCount > 1)
		{
			MessageBox(NULL, TEXT("�����̃}�e���A���͂�肠�Ă��܂���"), TEXT("�G���["), MB_OK);
			break;
		}


		// �}�e���A���̎擾
		fbxsdk::FbxSurfaceMaterial* Material = Node->GetMaterial(i);

		if (Material->GetClassId().Is(fbxsdk::FbxSurfaceLambert::ClassId))
		{
			// Lambert�Ƀ_�E���L���X�g
			fbxsdk::FbxSurfaceLambert* lambert = (fbxsdk::FbxSurfaceLambert*)Material;


			// �A���r�G���g
			MaterialData.Ambient.r = (float)lambert->Ambient.Get().mData[0] * (float)lambert->AmbientFactor.Get();
			MaterialData.Ambient.g = (float)lambert->Ambient.Get().mData[1] * (float)lambert->AmbientFactor.Get();
			MaterialData.Ambient.b = (float)lambert->Ambient.Get().mData[2] * (float)lambert->AmbientFactor.Get();
			GetTextureName(lambert, fbxsdk::FbxSurfaceMaterial::sAmbient, &TextureFileName, &TextureUvSetName, &TextureFileCount);

			// �f�B�t���[�Y
			MaterialData.Diffuse.r = (float)lambert->Diffuse.Get().mData[0] * (float)lambert->DiffuseFactor.Get();
			MaterialData.Diffuse.g = (float)lambert->Diffuse.Get().mData[1] * (float)lambert->DiffuseFactor.Get();
			MaterialData.Diffuse.b = (float)lambert->Diffuse.Get().mData[2] * (float)lambert->DiffuseFactor.Get();
			GetTextureName(lambert, fbxsdk::FbxSurfaceMaterial::sDiffuse, &TextureFileName, &TextureUvSetName, &TextureFileCount);

			// �G�~�b�V�u
			MaterialData.Emissive.r = (float)lambert->Emissive.Get().mData[0] * (float)lambert->EmissiveFactor.Get();
			MaterialData.Emissive.g = (float)lambert->Emissive.Get().mData[1] * (float)lambert->EmissiveFactor.Get();
			MaterialData.Emissive.b = (float)lambert->Emissive.Get().mData[2] * (float)lambert->EmissiveFactor.Get();
			GetTextureName(lambert, fbxsdk::FbxSurfaceMaterial::sEmissive, &TextureFileName, &TextureUvSetName, &TextureFileCount);

			// ���ߓx
			MaterialData.Ambient.a = (float)lambert->TransparentColor.Get().mData[0];
			MaterialData.Diffuse.a = (float)lambert->TransparentColor.Get().mData[1];
			MaterialData.Emissive.a = (float)lambert->TransparentColor.Get().mData[2];
			GetTextureName(lambert, fbxsdk::FbxSurfaceMaterial::sTransparentColor, &TextureFileName, &TextureUvSetName, &TextureFileCount);


			GetTextureName(lambert, fbxsdk::FbxSurfaceMaterial::sNormalMap, &TextureFileName, &TextureUvSetName, &TextureFileCount);


		}
		else if (Material->GetClassId().Is(fbxsdk::FbxSurfacePhong::ClassId))
		{
			// Phong�Ƀ_�E���L���X�g
			fbxsdk::FbxSurfacePhong* phong = (fbxsdk::FbxSurfacePhong*)Material;

			/// @todo Phong������Ƃ�

			MessageBox(NULL, TEXT("Material��Phong�ł�(�Ή����Ă��܂���)"), TEXT("�G���["), MB_OK);
		}
		else
		{
			/// @todo Unity�����Ȃǂ̃p�^�[���͍l�����Ȃ�

			// ����͂ق��̃p�^�[���͂����Ă���
			MessageBox(NULL, TEXT("Material���s���ł�"), TEXT("�G���["), MB_OK);
		}

	}



	//-------------------------------------------------------------------------
	//							�擾�����f�[�^���l�߂�
	//-------------------------------------------------------------------------


	// �C���f�b�N�X�`��ƕ�������

	// �f�[�^�Z�b�g
	ModelData->PolygonCount = PolygonCount;
	ModelData->PrimitiveCount = PrimitiveCount;
	ModelData->pVertex = new UserVertex[VertexCount];		// ���݂͂��ׂĂ̒��_�Ԃ�m��
	ModelData->pIndex.IndexAry = IndexAry;
	ModelData->pIndex.IndexCount = VertexCount;


	// ���_�����Z�b�g
	for (int i = 0; i < VertexCount; i++)
	{
		ModelData->pVertex[i].Vec.x = pVertex[IndexAry[i]].x;	// x���W
		ModelData->pVertex[i].Vec.y = pVertex[IndexAry[i]].y;	// y���W
		ModelData->pVertex[i].Vec.z = pVertex[IndexAry[i]].z;	// z���W
	}

	// �@������Ȃ�߂�
	if (pNormalVec != NULL)
	{
		for (int i = 0; i < VertexCount; i++)
		{
			ModelData->pVertex[i].Normal.x = pNormalVec[i].x;	// �@����x�x�N�g��
			ModelData->pVertex[i].Normal.y = pNormalVec[i].y;	// �@����y�x�N�g��
			ModelData->pVertex[i].Normal.z = pNormalVec[i].z;	// �@����z�x�N�g��
		}
	}
	else
	{
		for (int i = 0; i < VertexCount; i++)
		{
			ModelData->pVertex[i].Normal.x = 0;		// �@����x�x�N�g��
			ModelData->pVertex[i].Normal.y = 0;		// �@����y�x�N�g��
			ModelData->pVertex[i].Normal.z = 0;		// �@����z�x�N�g��
		}
	}

	// Uv����Ȃ�l�߂�
	if (TextureUv[0] != NULL)
	{
		for (int i = 0; i < VertexCount; i++)
		{
			ModelData->pVertex[i].tu = TextureUv[0][i].x;	// �e�N�X�`����x���W
			ModelData->pVertex[i].tv = TextureUv[0][i].y;	// �e�N�X�`����y���W
		}
	}
	else
	{
		for (int i = 0; i < VertexCount; i++)
		{
			ModelData->pVertex[i].tu = 0;	// �e�N�X�`����x���W
			ModelData->pVertex[i].tv = 0;	// �e�N�X�`����y���W
		}

	}

	// �}�e���A���̂Ł[��������Ȃ�l�߂�
	if (MaterialCount != 0)
	{
		ModelData->Material = MaterialData;
	}

	for (int i = 0; i < TextureFileCount; i++)
	{
		// Uv�ɓ����Ă����O�ƃe�N�X�`���ɓ����Ă����O���r���Ă���
		for (int n = 0; n < UVSetCount; n++)
		{
			if (TextureUvSetName[i] == UvSetName[n])
			{
				ModelData->pTextureData.push_back(new UserTexture);
				ModelData->pTextureData[i]->TextureName = TextureFileName[i];
				if (FAILED(D3DX11CreateShaderResourceViewFromFile(m_pDevice, ModelData->pTextureData[i]->TextureName, NULL, NULL, &ModelData->pTextureData[i]->pTexture, NULL)))
				{
					ModelData->pTextureData[i]->pTexture = NULL;
				}
			}
		}
	}

	m_pFBXModel->m_pFbxModelData.push_back(ModelData);

	//-------------------------------------------------------------------------
	//								�������
	//-------------------------------------------------------------------------

	delete[] pNormalVec;

	for (unsigned int i = 0; i < TextureUv.size(); i++)
	{
		delete[] TextureUv[i];
	}

	delete[] pVertex;

}

