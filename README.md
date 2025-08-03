# MSBuild+VCPKG integration

This project is a demonstration of a fully standalone method for making use of the *Microsoft GDK* without having to install anything. It makes use of the [vc Package Manager](https://aka.ms/vcpkg) to add the packaged versions of *Microsoft GDK*, the *DirectX 12 Agility SDK*, etc. This can be used as a baseline to ship in the Microsoft Store and the Xbox PC App.

# Newly created project

* Precompiled header files
  * pch.cpp
  * pch.h

* Main application entry-point and classic Windows procedure function
  * Main.cpp

* Timer helper class
  * StepTimer.h

* The Game class
  * Game.cpp
  * Game.h

* The Direct3D 12 device and swapchain class
  * DeviceResources.cpp
  * DeviceResources.h

* The Microsoft Game configuration file
  * MicrosoftGameConfig.mgc

* Resources
  * xbox.ico
  * resource.rc
  * settings.manifest
  * MGC image assets

* vcpkg 'manifest-mode' integration
  * vcpkg.json
  * vcpkg-configuration.json

For a detailed description of the C++ source in the template, see [GitHub](https://github.com/microsoft/DirectXTK12/wiki/Using-DeviceResources#tour-of-the-code).

# Microsoft GDK

To add PlayFab libraries to the project, enable the *playfab* feature in the **ms-gdk** port in ``vcpkg.json`` by replacing the `"ms-gdk",` entry with the following:

```
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "dependencies": [
...
    {
      "name": "ms-gdk",
      "features": [
        "playfab"
      ]
    },
    "winpixevent"
  ]
}
```

Then uncomment in the **pch.h** the include headers:

```cpp
#include <playfab/core/PFErrors.h>
#include <playfab/services/PFServices.h>
```

# VCPKG integration

The project makes use of Visual Studio 2022 **Microsoft.VisualStudio.Component.vcpkg**. This requires Visual Studio 2022 v17.6 or later.

When the project is created, the 'head commit id' is taken from the vcpkg GitHub project making it use the 'latest' available at that time. To move to newer versions of the ports, update the baseline hash in ``vcpkg-configuration.json``. For example, the following sets the baseline to match the June 2025 release of the registry on [GitHub](https://github.com/microsoft/vcpkg/releases):

```
{
    "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg-configuration.schema.json",
    "default-registry": {
      "kind": "builtin",
      "baseline": "ef7dbf94b9198bc58f45951adcf1f041fcbc5ea0"
    }
}
```

The triplet and host triplet are explicitly set in the project as they are needed to access the DXC shader compiler at build time. If you want to use static libraries where possible rather than DLLs, use the [vcpkg property sheet](https://devblogs.microsoft.com/cppblog/vcpkg-is-now-included-with-visual-studio/) and change ``x64-windows`` to ``x64-windows-static-md``.

To use a specific version of a port, update ``vcpkg.json`` with an *overrides* section:

```
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "dependencies": [
...
  ],
  "overrides": [
    {
      "name": "ms-gdk",
      "version": "2410.2.1916"
    }
  ]
}
```

> The **directx-dxc** port is listed twice by design. The 'host: true' case is for build-time usage. The second entry makes the DXC API available at runtime.

# Rendering a simple triangle

We can quickly add a simple triangle render to the template using the [DirectX Tool Kit](https://github.com/microsoft/DirectXTK12/wiki).

1. Edit the ``vcpkg.json`` and add a entry for the **directxtk12** port:

```
{
...
    "directxmath",
    {
      "name": "directxtk12",
      "default-features": false,
      "features": [
        "gameinput"
      ]
    },
    "dstorage",
...
}
```

> Note that this specifically opts in to using [GameInput](http://aka.ms/gameinput) for the GamePad, Keyboard, and Mouse implementation.

2. In the **pch.h** header uncomment the include header and then add a few more required headers:

```cpp
#define DIRECTX_TOOLKIT_IMPORT
#include <directxtk12/CommonStates.h>
#include <directxtk12/Effects.h>
#include <directxtk12/GraphicsMemory.h>
#include <directxtk12/PrimitiveBatch.h>
#include <directxtk12/SimpleMath.h>
#include <directxtk12/VertexTypes.h>
```

> If using the `x64-windows-static-md` triplet for static libraries, don't add the `#define DIRECTX_TOOLKIT_IMPORT`.

3. In the **Game.h** header, uncomment the variable near the bottom of the class declaration and then add a few more required variables:

```cpp
std::unique_ptr<DirectX::GraphicsMemory>    m_graphicsMemory;

using VertexType = DirectX::VertexPositionColor;

std::unique_ptr<DirectX::BasicEffect>       m_effect;
std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;
```

4. In the **Game.cpp** source file, near the top of the file, add another using statement for the *SimpleMath* namespace:

```cpp
using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;
```

5. Modify the **Render** method in **Game.cpp** by replacing the TODO comment with the following:

```cpp
// Add your rendering code here.
m_effect->Apply(commandList);

m_batch->Begin(commandList);

VertexPositionColor v1(Vector3(0.f, 0.5f, 0.5f), Colors::Red);
VertexPositionColor v2(Vector3(0.5f, -0.5f, 0.5f), Colors::Green);
VertexPositionColor v3(Vector3(-0.5f, -0.5f, 0.5f), Colors::Blue);

m_batch->DrawTriangle(v1, v2, v3);

m_batch->End();
```

Be sure to also uncomment the line after the call to `Present`:

```cpp
m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
```

6. Modify the **CreateDeviceDependentResources** method in **Game.cpp** by uncommmenting the line and replacing the TODO comment with the following:

```cpp
m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

// Initialize device dependent objects here (independent of window size).
const RenderTargetState rtState(m_deviceResources->GetBackBufferFormat(),
    m_deviceResources->GetDepthBufferFormat());

m_batch = std::make_unique<PrimitiveBatch<VertexType>>(device);

EffectPipelineStateDescription pd(
    &VertexType::InputLayout,
    CommonStates::Opaque,
    CommonStates::DepthDefault,
    CommonStates::CullNone,
    rtState);

m_effect = std::make_unique<BasicEffect>(device, EffectFlags::VertexColor, pd);
```

7. Modify the **OnDeviceLost** method in **Game.cpp** by uncommmenting the line and replacing the TODO comment with the following:

```cpp
// Add Direct3D resource cleanup here.
m_effect.reset();
m_batch.reset();

m_graphicsMemory.reset();
```

8. Then build & run the project.

> For more things you can try out with *DirectX Tool Kit*, see the [GitHub wiki](https://github.com/microsoft/DirectXTK12/wiki/Simple-rendering). You may need to add more include *directxtk12* headers to the **pch.h**. For *DirectX Tool Kit for Audio*, you also need to add "xaudio2-9" or "xaudio2redist" to the list of features for the *directxtk12* port in `vcpkg.json`.

# Packaging

A PowerShell script provides a simple method for creating a loose layout and packaging after making the build.

```
powershell -File PackageLayout.ps1 -Destination layout -Configuration Release
```

> The file ``PackageLayout.flt`` lists string patterns of filenames to exclude such as ".exp" and ".pdb"

# Known Issues

* Depending on where the project is created, the overall path-length with the vcpkg_installed and triplet folders can exceed ``_MAX_PATH``. If this happens, move the project to a more 'shallow' parent directory location.

* IntelliSense may still show the headers coming from VCPKG as 'unknown' even after building and generating the `vcpkg_installed` folder. This can be resolved by unloading and reloading the project.

* The project initializes the GameRuntime as part of startup. If it is not present, you will see an error that reads "Game Runtime is not installed on this sytem or needs updating". To fix this, run the [Gaming Services Repair Tool for PC](https://aka.ms/GamingRepairTool) or use *winget*:

```
winget install 9MWPM2CQNLHN -s msstore
```

# Further reading

[Game Development Kit (GDK) documentation](http://aka.ms/gdkdocs)

[DirectX 12 Agility SDK](https://aka.ms/directx12agility)

[StepTimer](https://walbourn.github.io/understanding-game-time-revisited/)
