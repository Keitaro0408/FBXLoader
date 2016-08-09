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
	if (m_pFBXManager == NULL){ MessageBox(NULL, TEXT("FbxManagerクラスの生成に失敗"), TEXT("エラー"), MB_OK); }

	m_pFBXScene = fbxsdk::FbxScene::Create(m_pFBXManager, "");
	if (m_pFBXScene == NULL){ MessageBox(NULL, TEXT("FbxSceneクラスの生成に失敗"), TEXT("エラー"), MB_OK); }

	m_pFBXImporter = fbxsdk::FbxImporter::Create(m_pFBXManager, "");
	if (m_pFBXImporter == NULL){ MessageBox(NULL, TEXT("FbxImporterクラスの生成に失敗"), TEXT("エラー"), MB_OK); }

	m_pFBXIOSettings = fbxsdk::FbxIOSettings::Create(m_pFBXManager, IOSROOT);
	if (m_pFBXIOSettings == NULL){ MessageBox(NULL, TEXT("FbxIOSettingクラスの生成に失敗"), TEXT("エラー"), MB_OK); }
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
	//FBXファイルを開く
	if (m_pFBXImporter->Initialize(_FileName) == false)
	{
		MessageBox(NULL, TEXT("FbxImporterのInitializeに失敗しました"), TEXT("エラー"), MB_OK);
		return false;
	}

	if (m_pFBXImporter->Import(m_pFBXScene) == false)
	{
		MessageBox(NULL, TEXT("FbxImporterのImportに失敗しました"), TEXT("エラー"), MB_OK);
		return false;
	}

	FbxGeometryConverter GeometryConverter(m_pFBXManager);
	if (GeometryConverter.Triangulate(m_pFBXScene, true) == false)
	{
		MessageBox(NULL, TEXT("三角形化に失敗しました。"), TEXT("エラー"), MB_OK);
		return false;
	}
	return true;
}

bool FBXLoader::GetModelData(FBXModel* _pFBXModel)
{
	m_pFBXModel = _pFBXModel;

	// ルートノード(最上位ノード)の取得
	fbxsdk::FbxNode* pRootNode = m_pFBXScene->GetRootNode();
	RecursiveNode(pRootNode);

	return true;
}

void FBXLoader::RecursiveNode(fbxsdk::FbxNode* pNode)
{
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		fbxsdk::FbxNode* pChild = pNode->GetChild(i);  // 子ノードを取得
		RecursiveNode(pChild);
	}

	fbxsdk::FbxNodeAttribute* pAttribute = pNode->GetNodeAttribute();

	if (pAttribute != NULL)
	{
		switch (pAttribute->GetAttributeType())
		{
		case fbxsdk::FbxNodeAttribute::eMesh:
			GetMesh(pAttribute);		//Meshを作成
			break;
		}
	}
}

