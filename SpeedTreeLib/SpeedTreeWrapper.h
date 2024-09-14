
#pragma once


#include "SpeedTreeMaterial.h"
#include <SpeedTreeRT.h>

#include <d3d9.h>
#include <d3d9types.h>
#include <directxsdk/d3dx9.h>
#include <vector>
#include <memory>
#include <list>

#include "../EterLib/GrpObjectInstance.h"
#include "../EterLib/GrpImageInstance.h"
#include "../EterLib/GrpCollisionObject.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p) = NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p) = NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = NULL; } }
#endif


class CSpeedTreeWrapper : public CGraphicObjectInstance
{
public:
	enum
	{
		ID = TREE_OBJECT
	};
	int GetType() const { return ID; }
	
protected:
	UINT GetCollisionObjectCount();
	void GetCollisionObject(UINT nIndex, CSpeedTreeRT::ECollisionObjectType& eType, float* pPosition, float* pDimensions);
	virtual void OnUpdateCollisionData(const CStaticCollisionDataVector * pscdVector);
	virtual void OnUpdateHeighInstance(CAttributeInstance * pAttributeInstance) {}
	virtual bool OnGetObjectHeight(float fX, float fY, float * pfHeight) { return false; }
public:
	virtual bool GetBoundingSphere(D3DXVECTOR3 & v3Center, float & fRadius);
	
public:
	static bool					ms_bSelfShadowOn;

public:
	virtual void				SetPosition(float x, float y, float z);
	virtual void				CalculateBBox();
	virtual void				OnRender();
	virtual void				OnRenderPCBlocker();
	virtual void				OnBlendRender() {}
	virtual void				OnRenderToShadowMap() {}
	virtual void				OnRenderShadow() {}

public:
	CSpeedTreeWrapper();
	virtual	~CSpeedTreeWrapper();

	bool                        LoadTree(const char* pszSptFile, const BYTE* c_pbBlock, unsigned int uiBlockSize, unsigned int nSeed = 1, float fSize = -1.0f, float fSizeVariance = -1.0f);
	const float* GetBoundingBox(void) const { return m_afBoundingBox; }

	void						SetupBranchForTreeType(void) const;
	void						SetupFrondForTreeType(void) const;
	void						SetupLeafForTreeType(void) const;
	void						EndLeafForTreeType(void);
	void						RenderBranches(void) const;
	void                        RenderFronds(void) const;
	void						RenderLeaves(void) const;
	void						RenderBillboards(void) const;
	
	CSpeedTreeWrapper** GetInstances(unsigned int& nCount);
	CSpeedTreeWrapper* InstanceOf(void) const { return m_pInstanceOf; }
	CSpeedTreeWrapper* MakeInstance(void);
	void						DeleteInstance(CSpeedTreeWrapper* pInstance);
	CSpeedTreeRT *				GetSpeedTree(void) const						{ return m_pSpeedTree; }
	
	const CSpeedTreeMaterial &	GetBranchMaterial(void) const					{ return m_cBranchMaterial; }
	const CSpeedTreeMaterial &	GetFrondMaterial(void) const					{ return m_cFrondMaterial; }
	const CSpeedTreeMaterial &	GetLeafMaterial(void) const						{ return m_cLeafMaterial; }
	float                       GetLeafLightingAdjustment(void) const			{ return m_pSpeedTree->GetLeafLightingAdjustment( ); }
	
	void						Advance(void);
	
	void						CleanUpMemory(void);
	
private:
	void						SetupBuffers(void);
	void						SetupBranchBuffers(void);
	void						SetupFrondBuffers(void);
	void						SetupLeafBuffers(void);
	void						PositionTree(void) const;
	static bool					LoadTexture(const char* pFilename, CGraphicImageInstance & rImage);
	void						SetShaderConstants(const float* pMaterial) const;
	
	
private:
	

	
	unsigned int					m_unNumFrondLods;
	LPDIRECT3DINDEXBUFFER9*			m_pFrondIndexBuffers;
	unsigned short*					m_pFrondIndexCounts;
	
	
	
	
	CGraphicImageInstance			m_BranchImageInstance;
	CGraphicImageInstance			m_CompositeImageInstance;

	static  unsigned int					m_unNumWrappersActive;

	static LPDIRECT3DVERTEXSHADER9					ms_lpBranchVertexShader;
	static LPDIRECT3DVERTEXSHADER9					ms_lpLeafVertexShader;
};

