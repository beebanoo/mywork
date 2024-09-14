#include "StdAfx.h"
#include "StateManager.h"

//#define StateManager_Assert(a) if (!(a)) puts("assert"#a)
#define StateManager_Assert(a) assert(a)

struct SLightData
{
    enum
    {
        LIGHT_NUM = 8,
    };

    struct Light
    {
        DirectX::XMFLOAT4 Position;
        DirectX::XMFLOAT4 Direction;
        DirectX::XMFLOAT4 Color;
        float Intensity;
        float Range;
        float Padding[2]; // Padding to align to 16-byte boundary for constant buffers
    } m_akLights[LIGHT_NUM];
};

void CStateManager::SetLight(DWORD index, const SLightData::Light& light)
{
    assert(index < SLightData::LIGHT_NUM);
    m_kLightData.m_akLights[index] = light;

    // Actualizează constant buffer-ul pentru lumini
    UpdateLightBuffer();
}

void CStateManager::GetLight(DWORD index, SLightData::Light& light)
{
    assert(index < SLightData::LIGHT_NUM);
    light = m_kLightData.m_akLights[index];
}

void CStateManager::UpdateLightBuffer(ID3D11DeviceContext* context)
{
    // Map constant buffer and copy light data
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    context->Map(m_lightConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, &m_kLightData.m_akLights, sizeof(SLightData::Light) * SLightData::LIGHT_NUM);
    context->Unmap(m_lightConstantBuffer.Get(), 0);

    // Bind constant buffer to the shader
    context->VSSetConstantBuffers(0, 1, m_lightConstantBuffer.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_lightConstantBuffer.GetAddressOf());
}

bool CStateManager::BeginScene()
{
    m_bScene = true;

    DirectX::XMMATRIX m4Proj;
    DirectX::XMMATRIX m4View;
    DirectX::XMMATRIX m4World;
    
    GetTransform(D3DTS_WORLD, &m4World);
    GetTransform(D3DTS_PROJECTION, &m4Proj);
    GetTransform(D3DTS_VIEW, &m4View);

    SetTransform(D3DTS_WORLD, &m4World);
    SetTransform(D3DTS_PROJECTION, &m4Proj);
    SetTransform(D3DTS_VIEW, &m4View);

    // În DirectX 11 nu este nevoie să apelezi BeginScene

    return true;
}

void CStateManager::EndScene(ID3D11DeviceContext* pContext)
{
    // În DirectX 11 nu este nevoie să apelezi EndScene. 
    // În schimb, finalizezi orice operațiune de desenare și trimiți comenzi de randare.

    pContext->Flush();  // Asigură că toate comenzile de randare sunt executate

    m_bScene = false;
}

CStateManager::CStateManager(ID3D11Device* lpDevice, ID3D11DeviceContext* lpContext)
    : m_lpD3DDev(NULL), m_lpD3DContext(NULL)
{
    m_bScene = false;
    m_dwBestMinFilter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_LINEAR;
    m_dwBestMagFilter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_LINEAR;

    SetDevice(lpDevice, lpContext);
}

CStateManager::~CStateManager()
{
    if (m_lpD3DDev)
    {
        m_lpD3DDev->Release();
        m_lpD3DDev = NULL;
    }

    if (m_lpD3DContext)
    {
        m_lpD3DContext->Release();
        m_lpD3DContext = NULL;
    }
}

void CStateManager::SetDevice(ID3D11Device* lpDevice, ID3D11DeviceContext* lpContext)
{
    StateManager_Assert(lpDevice);
    StateManager_Assert(lpContext);

    lpDevice->AddRef();
    lpContext->AddRef();

    if (m_lpD3DDev)
    {
        m_lpD3DDev->Release();
        m_lpD3DDev = NULL;
    }

    if (m_lpD3DContext)
    {
        m_lpD3DContext->Release();
        m_lpD3DContext = NULL;
    }

    m_lpD3DDev = lpDevice;
    m_lpD3DContext = lpContext;

    // Nu mai folosim GetDeviceCaps în DirectX 11. Filtrarea și anisotropia sunt gestionate separat.
    m_dwBestMagFilter = D3D11_FILTER_ANISOTROPIC;
    m_dwBestMinFilter = D3D11_FILTER_ANISOTROPIC;

    // Setarea sampler-elor cu parametri anisotropici (exemplu)
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MaxAnisotropy = 4;

    for (int i = 0; i < 8; ++i)
    {
        ID3D11SamplerState* pSamplerState = NULL;
        m_lpD3DDev->CreateSamplerState(&samplerDesc, &pSamplerState);
        m_lpD3DContext->PSSetSamplers(i, 1, &pSamplerState);

        if (pSamplerState)
            pSamplerState->Release();
    }

    SetDefaultState();
}