void FBXLoader::GetMesh(fbxsdk::FbxNodeAttribute* _pAttribute)
{

	FBXModelData* ModelData = new FBXModelData;
	ModelData->pVertex = NULL;

	// ダウンキャスト
	fbxsdk::FbxMesh* pFbxMesh = (fbxsdk::FbxMesh*)_pAttribute;

	//-------------------------------------------------------------------------
	//							頂点情報とインデックス
	//-------------------------------------------------------------------------

	// ポリゴン数を取得する
	int PolygonCount = pFbxMesh->GetPolygonCount();

	// メッシュの三角形の数カウントする
	int PrimitiveCount = 0;

	// すべての頂点の数を取得
	int VertexCount = pFbxMesh->GetPolygonVertexCount();

	// インデックスバッファの数だけ確保
	WORD* IndexAry = new WORD[VertexCount];
	int* TmpIndexAry = new int[VertexCount];

	// コントロールポイント(インデックスバッファが指すデータ)の数を取得
	int ControlPointCount = pFbxMesh->GetControlPointsCount();
	ModelData->ControlPointCount = ControlPointCount;
	// コントロールポイントの取得
	fbxsdk::FbxVector4* pFbxVec = pFbxMesh->GetControlPoints();

	// コントロールポイントの数だけメモリに確保
	D3DXVECTOR3* pVertex = new D3DXVECTOR3[ControlPointCount];



	// 頂点情報をセット
	for (int i = 0; i < ControlPointCount; i++)
	{
		pVertex[i].x = (float)pFbxVec[i][0];	// x
		pVertex[i].y = (float)pFbxVec[i][1];	// y
		pVertex[i].z = (float)pFbxVec[i][2];	// z
	}

	// インデックス情報をセット
	memcpy(TmpIndexAry, pFbxMesh->GetPolygonVertices(), sizeof(int) * VertexCount);
	for (int i = 0; i < VertexCount; i++)
	{
		IndexAry[i] = TmpIndexAry[i];
	}
	delete[] TmpIndexAry;

	// メッシュ内の三角形の数を取得
	for (int i = 0; i < PolygonCount; i++)
	{
		switch (pFbxMesh->GetPolygonSize(i))
		{
		case 3:
			PrimitiveCount += 1;
			break;
		default:
			// 三角形化されてなかったらエラー
			MessageBox(NULL, TEXT("三角形化されていません"), TEXT("エラー"), MB_OK);
			break;

		}
	}





	////-------------------------------------------------------------------------
	////								 法線
	////-------------------------------------------------------------------------

	//法線ベクトルを格納する場所
	D3DXVECTOR3* pNormalVec = NULL;

	// 法線セット(法線データの塊)の数
	int NormalSetCount = pFbxMesh->GetElementNormalCount();

	for (int i = 0; i < NormalSetCount; i++)
	{
		if (NormalSetCount > 1)
		{
			MessageBox(NULL, TEXT("法線セットは一つしか対応してないです"), TEXT("エラー"), MB_OK);
			break;
		}

		// 法線セットの取得
		fbxsdk::FbxGeometryElementNormal* Normal = pFbxMesh->GetElementNormal(i);

		// マッピングモード(法線がどういうふうに定義されてるか)取得
		FbxGeometryElement::EMappingMode MappingMode = Normal->GetMappingMode();

		// リファレンスモード(データがどのように格納されてるか)取得
		FbxGeometryElement::EReferenceMode ReferenceMode = Normal->GetReferenceMode();


		switch (MappingMode)
		{
		case FbxGeometryElement::eNone:
			MessageBox(NULL, TEXT("マッピングモードが定義されてません"), TEXT("エラー"), MB_OK);
			break;
		case FbxGeometryElement::eByControlPoint:			// コントロールポイント全てに情報がある
			switch (ReferenceMode)
			{
			case FbxGeometryElement::eDirect:
			{

			}
			break;
			case FbxGeometryElement::eIndexToDirect:
			{
				// 調査中
			}
			break;
			default:
			{
				MessageBox(NULL, TEXT("リファレンスモード不明です"), TEXT("エラー"), MB_OK);
			}
			break;

			}
			break;
		case FbxGeometryElement::eByPolygonVertex:		// 頂点すべてに情報がある
			switch (ReferenceMode)
			{
			case FbxGeometryElement::eDirect:
			{
				// @todo なんかミスっとる
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
				// 調査中
			}
			break;
			default:
			{
				MessageBox(NULL, TEXT("リファレンスモード不明です"), TEXT("エラー"), MB_OK);
			}
			break;
			}

			break;
		default:
			// ほかのパターンは今のとこ勘弁してほしい…
			MessageBox(NULL, TEXT("マッピングモードが不明です"), TEXT("エラー"), MB_OK);
			break;
		}
	}


	//-------------------------------------------------------------------------
	//							テクスチャ座標
	//-------------------------------------------------------------------------

	std::vector<fbxsdk::FbxString>UvSetName;
	std::vector<D3DXVECTOR2*>TextureUv;
	int* UvIndexAry = NULL;

	int UVSetCount = pFbxMesh->GetElementUVCount();

	for (int i = 0; i < UVSetCount; i++)
	{
		// UVセットの取得
		fbxsdk::FbxGeometryElementUV* UVSet = pFbxMesh->GetElementUV(i);

		// マッピングモードの取得
		FbxGeometryElement::EMappingMode mapping = UVSet->GetMappingMode();

		// リファレンスモード取得
		FbxGeometryElement::EReferenceMode reference = UVSet->GetReferenceMode();

		switch (mapping)
		{
		case FbxGeometryElement::eByControlPoint:// eByControlPointのパターンはちょっとまって
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

				/// @todo セット二つに対応してないからリークする
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
	//							マテリアルとテクスチャ
	//-------------------------------------------------------------------------

	int								TextureFileCount = 0;
	std::vector<const char*>		TextureFileName;
	std::vector<fbxsdk::FbxString>	TextureUvSetName;

	UserMaterial MaterialData;
	ZeroMemory(&MaterialData, sizeof(MaterialData));


	// Nodeに戻る
	fbxsdk::FbxNode* Node = pFbxMesh->GetNode();

	//// materialの数を取得する
	//int materialcount = node->getmaterialcount();

	//for (int i = 0; i < materialcount; i++)
	//{
	//	if (materialcount > 1)
	//	{
	//		messagebox(null, text("複数のマテリアルはわりあてられません"), text("エラー"), mb_ok);
	//		break;
	//	}


	//	// マテリアルの取得
	//	fbxsdk::fbxsurfacematerial* material = node->getmaterial(i);

	//	if (material->getclassid().is(fbxsdk::fbxsurfacelambert::classid))
	//	{
	//		// lambertにダウンキャスト
	//		fbxsdk::fbxsurfacelambert* lambert = (fbxsdk::fbxsurfacelambert*)material;


	//		// materialはカラーとバンプとすぺきゅらーを優先的にしたい

	//		// アンビエント
	//		materialdata.ambient.r = (float)lambert->ambient.get().mdata[0] * (float)lambert->ambientfactor.get();
	//		materialdata.ambient.g = (float)lambert->ambient.get().mdata[1] * (float)lambert->ambientfactor.get();
	//		materialdata.ambient.b = (float)lambert->ambient.get().mdata[2] * (float)lambert->ambientfactor.get();
	//		gettexturename(lambert, fbxsdk::fbxsurfacematerial::sambient, &texturefilename, &textureuvsetname, &texturefilecount);

	//		// ディフューズ
	//		materialdata.diffuse.r = (float)lambert->diffuse.get().mdata[0] * (float)lambert->diffusefactor.get();
	//		materialdata.diffuse.g = (float)lambert->diffuse.get().mdata[1] * (float)lambert->diffusefactor.get();
	//		materialdata.diffuse.b = (float)lambert->diffuse.get().mdata[2] * (float)lambert->diffusefactor.get();
	//		gettexturename(lambert, fbxsdk::fbxsurfacematerial::sdiffuse, &texturefilename, &textureuvsetname, &texturefilecount);

	//		// エミッシブ
	//		materialdata.emissive.r = (float)lambert->emissive.get().mdata[0] * (float)lambert->emissivefactor.get();
	//		materialdata.emissive.g = (float)lambert->emissive.get().mdata[1] * (float)lambert->emissivefactor.get();
	//		materialdata.emissive.b = (float)lambert->emissive.get().mdata[2] * (float)lambert->emissivefactor.get();
	//		gettexturename(lambert, fbxsdk::fbxsurfacematerial::semissive, &texturefilename, &textureuvsetname, &texturefilecount);

	//		// 透過度
	//		materialdata.ambient.a = (float)lambert->transparentcolor.get().mdata[0];
	//		materialdata.diffuse.a = (float)lambert->transparentcolor.get().mdata[1];
	//		materialdata.emissive.a = (float)lambert->transparentcolor.get().mdata[2];
	//		gettexturename(lambert, fbxsdk::fbxsurfacematerial::stransparentcolor, &texturefilename, &textureuvsetname, &texturefilecount);


	//		gettexturename(lambert, fbxsdk::fbxsurfacematerial::snormalmap, &texturefilename, &textureuvsetname, &texturefilecount);
	//	}
	//	else if (material->getclassid().is(fbxsdk::fbxsurfacephong::classid))
	//	{
	//		// phongにダウンキャスト
	//		fbxsdk::fbxsurfacephong* phong = (fbxsdk::fbxsurfacephong*)material;

	//		/// @todo phongもやっとく

	//		messagebox(null, text("materialがphongです(対応していません)"), text("エラー"), mb_ok);
	//	}
	//	else
	//	{
	//		/// @todo unityちゃんなどのパターンは考慮しない

	//		// 現状はほかのパターンはおいておく
	//		messagebox(null, text("materialが不明です"), text("エラー"), mb_ok);
	//	}

	//}



	//-------------------------------------------------------------------------
	//							取得したデータを詰める
	//-------------------------------------------------------------------------


	// インデックス描画と分けたい

	// データセット
	ModelData->PolygonCount = PolygonCount;
	ModelData->PrimitiveCount = PrimitiveCount;
	ModelData->pVertex = new UserVertex[VertexCount];		// 現在はすべての頂点ぶん確保
	ModelData->pIndex.IndexAry = IndexAry;
	ModelData->pIndex.IndexCount = VertexCount;


	// 頂点情報をセット
	for (int i = 0; i < VertexCount; i++)
	{
		ModelData->pVertex[i].Vec.x = pVertex[IndexAry[i]].x;	// x座標
		ModelData->pVertex[i].Vec.y = pVertex[IndexAry[i]].y;	// y座標
		ModelData->pVertex[i].Vec.z = pVertex[IndexAry[i]].z;	// z座標
	}

	// 法線あるならつめる
	if (pNormalVec != NULL)
	{
		for (int i = 0; i < VertexCount; i++)
		{
			ModelData->pVertex[i].Normal.x = pNormalVec[i].x;	// 法線のxベクトル
			ModelData->pVertex[i].Normal.y = pNormalVec[i].y;	// 法線のyベクトル
			ModelData->pVertex[i].Normal.z = pNormalVec[i].z;	// 法線のzベクトル
		}
	}
	else
	{
		for (int i = 0; i < VertexCount; i++)
		{
			ModelData->pVertex[i].Normal.x = 0;		// 法線のxベクトル
			ModelData->pVertex[i].Normal.y = 0;		// 法線のyベクトル
			ModelData->pVertex[i].Normal.z = 0;		// 法線のzベクトル
		}
	}

	// Uvあるなら詰める
	if (TextureUv[0] != NULL)
	{
		for (int i = 0; i < VertexCount; i++)
		{
			ModelData->pVertex[i].tu = TextureUv[0][i].x;	// テクスチャのx座標
			ModelData->pVertex[i].tv = TextureUv[0][i].y;	// テクスチャのy座標
		}
	}
	else
	{
		for (int i = 0; i < VertexCount; i++)
		{
			ModelData->pVertex[i].tu = 0;	// テクスチャのx座標
			ModelData->pVertex[i].tv = 0;	// テクスチャのy座標
		}

	}

	//// マテリアルのでーたがあるなら詰める
	//if (MaterialCount != 0)
	//{
	//	ModelData->Material = MaterialData;
	//}

	for (int i = 0; i < TextureFileCount; i++)
	{
		// Uvに入ってた名前とテクスチャに入ってた名前を比較していく
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
	//								解放処理
	//-------------------------------------------------------------------------



	delete[] pNormalVec;

	for (unsigned int i = 0; i < TextureUv.size(); i++)
	{
		delete[] TextureUv[i];
	}

	delete[] pVertex;
}

