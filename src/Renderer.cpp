//
// Game.cpp
//

#include "pch.h"
#include "Renderer.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Renderer::Renderer() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    //   Add DX::DeviceResources::c_ReverseDepth to optimize depth buffer clears for 0 instead of 1.
    m_deviceResources->RegisterDeviceNotify(this);
}

Renderer::~Renderer()
{
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
}

// Initialize the Direct3D resources required to run.
void Renderer::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable
    // timestep mode. e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Renderer::Tick()
{
    m_timer.Tick(
        [&]()
        { Update(m_timer); }
    );

    Render();
}

// Updates the world.
void Renderer::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Renderer::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    // Prepare the command list to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    // Set pipeline state
    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    // Set primitive topology
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set vertex buffer
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    // Draw triangle
    commandList->DrawInstanced(3, 1, 0, 0);

    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    // m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());

    PIXEndEvent(m_deviceResources->GetCommandQueue());
}

// Helper method to clear the back buffers.
void Renderer::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    const auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    const auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    const auto viewport = m_deviceResources->GetScreenViewport();
    const auto scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    PIXEndEvent(commandList);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Renderer::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Renderer::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Renderer::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Renderer::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Renderer::OnWindowMoved()
{
    const auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Renderer::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Renderer::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Renderer::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Renderer::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Check Shader Model 6 support
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {D3D_SHADER_MODEL_6_0};
    if (
        FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) ||
        (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0)
    )
    {
#ifdef _DEBUG
        OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
#endif
        throw std::runtime_error("Shader Model 6.0 is not supported!");
    }

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    // m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

    // TODO: Initialize device dependent objects here (independent of window size).

    // Create triangle rendering resources
    CreateTriangleResources();
}

// Allocate all memory resources that change on a window SizeChanged event.
void Renderer::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void Renderer::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    // Clean up triangle resources
    m_pipelineState.Reset();
    m_rootSignature.Reset();
    m_vertexBuffer.Reset();

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    // m_graphicsMemory.reset();
}

void Renderer::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}

void Renderer::CreateTriangleResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Define triangle vertices
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
    };

    Vertex triangleVertices[] =
        {
            {DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f)},  // Top
            {DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f)}, // Bottom right
            {DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f)} // Bottom left
        };

    const UINT vertexBufferSize = sizeof(triangleVertices);

    // Create vertex buffer
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

    DX::ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &vertexBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_vertexBuffer.GetAddressOf())
    ));

    // Copy vertex data to the vertex buffer
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;

    // Create root signature
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    DX::ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    DX::ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

    // Compile shaders
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    DX::ThrowIfFailed(D3DCompileFromFile(L"Assets\\shaders\\TriangleVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
    DX::ThrowIfFailed(D3DCompileFromFile(L"Assets\\shaders\\TrianglePixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

    // Define the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };

    // Create graphics pipeline state
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
    psoDesc.SampleDesc.Count = 1;

    DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

#pragma endregion