void CStateManager::SetBestFiltering(DWORD dwStage)
{
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = (m_dwBestMinFilter == D3D11_FILTER_ANISOTROPIC) ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = (m_dwBestMinFilter == D3D11_FILTER_ANISOTROPIC) ? 4 : 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ID3D11SamplerState* pSamplerState = nullptr;
    m_lpD3DDev->CreateSamplerState(&samplerDesc, &pSamplerState);

    // Setăm sampler-ul pentru etapa corespunzătoare
    m_lpD3DContext->PSSetSamplers(dwStage, 1, &pSamplerState);

    if (pSamplerState)
        pSamplerState->Release();
}

void CStateManager::Restore(ID3D11DeviceContext* pContext)
{
    m_bForce = true;

    // Restaurăm render states (prin PSO în DirectX 11)
    for (int i = 0; i < STATEMANAGER_MAX_RENDERSTATES; ++i)
    {
        // DirectX 11 nu folosește render states individuale, acestea sunt parte din PSO
        // Aici ar trebui să refacem PSO dacă este necesar
        SetRenderState(D3D11_RENDER_STATE_TYPE(i), m_CurrentState.m_RenderStates[i]);
    }

    // Restaurăm sampler states
    for (int i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
    {
        for (int j = 0; j < STATEMANAGER_MAX_SAMPLERSTATES; ++j)
        {
            ID3D11SamplerState* pSamplerState = m_CurrentState.m_SamplerStates[i][j];
            pContext->PSSetSamplers(i, 1, &pSamplerState);  // Setăm samplerul în pixel shader
        }
    }

    // Restaurăm texturile
    for (int i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
    {
        ID3D11ShaderResourceView* pTextureSRV = m_CurrentState.m_Textures[i];
        pContext->PSSetShaderResources(i, 1, &pTextureSRV);  // Setăm texturile în pixel shader
    }

    m_bForce = false;
}


void CStateManager::SetDefaultState(ID3D11DeviceContext* pContext)
{
    // Resetăm toate state-urile
    m_CurrentState.ResetState();
    m_CopyState.ResetState();
    m_ChipState.ResetState();

    m_bScene = false;
    m_bForce = true;

    // Setăm matricea de identitate pentru transformări
    DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
    SetTransform(D3DTS_WORLD, &identity);
    SetTransform(D3DTS_VIEW, &identity);
    SetTransform(D3DTS_PROJECTION, &identity);

    // Setăm un material implicit în constant buffer
    Material defaultMat = {};
    defaultMat.Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    defaultMat.Ambient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    defaultMat.Specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    defaultMat.Emissive = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    defaultMat.Power = 0.0f;
    SetMaterial(&defaultMat);

    // Setăm blend state-uri
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    pContext->OMSetBlendState(m_blendState.Get(), nullptr, 0xFFFFFFFF);

    // Setăm rasterizer state
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    pContext->RSSetState(m_rasterState.Get());

    // Setăm depth/stencil state
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    pContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

    // Setăm sampler state-urile
    for (int i = 0; i < 8; ++i)
    {
        pContext->PSSetSamplers(i, 1, m_samplerState.GetAddressOf());
        pContext->PSSetShaderResources(i, 1, nullptr);  // Setăm texturile ca nullptr pentru a reseta
    }

    m_bForce = false;

    // Resetăm stările de salvare
#ifdef _DEBUG
    ResetStateFlags();
#endif
}


// Material
void CStateManager::SaveMaterial()
{
    m_CopyState.m_D3DMaterial = m_CurrentState.m_D3DMaterial;
}

void CStateManager::RestoreMaterial()
{
    // Restaurăm materialul din copie
    SetMaterial(&m_CopyState.m_D3DMaterial);
}

void CStateManager::SetMaterial(const D3DMATERIAL11* pMaterial)
{
    // Convertim materialul din structura DirectX 9 în noua structură personalizată
    Material material;
    material.Ambient = DirectX::XMFLOAT4(pMaterial->Ambient.r, pMaterial->Ambient.g, pMaterial->Ambient.b, pMaterial->Ambient.a);
    material.Diffuse = DirectX::XMFLOAT4(pMaterial->Diffuse.r, pMaterial->Diffuse.g, pMaterial->Diffuse.b, pMaterial->Diffuse.a);
    material.Specular = DirectX::XMFLOAT4(pMaterial->Specular.r, pMaterial->Specular.g, pMaterial->Specular.b, pMaterial->Specular.a);
    material.Emissive = DirectX::XMFLOAT4(pMaterial->Emissive.r, pMaterial->Emissive.g, pMaterial->Emissive.b, pMaterial->Emissive.a);
    material.Power = pMaterial->Power;

    // Actualizăm constant buffer-ul pentru material
    UpdateMaterialConstantBuffer(&material);

    // Actualizăm starea curentă
    m_CurrentState.m_D3DMaterial = *pMaterial;
}

void CStateManager::UpdateMaterialConstantBuffer(const Material* pMaterial)
{
    // Mapăm constant buffer-ul pentru material
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = m_lpD3DDevContext->Map(m_MaterialConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    if (SUCCEEDED(hr))
    {
        // Copiem datele materialului în buffer
        memcpy(mappedResource.pData, pMaterial, sizeof(Material));

        // Unmap pentru a finaliza scrierea în buffer
        m_lpD3DDevContext->Unmap(m_MaterialConstantBuffer.Get(), 0);
    }

    // Setăm constant buffer-ul pentru material în shader
    m_lpD3DDevContext->PSSetConstantBuffers(0, 1, m_MaterialConstantBuffer.GetAddressOf());
}

void CStateManager::GetMaterial(D3DMATERIAL11* pMaterial)
{
    // Returnăm materialul curent din starea managerului de stare
    *pMaterial = m_CurrentState.m_D3DMaterial;
}

D3D11_BLEND CStateManager::ConvertBlendValue(DWORD dwBlendValue)
{
    switch (dwBlendValue)
    {
        case D3DBLEND_ZERO:
            return D3D11_BLEND_ZERO;
        case D3DBLEND_ONE:
            return D3D11_BLEND_ONE;
        case D3DBLEND_SRCCOLOR:
            return D3D11_BLEND_SRC_COLOR;
        case D3DBLEND_INVSRCCOLOR:
            return D3D11_BLEND_INV_SRC_COLOR;
        case D3DBLEND_SRCALPHA:
            return D3D11_BLEND_SRC_ALPHA;
        case D3DBLEND_INVSRCALPHA:
            return D3D11_BLEND_INV_SRC_ALPHA;
        case D3DBLEND_DESTALPHA:
            return D3D11_BLEND_DEST_ALPHA;
        case D3DBLEND_INVDESTALPHA:
            return D3D11_BLEND_INV_DEST_ALPHA;
        case D3DBLEND_DESTCOLOR:
            return D3D11_BLEND_DEST_COLOR;
        case D3DBLEND_INVDESTCOLOR:
            return D3D11_BLEND_INV_DEST_COLOR;
        case D3DBLEND_SRCALPHASAT:
            return D3D11_BLEND_SRC_ALPHA_SAT;
        default:
            // Default to D3D11_BLEND_ONE if an unrecognized value is passed.
            return D3D11_BLEND_ONE;
    }
}

// Renderstates
DWORD CStateManager::GetRenderState(D3D11_RENDER_STATE Type)
{
    return m_CurrentState.m_RenderStates[Type];
}

void CStateManager::SaveRenderState(D3D11_RENDER_STATE Type, DWORD dwValue)
{
    // Salvăm starea curentă în copia stării
    m_CopyState.m_RenderStates[Type] = m_CurrentState.m_RenderStates[Type];
    SetRenderState(Type, dwValue);
}

void CStateManager::RestoreRenderState(D3D11_RENDER_STATE Type)
{
    // Restaurăm starea din copie
    SetRenderState(Type, m_CopyState.m_RenderStates[Type]);
}

void CStateManager::SetRenderState(D3D11_RENDER_STATE Type, DWORD Value)
{
    // Verificăm dacă starea curentă este deja setată la aceeași valoare
    if (m_CurrentState.m_RenderStates[Type] == Value)
        return;

    // Actualizăm starea folosind termeni nativi DirectX 11
    switch (Type)
    {
        case D3D11_BLEND_ENABLE:
            m_BlendDesc.RenderTarget[0].BlendEnable = (Value != FALSE);
            break;
        case D3D11_BLEND_SRC_ALPHA:
            m_BlendDesc.RenderTarget[0].SrcBlend = static_cast<D3D11_BLEND>(Value);
            break;
        case D3D11_BLEND_DEST_ALPHA:
            m_BlendDesc.RenderTarget[0].DestBlend = static_cast<D3D11_BLEND>(Value);
            break;
        case D3D11_CULL_MODE:
            m_RasterizerDesc.CullMode = static_cast<D3D11_CULL_MODE>(Value);
            break;
        case D3D11_DEPTH_WRITE_MASK:
            m_DepthStencilDesc.DepthWriteMask = static_cast<D3D11_DEPTH_WRITE_MASK>(Value);
            break;
        default:
            break;
    }

    // Marcăm necesitatea recreării Pipeline State Object (PSO)
    m_bPSORecreateNeeded = true;

    // Actualizăm starea curentă
    m_CurrentState.m_RenderStates[Type] = Value;
}

void CStateManager::GetRenderState(D3D11_RENDER_STATE Type, DWORD* pdwValue)
{
    *pdwValue = m_CurrentState.m_RenderStates[Type];
}

// Textures
void CStateManager::SaveTexture(DWORD dwStage, ID3D11ShaderResourceView* pTexture)
{
	// Salvează textura curentă
	m_CopyState.m_Textures[dwStage] = m_CurrentState.m_Textures[dwStage];
	// Setează textura nouă
	SetTexture(dwStage, pTexture);
}

void CStateManager::RestoreTexture(DWORD dwStage)
{
	// Restaurează textura salvată anterior
	SetTexture(dwStage, m_CopyState.m_Textures[dwStage]);
}

void CStateManager::SetTexture(DWORD dwStage, ID3D11ShaderResourceView* pTexture)
{
	// Dacă textura este aceeași, nu schimbăm nimic
	if (pTexture == m_CurrentState.m_Textures[dwStage])
		return;

	// Setează textura nouă în stagiul shaderului
	m_lpD3DDevContext->PSSetShaderResources(dwStage, 1, &pTexture);
	// Actualizează starea curentă
	m_CurrentState.m_Textures[dwStage] = pTexture;
}

void CStateManager::GetTexture(DWORD dwStage, ID3D11ShaderResourceView** ppTexture)
{
	// Returnează textura curentă
	*ppTexture = m_CurrentState.m_Textures[dwStage];
}


// Texture stage states
void CStateManager::SaveTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type, DWORD dwValue)
{
    // Stocăm starea texturii în copie și actualizăm PSO-ul în funcție de parametri.
    m_CopyState.m_TextureStates[dwStage][Type] = m_CurrentState.m_TextureStates[dwStage][Type];
    SetTextureStageState(dwStage, Type, dwValue);
}

void CStateManager::RestoreTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type)
{
    // Restaurăm starea texturii
    SetTextureStageState(dwStage, Type, m_CopyState.m_TextureStates[dwStage][Type]);
}

void CStateManager::SetTextureStageState(DWORD dwStage, D3D11_BLEND_DESC BlendDesc, ID3D11ShaderResourceView* pSRV)
{
    // Check if the state has changed
    if (m_CurrentState.m_TextureStates[dwStage].pSRV == pSRV && 
        m_CurrentState.m_TextureStates[dwStage].BlendDesc == BlendDesc)
        return;

    // Set the shader resource view (SRV) for the texture
    m_lpD3DDevContext->PSSetShaderResources(dwStage, 1, &pSRV);

    // Update blend state
    m_lpD3DDevContext->OMSetBlendState(m_pBlendState, NULL, 0xffffffff);

    // Update the current state
    m_CurrentState.m_TextureStates[dwStage].pSRV = pSRV;
    m_CurrentState.m_TextureStates[dwStage].BlendDesc = BlendDesc;

    // Mark PSO as needing recreation
    m_bPSORecreateNeeded = true;
}



void CStateManager::GetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pdwValue)
{
    *pdwValue = m_CurrentState.m_TextureStates[dwStage][Type];
}


