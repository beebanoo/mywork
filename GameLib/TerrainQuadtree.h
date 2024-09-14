
#if !defined(AFX_TERRAINQUADTREENODE_H__C788298F_1098_4CEE_B6F3_5975D618BBF3__INCLUDED_)
#define AFX_TERRAINQUADTREENODE_H__C788298F_1098_4CEE_B6F3_5975D618BBF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once

class CTerrainQuadtreeNode  
{
public:
	CTerrainQuadtreeNode();
	virtual ~CTerrainQuadtreeNode();

public:
	long					x0, y0, x1, y1;
	CTerrainQuadtreeNode *	NW_Node;
	CTerrainQuadtreeNode *	NE_Node;
	CTerrainQuadtreeNode *	SW_Node;
	CTerrainQuadtreeNode *	SE_Node;
	long					Size;
	long					PatchNum;
	D3DXVECTOR3				center;
	float					radius;
	BYTE					m_byLODLevel;
};


