#include "pch.h"
#include <engine/DxDeviceContext.h>
#include <engine/dxHelper.h>

using namespace Engine;
using namespace Concurrency;
using namespace DirectX;


DxDeviceContext::DxDeviceContext(ID3D11DeviceContext* deviceContext)
    : m_deviceContext(deviceContext)
{
}