// Sampler states
void CStateManager::SaveSamplerState(DWORD dwStage, const D3D11_SAMPLER_DESC& samplerDesc)
{
#ifdef _DEBUG
    if (m_bSamplerStateSavingFlag[dwStage])
    {
        Tracef(" CStateManager::SaveSamplerState - This sampler state is already saved [%d]\n", dwStage);
        StateManager_Assert(!" This sampler state is already saved!");
    }
    m_bSamplerStateSavingFlag[dwStage] = TRUE;
#endif

    m_CopyState.m_SamplerStates[dwStage] = m_CurrentState.m_SamplerStates[dwStage];

    SetSamplerState(dwStage, samplerDesc);
}

void CStateManager::RestoreSamplerState(DWORD dwStage)
{
#ifdef _DEBUG
    if (!m_bSamplerStateSavingFlag[dwStage])
    {
        Tracef(" CStateManager::RestoreSamplerState - This sampler state was not saved [%d]\n", dwStage);
        StateManager_Assert(!" This sampler state was not saved!");
    }
    m_bSamplerStateSavingFlag[dwStage] = FALSE;
#endif

    D3D11_SAMPLER_DESC& savedSamplerDesc = m_CopyState.m_SamplerStates[dwStage];
    SetSamplerState(dwStage, savedSamplerDesc);
}

