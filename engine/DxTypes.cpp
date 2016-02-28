#include <Pch.h>
#include <engine/DxTypes.h>
#include <engine/DxHelper.h>
#include <engine/dxDevice.h>
using namespace Engine;

DxDeviceContextState::DxDeviceContextState()
    : m_IAManualPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
{
    ZeroMemory(&m_RSViewport, sizeof(D3D11_VIEWPORT));
}

void DxConstantBuffer::ConstantBuffer::Release()
{
    SafeDeleteArray(cpuMemBuffer);
}

void DxConstantBuffer::Release()
{
    for (auto& cb : m_buffers)
        SafeDeleteArray(cb.cpuMemBuffer);
    m_buffers.resize(0);
    m_constants.resize(0);
    for (auto& db : m_d3dBuffers)
        DxSafeRelease(db);
    m_d3dBuffers.resize(0);
}

int32_t DxConstantBuffer::FindConstantIndexByName(const std::string& constantName) const
{
    const auto newhash = std::hash<std::string>()(constantName);

    const size_t count = m_constants.size();
    for (size_t i = 0; i < count; ++i)
    {
        if (m_constants[i].nameHash == newhash)
            return (int32_t)i;
    }
    return -1;
}

DxConstantBuffer::ConstantBuffer& DxConstantBuffer::GetCBIndexAndOffset(uint32_t cbIndexAndOffset, uint32_t& outOffset)
{
    const uint32_t cbIndex = (cbIndexAndOffset >> 24);
    outOffset = (cbIndexAndOffset & 0x00ffffff);
    return m_buffers[cbIndex];
}

void DxConstantBuffer::SetConstantValue(int32_t constantNdx, const float* values, uint32_t count)
{
    ThrowIfAssert(constantNdx >= 0 && constantNdx < (int32_t)m_constants.size());
    ThrowIfAssert(values != nullptr);
    ThrowIfAssert(count > 0);

    const auto& constant = m_constants[constantNdx];
    ThrowIfAssert(constant.IsFloatArray());

    // gets the constant buffer and the register offset
    uint32_t regOffset = 0;
    auto& cBuffer = GetCBIndexAndOffset(constant.cbIndexAndOffset, regOffset);
    
    const uint32_t memoryOffset = regOffset/sizeof(float);
    float* startMem = cBuffer.cpuMemBuffer + memoryOffset;
    for (uint32_t i = 0; i < count; ++i)
        startMem[i] = values[i];
    cBuffer.dirty = true;
}

void DxConstantBuffer::SetConstantValue(int32_t constantNdx, const bool* values, uint32_t count)
{
    ThrowIfAssert(constantNdx >= 0 && constantNdx < (int32_t)m_constants.size());
    ThrowIfAssert(values != nullptr);
    ThrowIfAssert(count > 0);

    ThrowNotImplemented();
}

void DxConstantBuffer::SetConstantValue(int32_t constantNdx, const int* values, uint32_t count)
{
    ThrowIfAssert(constantNdx >= 0 && constantNdx < (int32_t)m_constants.size());
    ThrowIfAssert(values != nullptr);
    ThrowIfAssert(count > 0);

    ThrowNotImplemented();
}

void DxConstantBuffer::SetConstantValue(int32_t constantNdx, IdTexture texture)
{
    ThrowIfAssert(constantNdx >= 0 && constantNdx < (int32_t)m_constants.size());
    ThrowIfAssert(texture.IsValid());

    auto& constant = m_constants[constantNdx];
    ThrowIfAssert(constant.IsTexture());
    
    uint32_t regOffset = 0;
    auto& cBuffer = GetCBIndexAndOffset(constant.cbIndexAndOffset, regOffset);
    constant.resNumber = texture.Number();
    cBuffer.dirty = true;
}

void DxConstantBuffer::SetConstantValue(int32_t constantNdx, IdSamplerState samplerState)
{
    ThrowIfAssert(constantNdx >= 0 && constantNdx < (int32_t)m_constants.size());
    ThrowIfAssert(samplerState.IsValid());

    auto& constant = m_constants[constantNdx];
    ThrowIfAssert(constant.IsSampler());

    uint32_t regOffset = 0;
    auto& cBuffer = GetCBIndexAndOffset(constant.cbIndexAndOffset, regOffset);
    constant.resNumber = samplerState.Number();
    cBuffer.dirty = true;
}

