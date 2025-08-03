//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"

#include <memory>

// A basic renderer implementation that creates a D3D12 device and
// provides rendering functionality.
class Renderer final : public DX::IDeviceNotify
{
public:
    Renderer() noexcept(false);
    ~Renderer();

    Renderer(Renderer&&) = default;
    Renderer& operator=(Renderer&&) = default;

    Renderer(Renderer const&) = delete;
    Renderer& operator=(Renderer const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const noexcept;

private:
    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void CreateTriangleResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources> m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer m_timer;

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    // std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;

    // Triangle rendering resources
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
};