void CStateManager::SetSamplerState(UINT dwStage, const D3D11_SAMPLER_DESC& samplerDesc)
{
    if (m_CurrentState.m_SamplerState[dwStage].Filter == samplerDesc.Filter &&
        m_CurrentState.m_SamplerState[dwStage].AddressU == samplerDesc.AddressU &&
        m_CurrentState.m_SamplerState[dwStage].AddressV == samplerDesc.AddressV &&
        m_CurrentState.m_SamplerState[dwStage].AddressW == samplerDesc.AddressW)
        return;

    ID3D11SamplerState* pSamplerState = nullptr;
    HRESULT hr = m_lpD3DDev->CreateSamplerState(&samplerDesc, &pSamplerState);
    if (SUCCEEDED(hr))
    {
        m_lpD3DDevContext->PSSetSamplers(dwStage, 1, &pSamplerState);
        pSamplerState->Release();
    }

    // Update current state
    m_CurrentState.m_SamplerState[dwStage] = samplerDesc;
}

void CStateManager::GetSamplerState(DWORD dwStage, D3D11_SAMPLER_DESC* pSamplerDesc)
{
    // Retrieve the current sampler state for the specified stage
    *pSamplerDesc = m_CurrentState.m_SamplerStates[dwStage];
}

void CStateManager::SetShaderResource(UINT slot, ID3D11ShaderResourceView* pShaderResource)
{
    if (m_CurrentState.m_ShaderResources[slot] == pShaderResource)
        return; // Dacă resursa curentă este deja setată, nu mai facem nimic

    // Setăm Shader Resource View (SRV) în slotul specificat
    m_lpD3DDevContext->PSSetShaderResources(slot, 1, &pShaderResource);

    // Actualizăm starea curentă
    m_CurrentState.m_ShaderResources[slot] = pShaderResource;
}
// Vertex Shader
void CStateManager::SaveVertexShader(ID3D11VertexShader* pShader)
{
    // Salvăm shader-ul actual
    m_CopyState.m_pVertexShader = m_CurrentState.m_pVertexShader;
    SetVertexShader(pShader);
}