static eDxShaderConstantType D3D11DimensionToSCT(D3D_SRV_DIMENSION dim)
{
    switch (dim)
    {
    case D3D11_SRV_DIMENSION_BUFFER:
    case D3D11_SRV_DIMENSION_TEXTURE1D:
    case D3D11_SRV_DIMENSION_TEXTURE2D:
    case D3D11_SRV_DIMENSION_TEXTURE2DMS:
        return SCT_TEXTURE;
    case D3D11_SRV_DIMENSION_TEXTURE1DARRAY:
    case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
    case D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY:
        return SCT_TEXTURE2DARRAY;
    case D3D11_SRV_DIMENSION_TEXTURE3D:
        return SCT_TEXTURE3D;
    case D3D11_SRV_DIMENSION_TEXTURECUBE:
        return SCT_CUBEMAP;
    }
    return SCT_NONE;
}

static eDxShaderConstantType D3D11TypeToSCT(const D3D11_SHADER_TYPE_DESC &c)
{
    switch (c.Type)
    {
    case D3D10_SVT_BOOL:
        switch (c.Class)
        {
        case D3D10_SVC_SCALAR:
            return SCT_BOOL;
        case D3D10_SVC_VECTOR:
            if (c.Columns == 2) return SCT_BOOL2;
            if (c.Columns == 3) return SCT_BOOL3;
            return SCT_BOOL4;
        case D3D10_SVC_MATRIX_ROWS: ///< what to do here?
        case D3D10_SVC_MATRIX_COLUMNS:
            return SCT_BOOL4;
        }
        return SCT_BOOL;
    case D3D10_SVT_INT:
        switch (c.Class)
        {
        case D3D10_SVC_SCALAR:
            return SCT_INT;
        case D3D10_SVC_VECTOR:
            if (c.Columns == 2) return SCT_INT2;
            if (c.Columns == 3) return SCT_INT3;
            return SCT_INT4;
        case D3D10_SVC_MATRIX_ROWS: ///< what to do here?
        case D3D10_SVC_MATRIX_COLUMNS:
            return SCT_INT4;
        }
        return SCT_INT;
    case D3D10_SVT_FLOAT:
        switch (c.Class)
        {
        case D3D10_SVC_SCALAR:
            return SCT_FLOAT;
        case D3D10_SVC_VECTOR:
            if (c.Columns == 2) return SCT_FLOAT2;
            if (c.Columns == 3) return SCT_FLOAT3;
            return SCT_FLOAT4;
        case D3D10_SVC_MATRIX_ROWS: ///< what to do here?
        case D3D10_SVC_MATRIX_COLUMNS:
            if (((c.Columns == 3)) && (c.Rows == 4)) return SCT_FLOAT43;
            if ((c.Columns == 3) && (c.Rows == 3)) return SCT_FLOAT33;
            if ((c.Columns == 4) && (c.Rows == 4)) return SCT_FLOAT44;
            return SCT_NONE;
        }
        return SCT_FLOAT;
    case D3D10_SVT_STRING:
        return SCT_STRING;
    case D3D10_SVT_SAMPLER:
        return SCT_SAMPLER;
    case D3D10_SVT_TEXTURE:
    case D3D10_SVT_TEXTURE1D:
    case D3D10_SVT_TEXTURE2D:
        return SCT_TEXTURE;
    case D3D10_SVT_TEXTURE1DARRAY:
    case D3D10_SVT_TEXTURE2DARRAY:
        return SCT_TEXTURE2DARRAY;
    case D3D10_SVT_TEXTURE3D:
        return SCT_TEXTURE3D;
    case D3D10_SVT_TEXTURECUBE:
        return SCT_CUBEMAP;
    }
    return SCT_NONE;
}


