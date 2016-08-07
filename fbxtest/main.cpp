#include <stdio.h>
#include <crtdbg.h>
#include <tchar.h>
#include <windows.h>

#include <d3d11.h>
#include <d3dx11.h>	
#include "vs.h"
#include "ps.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "winmm.lib")

#define CLIENT_WIDTH  1280
#define CLIENT_HEIGHT  720
#define GAME_FPS (1000/60)
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define VERTEXNUM 4 //ポリゴンの頂点数

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

//ポリゴン頂点構造体
struct Vertex3D {
	float pos[3];	//x-y-z
	float col[4];	//r-g-b-a
	float tex[2];	//tu-tv
};

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szStr, INT iCmdShow)
{
	// メモリリーク検出
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	HWND hWnd = NULL;
	MSG msg;
	// ウィンドウの初期化
	static TCHAR* szAppName = _T("DragonKingdom");
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

	//インターフェース取得
	IDXGIDevice1* pDXGI = NULL;
	if (FAILED(pDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDXGI)))
	{
		MessageBox(hWnd, "DX11のインターフェイスの取得に失敗しました。", "Err", MB_ICONSTOP);
		return 1;
	}

	//アダプター取得
	IDXGIAdapter* pAdapter = NULL;
	if (FAILED(pDXGI->GetAdapter(&pAdapter)))
	{
		MessageBox(hWnd, "DX11のアダプターの取得に失敗しました。", "Err", MB_ICONSTOP);
		return 1;
	}

	//ファクトリー取得
	IDXGIFactory* pDXGIFactory = NULL;
	pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDXGIFactory);
	if (pDXGIFactory == NULL)
	{
		MessageBox(hWnd, "DX11のファクトリーの作成に失敗しました。", "Err", MB_ICONSTOP);
		return 1;
	}

	//ALT + Enterでフルスク
	if (FAILED(pDXGIFactory->MakeWindowAssociation(hWnd, 0)))
	{
		MessageBox(hWnd, "フルスクリーン化も失敗しました", "Err", MB_ICONSTOP);
		return 1;
	}

	//スワップチェイン作成
	IDXGISwapChain* pDXGISwpChain = NULL;
	DXGI_SWAP_CHAIN_DESC DXGISwapChainDesc;
	DXGISwapChainDesc.BufferDesc.Width = CLIENT_WIDTH;
	DXGISwapChainDesc.BufferDesc.Height = CLIENT_HEIGHT;
	DXGISwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
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
		MessageBox(hWnd, "スワップチェインの作成に失敗しました。", "Err", MB_ICONSTOP);
		return 1;
	}


	//そのスワップチェインのバックバッファ取得
	ID3D11Texture2D* pBackBuffer = NULL;
	if (FAILED(pDXGISwpChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer)))
	{
		MessageBox(hWnd, "スワップチェインのバックバッファ取得に失敗しました。", "Err", MB_ICONSTOP);
		return 1;
	}

	//そのバックバッファから描画ターゲット生成
	ID3D11RenderTargetView *pRenderTargetView = NULL;
	if (FAILED(pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView))){
		MessageBox(hWnd, "描画ターゲット生成に失敗しました。", "Err", MB_ICONSTOP);
		return 1;
	}

	//更にその描画ターゲットをコンテキストに設定
	pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);

	//ビューポート設定
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
		D3D11_FILL_SOLID, //D3D11_FILL_WIREFRAMEにするとワイヤフレームになる（かっこいい）
		D3D11_CULL_NONE,	//ポリゴンの裏表を無くす
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

	//ラスタライザーをコンテキストに設定
	pDeviceContext->RSSetState(pRasterizerState);

	// 頂点データ(三角ポリゴン1枚)
	Vertex3D VectorData[VERTEXNUM] =
	{
		{ { -0.5f, +0.5f, +0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { +0.5f, +0.5f, +0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, +0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { +0.5f, -0.5f, +0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }
	};

	//頂点レイアウト
	D3D11_INPUT_ELEMENT_DESC InElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	//頂点バッファ作成
	D3D11_BUFFER_DESC BufferDesc;
	BufferDesc.ByteWidth = sizeof(Vertex3D) * VERTEXNUM;
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = sizeof(float);

	D3D11_SUBRESOURCE_DATA SubResourceData;
	SubResourceData.pSysMem = VectorData;
	SubResourceData.SysMemPitch = 0;
	SubResourceData.SysMemSlicePitch = 0;

	ID3D11Buffer* pBuffer;
	if (FAILED(pDevice->CreateBuffer(&BufferDesc, &SubResourceData, &pBuffer)))
	{
		MessageBox(hWnd, _T("CreateBuffer"), _T("Err"), MB_ICONSTOP);
		return 1;
	}

	//その頂点バッファをコンテキストに設定
	UINT Strides = sizeof(Vertex3D);
	UINT Offsets = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Strides, &Offsets);

	//プリミティブ(ポリゴンの形状)をコンテキストに設定
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	//頂点レイアウト作成
	ID3D11InputLayout* pInputLayout = NULL;
	if (FAILED(pDevice->CreateInputLayout(InElementDesc, ARRAYSIZE(InElementDesc), &g_vs_main, sizeof(g_vs_main), &pInputLayout)))
	{
		MessageBox(hWnd, _T("CreateInputLayout"), _T("Err"), MB_ICONSTOP);
		return 1;
	}

	//頂点レイアウトをコンテキストに設定
	pDeviceContext->IASetInputLayout(pInputLayout);

	//頂点シェーダー生成
	ID3D11VertexShader* pVertexShader;
	if (FAILED(pDevice->CreateVertexShader(&g_vs_main, sizeof(g_vs_main), NULL, &pVertexShader))){
		MessageBox(hWnd, _T("CreateVertexShader"), _T("Err"), MB_ICONSTOP);
		return 1;
	}


	//頂点シェーダーをコンテキストに設定
	pDeviceContext->VSSetShader(pVertexShader, NULL, 0);

	//ピクセルシェーダー生成
	ID3D11PixelShader* pPixelShader;
	if (FAILED(pDevice->CreatePixelShader(&g_ps_main, sizeof(g_ps_main), NULL, &pPixelShader)))
	{
		MessageBox(hWnd, _T("CreateVertexShader"), _T("Err"), MB_ICONSTOP);
		return 1;
	}
	//ピクセルシェーダーをコンテキストに設定
	pDeviceContext->PSSetShader(pPixelShader, NULL, 0);

	//テクスチャーを読み込んでいる
	ID3D11ShaderResourceView* pShaderResourceView = NULL;
	if (FAILED(D3DX11CreateShaderResourceViewFromFile(pDevice, _T("test.jpg"), NULL, NULL, &pShaderResourceView, NULL)))
	{
		MessageBox(hWnd, _T("D3DX11CreateShaderResourceViewFromFile"), _T("Err"), MB_ICONSTOP);
		return 1;
	}

	//シェーダーに画像を渡す
	ID3D11ShaderResourceView* pShaderResourceViews[] = { pShaderResourceView };
	pDeviceContext->PSSetShaderResources(0, 1, pShaderResourceViews);

	// メッセージループ
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
			NowTime = timeGetTime();
			if (NowTime - OldTime >= GAME_FPS)
			{
				//描画
				pDeviceContext->Draw(VERTEXNUM, 0);
				//バックバッファをスワップ
				pDXGISwpChain->Present(0, 0);
			}
		}
	}
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

/*  ウインドウプロシージャ  */
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