void CStateManager::RestoreVertexShader()
{
    // Restaurăm shader-ul salvat
    SetVertexShader(m_CopyState.m_pVertexShader);
}

void CStateManager::SetVertexShader(ID3D11VertexShader* pShader)
{
    if (m_CurrentState.m_pVertexShader != pShader)
    {
        // Actualizăm starea curentă
        m_CurrentState.m_pVertexShader = pShader;

        // Setăm vertex shader-ul în contextul grafic
        m_lpD3DDevContext->VSSetShader(pShader, nullptr, 0);
    }
}

void CStateManager::GetVertexShader(ID3D11VertexShader** ppShader)
{
    *ppShader = m_CurrentState.m_pVertexShader;
}

// Vertex Declaration
void CStateManager::SaveVertexDeclaration(ID3D11InputLayout* pInputLayout)
{
    // Salvăm layout-ul de vertex actual
    m_CopyState.m_pInputLayout = m_CurrentState.m_pInputLayout;
    SetVertexDeclaration(pInputLayout);
}

void CStateManager::RestoreVertexDeclaration()
{
    // Restaurăm layout-ul de vertex salvat
    SetVertexDeclaration(m_CopyState.m_pInputLayout);
}

void CStateManager::SetVertexDeclaration(ID3D11InputLayout* pInputLayout)
{
    if (m_CurrentState.m_pInputLayout != pInputLayout)
    {
        // Actualizăm starea curentă
        m_CurrentState.m_pInputLayout = pInputLayout;

        // Setăm input layout-ul în contextul grafic
        m_lpD3DDevContext->IASetInputLayout(pInputLayout);
    }
}
void CStateManager::GetVertexDeclaration(ID3D11InputLayout** ppInputLayout)
{
    *ppInputLayout = m_CurrentState.m_pInputLayout;
}

// Pixel Shader
void CStateManager::SavePixelShader(ID3D11PixelShader* dwShader)
{
    // Salvăm pixel shader-ul curent
    m_CopyState.m_dwPixelShader = m_CurrentState.m_dwPixelShader;
    SetPixelShader(dwShader);
}

void CStateManager::RestorePixelShader()
{
    // Restaurăm pixel shader-ul salvat
    SetPixelShader(m_CopyState.m_dwPixelShader);
}