void DxConstantBuffer::CreateFromReflector(ID3D11ShaderReflection* pReflector)
{
    Release();
    // ************************* first pass, only TEXTURES and SAMPLERS at the beginning of array *************************
    // general info
    D3D11_SHADER_DESC shDesc;
    pReflector->GetDesc(&shDesc);

    uint32_t noCopied = 0;  // store the actual number of constants retrieved. should be == constantsCount at the end
                            // filling our constants table
    
    DxConstantBuffer::ShaderConstant shConstant;
    for (uint32_t i = 0; i < shDesc.BoundResources; ++i)
    {
        D3D11_SHADER_INPUT_BIND_DESC resDesc;
        pReflector->GetResourceBindingDesc(i, &resDesc);
        // we reflect the textures and samplers only
        if (resDesc.Type != D3D10_SIT_TEXTURE && resDesc.Type != D3D10_SIT_SAMPLER) continue;

#ifdef _DEBUG
        shConstant.name = resDesc.Name;
#endif
        shConstant.nameHash = std::hash<std::string>()(resDesc.Name);
        shConstant.cbIndexAndOffset = resDesc.BindPoint;
        shConstant.sizeInBytes = (int8_t)resDesc.BindCount;
        // is it a texture unit or a sampler state?
        shConstant.type = (resDesc.Type == D3D10_SIT_TEXTURE)
            ? (uint16_t)D3D11DimensionToSCT(resDesc.Dimension)
            : SCT_SAMPLER;
        ThrowIfFailed(shConstant.IsValid() ? S_OK : E_FAIL, L"Invalid constant info copied");

        m_constants.push_back(shConstant);
    } // for

    // ************************* second pass, rest of VARIABLES *************************  
    ConstantBuffer cb;
    for (uint32_t i = 0; i < shDesc.BoundResources; ++i)
    {
        D3D11_SHADER_INPUT_BIND_DESC resDesc;
        pReflector->GetResourceBindingDesc(i, &resDesc);
        if (resDesc.Type != D3D10_SIT_CBUFFER) continue;
        // getting CB and its desc
        ID3D11ShaderReflectionConstantBuffer* pCB = pReflector->GetConstantBufferByName(resDesc.Name);
        D3D11_SHADER_BUFFER_DESC bufDesc; pCB->GetDesc(&bufDesc);
        // internal cbuffers array

        cb.bindPoint = resDesc.BindPoint; // the real shader bind point index
        cb.sizeInBytes = bufDesc.Size;// constant buffer size
                                                // filling each variable
        for (uint32_t v = 0; v < bufDesc.Variables; ++v)
        {
            ID3D11ShaderReflectionVariable* pV = pCB->GetVariableByIndex(v);
            D3D11_SHADER_VARIABLE_DESC varDesc; pV->GetDesc(&varDesc);
            ID3D11ShaderReflectionType* pVT = pV->GetType();
            D3D11_SHADER_TYPE_DESC typeDesc; pVT->GetDesc(&typeDesc);
#ifdef _DEBUG
            shConstant.name = varDesc.Name;
#endif
            shConstant.nameHash = std::hash<std::string>()(varDesc.Name);
            shConstant.cbIndexAndOffset = (uint32_t)(m_buffers.size() << 24) | (varDesc.StartOffset & 0x00ffffff); // 8bits for internal cb index and 24bits for offset inside
            shConstant.type = (int8_t)D3D11TypeToSCT(typeDesc);
            shConstant.sizeInBytes = (int8_t)varDesc.Size;
            ThrowIfFailed(shConstant.IsValid() ? S_OK : E_FAIL, L"Invalid constant info copied");
            m_constants.push_back(shConstant);
        }
        m_buffers.push_back(cb);        
    }//for

    // create d3d buffers and cpu memory buffers
    if (!m_buffers.empty())
    {
        D3D11_BUFFER_DESC bd;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;

        auto dxDev = DxDevice::GetInstance()->GetD3DDevice();
        m_d3dBuffers.resize(m_buffers.size());
        size_t id3dbuf = 0;
        for (auto& cb : m_buffers)
        {
            bd.ByteWidth = cb.sizeInBytes;
            ThrowIfFailed(dxDev->CreateBuffer(&bd, nullptr, &m_d3dBuffers[id3dbuf]));
            ++id3dbuf;

            size_t nFloats = (size_t)std::ceilf(float(cb.sizeInBytes) / sizeof(float));
            ThrowIfAssert(nFloats > 0, L"Invalid number of floats to reserve");
            cb.cpuMemBuffer = new float[nFloats];
        }
    }
}
