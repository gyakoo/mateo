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
    deviceBuffer = nullptr;
    cpuMemBuffer.resize(0);
}

void DxConstantBuffer::release()
{
    m_buffers.resize(0);
    m_constantsDesc.resize(0);
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
        shConstant.size = (int8_t)resDesc.BindCount;
        // is it a texture unit or a sampler state?
        shConstant.type = (resDesc.Type == D3D10_SIT_TEXTURE)
            ? (uint16_t)D3D11DimensionToSCT(resDesc.Dimension)
            : SCT_SAMPLER;
        ThrowIfFailed(shConstant.IsValid() ? S_OK : E_FAIL, L"Invalid constant info copied");

        m_constantsDesc.push_back(shConstant);
    } // for

/*
      // ************************* second pass, rest of VARIABLES *************************
      // create internal dx11 c.buffers array
    if (noShaderCBs)
    {
        cbsCount = noShaderCBs;
        cbs = new DX11CBuffer[noShaderCBs];
        // helper array for batch d3d11 api calls
        arrayOfDX11CBuffers = new ID3D11Buffer*[noShaderCBs];
        ZeroMemory(arrayOfDX11CBuffers, PtrSize*noShaderCBs);
    }
    uint32_t cbIndex = 0;   // current internal cb index
    for (uint32_t i = 0; i < shDesc.BoundResources; ++i)
    {
        D3D11_SHADER_INPUT_BIND_DESC resDesc;
        pReflector->GetResourceBindingDesc(i, &resDesc);
        if (resDesc.Type != D3D10_SIT_CBUFFER) continue;
        // getting CB and its desc
        ID3D11ShaderReflectionConstantBuffer* pCB = pReflector->GetConstantBufferByName(resDesc.Name);
        D3D11_SHADER_BUFFER_DESC bufDesc; pCB->GetDesc(&bufDesc);
        // internal cbuffers array
        cbs[cbIndex].index = resDesc.BindPoint; // the real shader bind point index
        cbs[cbIndex].sizeInBytes = bufDesc.Size;// constant buffer size
                                                // filling each variable
        for (uint32_t v = 0; v < bufDesc.Variables; ++v)
        {
            ID3D11ShaderReflectionVariable* pV = pCB->GetVariableByIndex(v);
            D3D11_SHADER_VARIABLE_DESC varDesc; pV->GetDesc(&varDesc);
            ID3D11ShaderReflectionType* pVT = pV->GetType();
            D3D11_SHADER_TYPE_DESC typeDesc; pVT->GetDesc(&typeDesc);
#ifdef _DEBUG
            strncpy_s(srcCBuff->constants[noCopied].name, varDesc.Name, gyShaderConstant::MAX_CONSTANT_NAME);
            srcCBuff->constants[noCopied].name[gyShaderConstant::MAX_CONSTANT_NAME - 1] = 0;
#endif
            srcCBuff->constants[noCopied].nameHash = gyComputeHash((void*)varDesc.Name, (uint32_t)strlen(varDesc.Name));
            srcCBuff->constants[noCopied].index = (cbIndex << 24) | (varDesc.StartOffset & 0x00ffffff); // 8bits for internal cb index and 24bits for offset inside
            srcCBuff->constants[noCopied].type = (int8_t)gyRenderer::Impl::D3D11TypeToSCT(typeDesc);
            srcCBuff->constants[noCopied].size = (int8_t)varDesc.Size;
            ++noCopied;
        }
        ++cbIndex;
        R_FAIL_IF(srcCBuff->constants[noCopied - 1].IsInvalid(), "Invalid constant info copied");
    }//for

     // ****************************************************************************
    R_FAIL_IF(srcCBuff->constantsCount != noCopied, "No. of constants copied != allocated ones");

    // create internal D3D11 buffers for the CBs (noInternalCBs numbers), if proceed
    for (uint32_t i = 0; i < cbsCount; ++i)
    {
        if (cbs[i].Create() != 0)
            GYDEBUGRET("Error creating DX11CBuffer\n", -1);
        arrayOfDX11CBuffers[i] = cbs[i].d3dBuffer; // just a cache of pointers for convenient dx11 api calls
    }
    return R_OK;
    */
}