void CStateManager::SetPixelShader(ID3D11PixelShader* dwShader)
{
    if (m_CurrentState.m_dwPixelShader != dwShader)
    {
        // Setăm pixel shader-ul doar dacă este diferit de cel curent
        m_CurrentState.m_dwPixelShader = dwShader;

        // Apelul specific pentru setarea pixel shader-ului în DirectX 11
        m_lpD3DDevContext->PSSetShader(dwShader, nullptr, 0);
    }
}

void CStateManager::GetPixelShader(ID3D11PixelShader** pdwShader)
{
    // Returnăm pixel shader-ul curent
    *pdwShader = m_CurrentState.m_dwPixelShader;
}


// FVF
void CStateManager::SaveInputLayout(ID3D11InputLayout* pInputLayout)
{
    // Salvăm input layout-ul curent
    m_CopyState.m_InputLayout = m_CurrentState.m_InputLayout;
    SetInputLayout(pInputLayout);
}

void CStateManager::RestoreInputLayout()
{
    // Restaurăm input layout-ul salvat anterior
    SetInputLayout(m_CopyState.m_InputLayout);
}

void CStateManager::SetInputLayout(ID3D11InputLayout* pInputLayout)
{
    if (m_CurrentState.m_InputLayout != pInputLayout)
    {
        // Setăm input layout-ul doar dacă este diferit de cel curent
        m_lpD3DDevContext->IASetInputLayout(pInputLayout);
        m_CurrentState.m_InputLayout = pInputLayout;
    }
}

void CStateManager::GetInputLayout(ID3D11InputLayout** pInputLayout)
{
    // Returnăm input layout-ul curent
    *pInputLayout = m_CurrentState.m_InputLayout;
}


void CStateManager::RestoreTransform(UINT slot)
{
#ifdef _DEBUG
    if (!m_bTransformSavingFlag[slot])
    {
        Tracef("CStateManager::RestoreTransform - This transform was not saved [%d]\n", slot);
        StateManager_Assert(!"This transform was not saved!");
    }
    m_bTransformSavingFlag[slot] = FALSE;
#endif

    // Restaurează matricea din buffer-ul salvat
    SetTransform(slot, &m_CopyState.m_Matrices[slot]);
}

void CStateManager::SetTransform(UINT slot, const DirectX::XMMATRIX* pMatrix)
{
    // Copiază matricea în constant buffer-ul corespunzător
    memcpy(m_TransformBufferPtr[slot], pMatrix, sizeof(DirectX::XMMATRIX));

    // Trimite constant buffer-ul la shader
    m_lpD3DDevContext->VSSetConstantBuffers(slot, 1, m_TransformBuffer.GetAddressOf());

    // Actualizează matricea curentă în managerul de stare
    m_CurrentState.m_Matrices[slot] = *pMatrix;
}

void CStateManager::SaveTransform(UINT slot, const DirectX::XMMATRIX* pMatrix)
{
#ifdef _DEBUG
    if (m_bTransformSavingFlag[slot])
    {
        Tracef("CStateManager::SaveTransform - This transform is already saved [%d]\n", slot);
        StateManager_Assert(!"This transform is already saved!");
    }
    m_bTransformSavingFlag[slot] = TRUE;
#endif

    // Salvează matricea curentă
    m_CopyState.m_Matrices[slot] = m_CurrentState.m_Matrices[slot];

    // Setează noua matrice
    SetTransform(slot, pMatrix);
}

void CStateManager::GetTransform(UINT slot, DirectX::XMMATRIX* pMatrix)
{
    *pMatrix = m_CurrentState.m_Matrices[slot];
}


// SetVertexShaderConstant
void CStateManager::SaveVertexShaderConstant(UINT dwRegister, const void* pConstantData, UINT dwConstantCount)
{
    // Salvează constantele curente în copie
    for (UINT i = 0; i < dwConstantCount; i++)
    {
        StateManager_Assert((dwRegister + i) < STATEMANAGER_MAX_VCONSTANTS);
        m_CopyState.m_VertexShaderConstants[dwRegister + i] = m_CurrentState.m_VertexShaderConstants[dwRegister + i];
    }

    // Actualizează constantele din shader
    SetVertexShaderConstant(dwRegister, pConstantData, dwConstantCount);
}

