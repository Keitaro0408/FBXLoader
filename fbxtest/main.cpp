#include <stdio.h>
#include <crtdbg.h>
#include <tchar.h>
#include <windows.h>
#include <xnamath.h>
#include <d3d11.h>
#include <d3dx11.h>	
#include "vs.h"
#include "ps.h"
#include "FBXLoader.h"
#include "FBXModel.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "winmm.lib")

#define CLIENT_WIDTH  1280
#define CLIENT_HEIGHT  720
#define GAME_FPS (1000/60)
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szStr, INT iCmdShow)
{
	// ���������[�N���o
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	HWND hWnd = NULL;
	MSG msg;
	// �E�B���h�E�̏�����
	static TCHAR* szAppName = _T("FBXTest");
	WNDCLASSEX  wndclass;


	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInst;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wndclass);


	hWnd = CreateWindow(szAppName, szAppName, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		GetSystemMetrics(SM_CXSCREEN) / 2 - CLIENT_WIDTH / 2,
		GetSystemMetrics(SM_CYSCREEN) / 2 - CLIENT_HEIGHT / 2,
		CLIENT_WIDTH, CLIENT_HEIGHT, NULL, NULL, hInst, NULL);


	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	SetWindowText(hWnd, _T("DX11Sample"));

	DWORD NowTime = timeGetTime();
	DWORD OldTime = timeGetTime();

	ID3D11Device* pDevice = NULL;
	ID3D11DeviceContext* pDeviceContext = NULL;

	if (FAILED(D3D11CreateDevice(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL, 0, NULL, 0,
		D3D11_SDK_VERSION,
		&pDevice, NULL,
		&pDeviceContext)))
	{
		MessageBox(hWnd, "D3D11CreateDevice", "Err", MB_ICONSTOP);
		return 1;
	}

	//�C���^�[�t�F�[�X�擾
	IDXGIDevice1* pDXGI = NULL;
	if (FAILED(pDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDXGI)))
	{
		MessageBox(hWnd, "DX11�̃C���^�[�t�F�C�X�̎擾�Ɏ��s���܂����B", "Err", MB_ICONSTOP);
		return 1;
	}

	//�A�_�v�^�[�擾
	IDXGIAdapter* pAdapter = NULL;
	if (FAILED(pDXGI->GetAdapter(&pAdapter)))
	{
		MessageBox(hWnd, "DX11�̃A�_�v�^�[�̎擾�Ɏ��s���܂����B", "Err", MB_ICONSTOP);
		return 1;
	}

	//�t�@�N�g���[�擾
	IDXGIFactory* pDXGIFactory = NULL;
	pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDXGIFactory);
	if (pDXGIFactory == NULL)
	{
		MessageBox(hWnd, "DX11�̃t�@�N�g���[�̍쐬�Ɏ��s���܂����B", "Err", MB_ICONSTOP);
		return 1;
	}
	//ALT + Enter�Ńt���X�N
	if (FAILED(pDXGIFactory->MakeWindowAssociation(hWnd, 0)))
	{
		MessageBox(hWnd, "�t���X�N���[���������s���܂���", "Err", MB_ICONSTOP);
		return 1;
	}

	//�X���b�v�`�F�C���쐬
	IDXGISwapChain* pDXGISwpChain = NULL;
	DXGI_SWAP_CHAIN_DESC DXGISwapChainDesc;
	DXGISwapChainDesc.BufferDesc.Width = CLIENT_WIDTH;
	DXGISwapChainDesc.BufferDesc.Height = CLIENT_HEIGHT;
	DXGISwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	DXGISwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	DXGISwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	DXGISwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	DXGISwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	DXGISwapChainDesc.SampleDesc.Count = 1;
	DXGISwapChainDesc.SampleDesc.Quality = 0;
	DXGISwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	DXGISwapChainDesc.BufferCount = 1;
	DXGISwapChainDesc.OutputWindow = hWnd;
	DXGISwapChainDesc.Windowed = TRUE;
	DXGISwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	DXGISwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (FAILED(pDXGIFactory->CreateSwapChain(pDevice, &DXGISwapChainDesc, &pDXGISwpChain)))
	{
		MessageBox(hWnd, "�X���b�v�`�F�C���̍쐬�Ɏ��s���܂����B", "Err", MB_ICONSTOP);
		return 1;
	}


	//���̃X���b�v�`�F�C���̃o�b�N�o�b�t�@�擾
	ID3D11Texture2D* pBackBuffer = NULL;
	if (FAILED(pDXGISwpChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer)))
	{
		MessageBox(hWnd, "�X���b�v�`�F�C���̃o�b�N�o�b�t�@�擾�Ɏ��s���܂����B", "Err", MB_ICONSTOP);
		return 1;
	}

	//���̃o�b�N�o�b�t�@����`��^�[�Q�b�g����
	ID3D11RenderTargetView *pRenderTargetView = NULL;
	if (FAILED(pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView))){
		MessageBox(hWnd, "�`��^�[�Q�b�g�����Ɏ��s���܂����B", "Err", MB_ICONSTOP);
		return 1;
	}

	//--------------------
	// �o�b�N�o�b�t�@�ƃf�v�X�X�e���V���o�b�t�@
	//--------------------
	ID3D11Texture2D* pDepthStencilBuffer;			// �[�x�X�e���V���o�b�t�@
	ID3D11DepthStencilView* pDepthStencilView;		// �[�x�X�e���V���r���[
	//�f�v�X�X�e���V���r���[
	D3D11_TEXTURE2D_DESC DepthDesc;
	DepthDesc.Width = CLIENT_WIDTH;
	DepthDesc.Height = CLIENT_HEIGHT;
	DepthDesc.MipLevels = 1;
	DepthDesc.ArraySize = 1;
	DepthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DepthDesc.SampleDesc.Count = 1;
	DepthDesc.SampleDesc.Quality = 0;
	DepthDesc.Usage = D3D11_USAGE_DEFAULT;
	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthDesc.CPUAccessFlags = 0;
	DepthDesc.MiscFlags = 0;
	pDevice->CreateTexture2D(&DepthDesc, NULL, &pDepthStencilBuffer);
	pDevice->CreateDepthStencilView(pDepthStencilBuffer, NULL, &pDepthStencilView);

	// �����_�[�^�[�Q�b�g�r���[�ƃf�v�X�X�e���V���r���[���Z�b�g
	pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

	//�r���[�|�[�g�ݒ�
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = CLIENT_WIDTH;
	vp.Height = CLIENT_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pDeviceContext->RSSetViewports(1, &vp);

	ID3D11RasterizerState* pRasterizerState = NULL;
	D3D11_RASTERIZER_DESC RasterizerDesc = {
		D3D11_FILL_SOLID, //D3D11_FILL_WIREFRAME�ɂ���ƃ��C���t���[���ɂȂ�i�����������j 
		D3D11_CULL_BACK,	//�|���S���̗��\�𖳂���
		FALSE,
		0,
		0.0f,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		FALSE
	};
	if (FAILED(pDevice->CreateRasterizerState(&RasterizerDesc, &pRasterizerState))){
		MessageBox(hWnd, _T("CreateRasterizerState"), _T("Err"), MB_ICONSTOP);
		return 1;
	}

	//���X�^���C�U�[���R���e�L�X�g�ɐݒ�
	pDeviceContext->RSSetState(pRasterizerState);

	FBXLoader fbxLoader(pDevice);
	fbxLoader.FileLoad("fbx//house_seleb.fbx");
	FBXModel  testModel(pDevice,pDeviceContext);
	fbxLoader.GetModelData(&testModel);
	
	// ���b�Z�[�W���[�v
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			pDeviceContext->ClearRenderTargetView(pRenderTargetView, ClearColor);
			pDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
			//�`��
			testModel.Draw();
			//�o�b�N�o�b�t�@���X���b�v
			pDXGISwpChain->Present(1, 0);
		}
	}
	pDepthStencilBuffer->Release();
	pDepthStencilView->Release();
	SAFE_RELEASE(pRenderTargetView);
	SAFE_RELEASE(pBackBuffer);
	SAFE_RELEASE(pDXGISwpChain);
	SAFE_RELEASE(pDXGIFactory);
	SAFE_RELEASE(pAdapter);
	SAFE_RELEASE(pDXGI);
	SAFE_RELEASE(pDeviceContext);
	SAFE_RELEASE(pDevice);
	return (INT)msg.wParam;
}

/*  �E�C���h�E�v���V�[�W��  */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return  0;
		break;
	case WM_KEYDOWN:
		switch ((CHAR)wparam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
			break;
		}
		break;
	case WM_ACTIVATE:
		switch ((CHAR)wparam)
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:

			break;
		}
	}
	return  DefWindowProc(hwnd, message, wparam, lparam);
}