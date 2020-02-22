#include "stdafx.h"
#include "D3DGameEngine.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	D3DGameEngine game(1280, 720, L"D3D12 Hello World!");
	return Win32Application::Run(&game, hInstance, nCmdShow);
}
