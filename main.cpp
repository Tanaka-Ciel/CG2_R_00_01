#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")


// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam) {
	// ���b�Z�[�W�ɉ����ăQ�[���ŗL�̏������s��
	switch (msg)
	{
		// �E�B���h�E���j�󂳂ꂽ
	case WM_DESTROY:
		// OS�ɑ΂��āA�A�v���̏I����`����
		PostQuitMessage(0);
		return 0;
	
	}

	// �W���̃��b�Z�[�W�������s��
	return DefWindowProc(hwnd, msg, wparam, lparam);

}


void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}


std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);

	return result;
}


std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);

	return result;
}


// Windows�A�v���ł̃G���g���[�|�C���g(main�֐�)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	WNDCLASS wc{};
	// �E�B���h�E�v���V�[�W��
	wc.lpfnWndProc = WindowProc;
	// �E�B���h�E�N���X
	wc.lpszClassName = L"CG2WindowClass";
	// �C���X�^���X�n���h��
	wc.hInstance = GetModuleHandle(nullptr);
	// �J�[�\��
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// �E�B���h�E�N���X��o�^����
	RegisterClass(&wc);

	// �N���C�A���g�̈�̃T�C�Y
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	// �E�B���h�E�T�C�Y��\���\���̂ɃN���C�A���g�̈������
	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	// �N���C�A���g�̈�����Ɏ��ۂ̃T�C�Y��wrc��ύX���Ă��炤
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// �E�B���h�E�̐���
	HWND hwnd = CreateWindow(
		wc.lpszClassName,       // ���p����N���X��
		L"CG2"                 ,// �^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,    // �悭����E�B���h�E�X�^�C��
		CW_USEDEFAULT,           // �\��X���W
		CW_USEDEFAULT,           // �\��Y���W
		wrc.right - wrc.left,   // �E�B���h�E����
		wrc.bottom - wrc.top,   // �E�B���h�E�c��
		nullptr,                // �e�E�B���h�E�n���h��
		nullptr,                // ���j���[�n���h��
		wc.hInstance,           // �C���X�^���X�n���h��
		nullptr);               // �I�v�V����

	// �E�B���h�E��\������
	ShowWindow(hwnd, SW_SHOW);

	IDXGIFactory7* dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	IDXGIAdapter4* useAdapter = nullptr;
	for (UINT i = 0; dxgiFactory -> EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	assert(useAdapter != nullptr);

	ID3D12Device* device = nullptr;
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };
	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr))
		{
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	assert(device != nullptr);
	Log("Complete create D3D12Device!!!\n");

	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc,
	IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(hr));

	ID3D12CommandAllocator* commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(hr));

	ID3D12GraphicsCommandList* commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr,
		IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(hr));

	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth;
	swapChainDesc.Height = kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	assert(SUCCEEDED(hr));

	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr));

	ID3D12Resource* swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	// RTV�̐ݒ� 
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // �o�͌��ʂ�SRGB�ɕϊ����ď������� 
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2d�e�N�X�`���Ƃ��ď������� 
	// �f�B�X�N���v�^�̐擪���擾���� 
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	// RTV��2���̂Ńf�B�X�N���v�^��2�p�� 
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	// �܂�1�ڂ����B1�ڂ͍ŏ��̂Ƃ���ɍ��B���ꏊ��������Ŏw�肵�Ă�����K�v������ 
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
	// 2�ڂ̃f�B�X�N���v�^�n���h���𓾂�(���͂�) 
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2�ڂ���� 
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);

	// ���ꂩ�珑�����ރo�b�N�o�b�t�@�̃C���f�b�N�X���擾 
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	// �`����RTV��ݒ肷�� 
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	// �w�肵���F�ŉ�ʑS�̂��N���A���� 
	float clearColor[] = { 0.1f, 0.25, 0.5f, 1.0f };
	// ���ۂ��F�BRGBA�̏� 
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
	// �R�}���h���X�g�̓��e���m�肳����B���ׂẴR�}���h��ς�ł���Close���邱�� 
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	// GPU�ɃR�}���h���X�g�̎��s���s�킹�� 
	ID3D12CommandList* commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(1, commandLists);
	// GPU��OS�ɉ�ʂ̌������s���悤�ʒm���� 
	swapChain->Present(1, 0);
	//���̃t���[���p�̃R�}���h���X�g������ 
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator, nullptr);
	assert(SUCCEEDED(hr));

	MSG msg{};
	//�E�B���h�E�́~�{�^�����������܂Ń��[�v
	while (msg.message != WM_QUIT) {
		//Window�Ƀ��b�Z�[�W�����Ă���ŗD��ŏ���������
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}else {
			//�Q�[���̏���
		}
	}

	// �o�̓E�B���h�E�ւ̕����o��
	OutputDebugStringA("Hello,DirectX!/n");


	Log(ConvertString(std::format(L"---------------------------WSTRING{}\n", L"abc")));

	return 0;
}