void CStateManager::SetVertexShaderConstant(UINT dwRegister, const void* pConstantData, UINT dwConstantCount)
{
    // Mapăm constant buffer-ul pentru vertex shader și copiem datele
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_lpD3DDevContext->Map(m_VertexShaderConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    // Copiază datele constante în buffer
    memcpy((DirectX::XMVECTOR*)mappedResource.pData + dwRegister, pConstantData, dwConstantCount * sizeof(DirectX::XMVECTOR));

    // Finalizează maparea buffer-ului
    m_lpD3DDevContext->Unmap(m_VertexShaderConstantBuffer.Get(), 0);

    // Trimite buffer-ul constant la shader
    m_lpD3DDevContext->VSSetConstantBuffers(0, 1, m_VertexShaderConstantBuffer.GetAddressOf());

    // Salvează valorile pentru restaurare ulterioară
    for (UINT i = 0; i < dwConstantCount; i++)
    {
        StateManager_Assert((dwRegister + i) < STATEMANAGER_MAX_VCONSTANTS);
        m_CurrentState.m_VertexShaderConstants[dwRegister + i] = *(((DirectX::XMVECTOR*)pConstantData) + i);
    }
}

void CStateManager::RestoreVertexShaderConstant(UINT dwRegister, UINT dwConstantCount)
{
    // Restaurează constantele salvate
    SetVertexShaderConstant(dwRegister, &m_CopyState.m_VertexShaderConstants[dwRegister], dwConstantCount);
}

// SetPixelShaderConstant
void CStateManager::SavePixelShaderConstant(DWORD dwRegister, const void* pConstantData, DWORD dwConstantCount)
{
    for (DWORD i = 0; i < dwConstantCount; i++)
    {
        StateManager_Assert((dwRegister + i) < STATEMANAGER_MAX_PCONSTANTS);
        m_CopyState.m_PixelShaderConstants[dwRegister + i] = *(((DirectX::XMVECTOR*)pConstantData) + i);
    }

    SetPixelShaderConstant(dwRegister, pConstantData, dwConstantCount);
}


void CStateManager::SetPixelShaderConstant(DWORD dwRegister, const void* pConstantData, DWORD dwConstantCount)
{
    // Mapăm constant buffer-ul pentru pixel shader și copiem datele
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_lpD3DDevContext->Map(m_PixelShaderConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    // Copiază datele constante în buffer
    memcpy((DirectX::XMVECTOR*)mappedResource.pData + dwRegister, pConstantData, dwConstantCount * sizeof(DirectX::XMVECTOR));

    // Finalizează maparea buffer-ului
    m_lpD3DDevContext->Unmap(m_PixelShaderConstantBuffer.Get(), 0);

    // Trimite buffer-ul constant la shader
    m_lpD3DDevContext->PSSetConstantBuffers(0, 1, m_PixelShaderConstantBuffer.GetAddressOf());

    // Salvează valorile pentru restaurare ulterioară
    for (DWORD i = 0; i < dwConstantCount; i++)
    {
        StateManager_Assert((dwRegister + i) < STATEMANAGER_MAX_PCONSTANTS);
        m_CurrentState.m_PixelShaderConstants[dwRegister + i] = *(((DirectX::XMVECTOR*)pConstantData) + i);
    }
}


void CStateManager::RestorePixelShaderConstant(UINT dwRegister, UINT dwConstantCount)
{
    SetPixelShaderConstant(dwRegister, &m_CopyState.m_PixelShaderConstants[dwRegister], dwConstantCount);
}


void CStateManager::SaveStreamSource(UINT StreamNumber, ID3D11Buffer* pStreamData, UINT Stride)
{
    // Salvează stream-ul curent în copia stării
    m_CopyState.m_StreamData[StreamNumber] = m_CurrentState.m_StreamData[StreamNumber];

    // Setează noul stream
    SetStreamSource(StreamNumber, pStreamData, Stride);
}


void CStateManager::RestoreStreamSource(UINT StreamNumber)
{
    SetStreamSource(StreamNumber,
                    m_CopyState.m_StreamData[StreamNumber].m_lpStreamData,
                    m_CopyState.m_StreamData[StreamNumber].m_Stride);
}

void CStateManager::SetStreamSource(UINT StreamNumber, ID3D11Buffer* pStreamData, UINT Stride)
{
    CStreamData kStreamData(pStreamData, Stride);
    if (m_CurrentState.m_StreamData[StreamNumber] == kStreamData)
        return;

    UINT offset = 0;

    // În DirectX 11, folosim funcția IASetVertexBuffers pentru a seta vertex buffer-ul
    m_lpD3DDevContext->IASetVertexBuffers(StreamNumber, 1, &pStreamData, &Stride, &offset);

    m_CurrentState.m_StreamData[StreamNumber] = kStreamData;
}


void CStateManager::SaveIndices(ID3D11Buffer* pIndexData, UINT BaseVertexIndex)
{
    m_CopyState.m_IndexData = m_CurrentState.m_IndexData;
    SetIndices(pIndexData, BaseVertexIndex);
}

void CStateManager::RestoreIndices()
{
    SetIndices(m_CopyState.m_IndexData.m_lpIndexData, m_CopyState.m_IndexData.m_BaseVertexIndex);
}

void CStateManager::SetIndices(ID3D11Buffer* pIndexData, UINT BaseVertexIndex)
{
    CIndexData kIndexData(pIndexData, BaseVertexIndex);

    if (m_CurrentState.m_IndexData == kIndexData)
        return;

    // În DirectX 11, folosim IASetIndexBuffer pentru a seta index buffer-ul
    m_lpD3DDevContext->IASetIndexBuffer(pIndexData, DXGI_FORMAT_R32_UINT, 0);

    m_CurrentState.m_IndexData = kIndexData;
}


HRESULT CStateManager::DrawPrimitive(D3D_PRIMITIVE_TOPOLOGY PrimitiveType, UINT StartVertex, UINT VertexCount)
{
    // Setăm topologia (tipul de primitiv)
    m_lpD3DDevContext->IASetPrimitiveTopology(PrimitiveType);

    // Desenăm primitivele
    m_lpD3DDevContext->Draw(VertexCount, StartVertex);

    return S_OK; // DirectX 11 nu returnează HRESULT la aceste funcții
}


HRESULT CStateManager::DrawPrimitiveUP(D3D_PRIMITIVE_TOPOLOGY PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    // Setăm topologia (tipul de primitiv)
    m_lpD3DDevContext->IASetPrimitiveTopology(PrimitiveType);

    // Setăm buffer-ul de date
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pVertexStreamZeroData;
    
    // Creăm un buffer temporar pentru datele de vârf
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(BufferDesc));
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth = VertexStreamZeroStride * PrimitiveCount;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer* pVertexBuffer;
    m_lpD3DDev->CreateBuffer(&BufferDesc, &InitData, &pVertexBuffer);

    // Setăm buffer-ul ca flux de date
    UINT Stride = VertexStreamZeroStride;
    UINT Offset = 0;
    m_lpD3DDevContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &Offset);

    // Desenăm primitivele
    m_lpD3DDevContext->Draw(PrimitiveCount * 3, 0);

    // Eliberăm buffer-ul temporar
    pVertexBuffer->Release();

    return S_OK;
}


