#include "Game.h"

void GameApp::Init()
{
}

void GameApp::Tick()
{

}

void GameApp::Run()
{
}

bool GameApp::InitDirect3D()
{
	ComPtr<ID3D12Debug> debugController;

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}

	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3dDevice));



	return false;
}
