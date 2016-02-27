#include <Pch.h>
#include <engine/DxTypes.h>
#include <engine/DxHelper.h>
using namespace Engine;

DxDeviceContextState::DxDeviceContextState()
    : m_IAManualPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
{
    ZeroMemory(&m_RSViewport, sizeof(D3D11_VIEWPORT));
}

void DxConstantBuffer::ConstantBuffer::Release()
{
    cpuMemBuffer.resize(0);
}

void DxConstantBuffer::Release()
{
    m_buffers.resize(0);
    m_constantsDesc.resize(0);
    for (auto& db : m_d3dBuffers)
        DxSafeRelease(db);
}

int32_t DxConstantBuffer::SetConstantValue(int32_t constantNdx, const float* values, uint32_t count)
{
    return -1;
}

int32_t DxConstantBuffer::SetConstantValue(int32_t constantNdx, const bool* values, uint32_t count)
{
    return -1;
}

int32_t DxConstantBuffer::SetConstantValue(int32_t constantNdx, const int* values, uint32_t count)
{
    return -1;
}

int32_t DxConstantBuffer::SetConstantValue(int32_t constantNdx, IdTexture texture, uint32_t count)
{
    return -1;
}

int32_t DxConstantBuffer::SetConstantValue(int32_t constantNdx, IdSamplerState samplerState, uint32_t count)
{
    return -1;
}

int32_t  DxConstantBuffer::FindConstantIndexByName(const std::wstring& constantName) const
{
    return -1;
}

eDxShaderConstantType D3D11DimensionToSCT(D3D_SRV_DIMENSION dim)
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

eDxShaderConstantType D3D11TypeToSCT(const D3D11_SHADER_TYPE_DESC &c)
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
        shConstant.index = resDesc.BindPoint;
        shConstant.sizeInBytes = (int8_t)resDesc.BindCount;
        // is it a texture unit or a sampler state?
        shConstant.type = (resDesc.Type == D3D10_SIT_TEXTURE)
            ? (uint16_t)D3D11DimensionToSCT(resDesc.Dimension)
            : SCT_SAMPLER;
        ThrowIfFailed(shConstant.IsValid() ? S_OK : E_FAIL, L"Invalid constant info copied");

        m_constantsDesc.push_back(shConstant);
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
            shConstant.index = (m_buffers.size() << 24) | (varDesc.StartOffset & 0x00ffffff); // 8bits for internal cb index and 24bits for offset inside
            shConstant.type = (int8_t)D3D11TypeToSCT(typeDesc);
            shConstant.sizeInBytes = (int8_t)varDesc.Size;
            ThrowIfFailed(shConstant.IsValid() ? S_OK : E_FAIL, L"Invalid constant info copied");
            m_constantsDesc.push_back(shConstant);
        }
        m_buffers.push_back(cb);
        
    }//for

    for (auto& cb : m_buffers)
    {


    }    
}