HRESULT CStateManager::DrawIndexedPrimitive(D3D_PRIMITIVE_TOPOLOGY PrimitiveType, UINT StartIndex, UINT IndexCount, INT BaseVertexIndex)
{
    // Setăm topologia (tipul de primitiv)
    m_lpD3DDevContext->IASetPrimitiveTopology(PrimitiveType);

    // Desenăm primitivele indexate
    m_lpD3DDevContext->DrawIndexed(IndexCount, StartIndex, BaseVertexIndex);

    return S_OK;
}


HRESULT CStateManager::DrawIndexedPrimitiveUP(
    D3D11_PRIMITIVE_TOPOLOGY PrimitiveType,
    UINT MinVertexIndex,
    UINT NumVertexIndices,
    UINT PrimitiveCount,
    CONST void* pIndexData,
    DXGI_FORMAT IndexDataFormat,
    CONST void* pVertexStreamZeroData,
    UINT VertexStreamZeroStride)
{
    // Resetează stările curente pentru index și stream
    m_CurrentState.m_IndexData = nullptr;
    m_CurrentState.m_StreamData[0] = nullptr;

    // Mapăm buffer-ul de vertex și index într-un buffer temporar
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    
    // Asumăm că buffer-ul pentru vertex și index a fost creat
    m_lpD3DDevContext->Map(m_VertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, pVertexStreamZeroData, NumVertexIndices * VertexStreamZeroStride);
    m_lpD3DDevContext->Unmap(m_VertexBuffer.Get(), 0);

    m_lpD3DDevContext->Map(m_IndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, pIndexData, NumVertexIndices * sizeof(UINT16));  // Presupunem un index format de 16 biți
    m_lpD3DDevContext->Unmap(m_IndexBuffer.Get(), 0);

    // Setăm buffer-ul de vertex și index
    UINT stride = VertexStreamZeroStride;
    UINT offset = 0;
    m_lpD3DDevContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
    m_lpD3DDevContext->IASetIndexBuffer(m_IndexBuffer.Get(), IndexDataFormat, 0);
    m_lpD3DDevContext->IASetPrimitiveTopology(PrimitiveType);

    // Desenăm primitivele
    m_lpD3DDevContext->DrawIndexed(PrimitiveCount * 3, 0, 0);

    return S_OK;
}

