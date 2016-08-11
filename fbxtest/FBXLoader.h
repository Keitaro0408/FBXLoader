#ifndef _FBXLOADER_
#define _FBXLOADER_
#include <fbxsdk.h>
#include <d3d11.h>
#include <d3dx11.h>	
#include <vector>
class FBXModel;

class FBXLoader
{
public:
	FBXLoader(ID3D11Device* _pDevice);
	~FBXLoader();
	bool FileLoad(char* _FileName);
	bool GetModelData(FBXModel* _pFBXModel);

private:
	FBXModel*				m_pFBXModel;
	fbxsdk::FbxManager*		m_pFBXManager;
	fbxsdk::FbxScene*		m_pFBXScene;
	fbxsdk::FbxImporter*	m_pFBXImporter;
	fbxsdk::FbxIOSettings*	m_pFBXIOSettings;

	ID3D11Device* m_pDevice;

	void GetTextureName(
		fbxsdk::FbxSurfaceMaterial* material,
		const char* matFlag,
		std::vector<const char*>* pOutFileName,
		std::vector<fbxsdk::FbxString>* pOutUvSetName,
		int* OutCount);
	void RecursiveNode(fbxsdk::FbxNode* pNode);
	void GetMesh(fbxsdk::FbxNodeAttribute* _pAttribute);

};
#endif