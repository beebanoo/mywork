

#pragma once
#include "SpeedTreeConfig.h"
#include <map>
#include <string>


static DWORD D3DFVF_SPEEDTREE_BRANCH_VERTEX =
		D3DFVF_NORMAL |
	#else
		D3DFVF_DIFFUSE |
	#endif
	#ifdef WRAPPER_RENDER_SELF_SHADOWS
	#else
	#endif
	#ifdef WRAPPER_USE_GPU_WIND					
	#endif
		;


struct SFVFBranchVertex
{
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING			
#else										     
#endif	
#ifdef WRAPPER_RENDER_SELF_SHADOWS
#endif
#ifdef WRAPPER_USE_GPU_WIND		
	FLOAT			m_fWindWeight;			
#endif
};



static const char g_achSimpleVertexProgram[] = 
{


	#ifdef WRAPPER_RENDER_SELF_SHADOWS
	#endif

		"mov		a0.x,	v9.x\n"



	#ifdef WRAPPER_USE_FOG
	#endif

	#ifdef WRAPPER_USE_STATIC_LIGHTING
	#else
	
			
	#endif
};



static LPDIRECT3DVERTEXDECLARATION9 LoadBranchShader(LPDIRECT3DDEVICE9 pDx)
{
	D3DVERTEXELEMENT9 pBranchShaderDecl[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		D3DDECL_END()
	};

	LPDIRECT3DVERTEXDECLARATION9 dwShader = NULL;

	if (pDx->CreateVertexDeclaration(pBranchShaderDecl, &dwShader) != D3D_OK)
	{
		char szError[1024];
		sprintf_s(szError, "Failed to create branch vertex shader.");
		MessageBox(NULL, szError, "Vertex Shader Error", MB_ICONSTOP);
	}

	return dwShader;
}


static DWORD D3DFVF_SPEEDTREE_LEAF_VERTEX =
		D3DFVF_NORMAL |
	#else
		D3DFVF_DIFFUSE |
	#endif
	#if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT					
	#endif
		;



struct SFVFLeafVertex
{
	#ifdef WRAPPER_USE_DYNAMIC_LIGHTING			
	#else										     
	#endif											
	#if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT
		FLOAT			m_fWindWeight;					
		FLOAT			m_fLeafPlacementIndex;
		FLOAT			m_fLeafScalarValue;
	#endif
};



static const char g_achLeafVertexProgram[] = 
{

		
	#ifdef WRAPPER_USE_GPU_WIND
		"mov		a0.x,	v9.x\n"

	#else
	#endif

	#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
		"mul		r1,		c[a0.x],	v9.w\n"
		"add		r0,		r1,			r0\n"	
	#endif


	#ifdef WRAPPER_USE_FOG
		"mul		oFog,		r2.x,		c[85].z\n"
	#endif

	#ifdef WRAPPER_USE_STATIC_LIGHTING
	#else

		
	#endif
};



static LPDIRECT3DVERTEXDECLARATION9 LoadLeafShader(LPDIRECT3DDEVICE9 pDx)
{
	D3DVERTEXELEMENT9 pLeafShaderDecl[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
		D3DDECL_END()
	};

	LPDIRECT3DVERTEXDECLARATION9 dwShader = NULL;

	if (pDx->CreateVertexDeclaration(pLeafShaderDecl, &dwShader) != D3D_OK)
	{
		char szError[1024];
		sprintf_s(szError, "Failed to create leaf vertex shader.");
		MessageBox(NULL, szError, "Vertex Shader Error", MB_ICONSTOP);

	}

	return dwShader;
}
