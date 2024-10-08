#pragma once
#include "GrpBase.h"
#include "GrpTexture.h"

class CGraphicRenderTargetTexture : public CGraphicTexture
{
public:
	CGraphicRenderTargetTexture();
	virtual ~CGraphicRenderTargetTexture();

public:
	bool Create(int width, int height, D3DFORMAT texFormat, D3DFORMAT depthFormat);
	void CreateTextures();
	bool CreateRenderTexture(int width, int height, D3DFORMAT format);
	bool CreateRenderDepthStencil(int width, int height, D3DFORMAT format);

	void SetRenderTarget();
	void ResetRenderTarget();

	void SetRenderingRect(RECT* rect){ m_renderRect = *rect; }
	RECT* GetRenderingRect(){ return &m_renderRect; }

	LPDIRECT3DTEXTURE9 GetRenderTargetTexture() const { return m_lpd3dRenderTexture; }

	void ReleaseTextures();
	static void Clear();

	void Render() const;

protected:
	void Reset();
	LPDIRECT3DTEXTURE9 m_lpd3dRenderTexture{};
	LPDIRECT3DSURFACE9 m_lpd3dRenderTargetSurface{};
	LPDIRECT3DSURFACE9 m_lpd3dDepthSurface{};

	LPDIRECT3DSURFACE9 m_lpd3dOriginalRenderTarget{};
	LPDIRECT3DSURFACE9 m_lpd3dOldDepthBufferSurface{};
	D3DFORMAT m_d3dFormat;
	D3DFORMAT m_depthStencilFormat;
	RECT m_renderRect{};
};
