

#include "StdAfx.h"
#include "BoundaryShapeManager.h"

#ifdef USE_SPEEDGRASS

inline float VecInterpolate(float fStart, float fEnd, float fPercent)
{
    return fStart + (fEnd - fStart) * fPercent;
}

#define VectorSinD(x)       sinf((x) / 57.29578f)
#define VectorCosD(x)       cosf((x) / 57.29578f)

using namespace std;

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b)) 
#endif
#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#endif

float CSpeedGrassRT::m_fLodFarDistance = 100.0f;
float CSpeedGrassRT::m_fLodTransitionLength = 37.5f;
float CSpeedGrassRT::m_afUnitBillboard[12] = { 0.0f };
float CSpeedGrassRT::m_afWindDir[4] = { 1.0f, 0.3f, 0.0f, 0.0f };

float CSpeedGrassRT::m_afCameraOut[3] = { 0.0f, 1.0f, 0.0f };
float CSpeedGrassRT::m_afCameraUp[3] = { 0.0f, 0.0f, 1.0f };
float CSpeedGrassRT::m_afCameraRight[3] = { 1.0f, 0.0f, 0.0f };
float CSpeedGrassRT::m_afCameraPos[3] = { 0.0f, 0.0f, 0.0f };
float CSpeedGrassRT::m_fFieldOfView = D3DXToRadian(40.0f);
float CSpeedGrassRT::m_fAspectRatio = 4.0f / 3.0f;

float CSpeedGrassRT::m_afFrustumBox[6] = { 0.0f };
float CSpeedGrassRT::m_afFrustumMin[2] = { FLT_MAX, FLT_MAX };
float CSpeedGrassRT::m_afFrustumMax[2] = { -FLT_MAX, -FLT_MAX };
float CSpeedGrassRT::m_afFrustumPlanes[5][4] = { 0.0f };



CSpeedGrassRT::SBlade::SBlade( ) :
	m_fSize(1.0f),
	m_fNoise(0.0f),
	m_fThrow(0.0f),
	m_ucWhichTexture(0)
{
	m_afBottomColor[0] = m_afBottomColor[1] = m_afBottomColor[2] = 1.0f;
	m_afTopColor[0] = m_afTopColor[1] = m_afTopColor[2] = 1.0f;
}



CSpeedGrassRT::SRegion::SRegion( ) :
	m_bCulled(false),
	m_fCullingRadius(1.0f)
{
	m_afCenter[0] = m_afCenter[1] = m_afCenter[2] = 0.5f;
	m_afMin[0] = m_afMin[1] = m_afMin[2] = 0.0f;
	m_afMax[0] = m_afMax[1] = m_afMax[2] = 1.0f;
	m_VertexBuffer.Destroy();
}



CSpeedGrassRT::CSpeedGrassRT( ) :
	m_nNumRegions(0),
	m_nNumRegionCols(0),
	m_nNumRegionRows(0),
	m_pRegions(NULL),
	m_bAllRegionsCulled(false)
{
	m_afBoundingBox[0] = m_afBoundingBox[1] = m_afBoundingBox[2] = 0.0f;
	m_afBoundingBox[3] = m_afBoundingBox[4] = m_afBoundingBox[5] = 1.0f;
}



CSpeedGrassRT::~CSpeedGrassRT( )
{
}



void CSpeedGrassRT::DeleteRegions(void)
{
	delete[] m_pRegions;
	m_pRegions = NULL;
	m_nNumRegions = 0;
}



const CSpeedGrassRT::SRegion* CSpeedGrassRT::GetRegions(unsigned int& uiNumRegions)
{
	uiNumRegions = m_nNumRegions;

	return m_pRegions;
}


bool CSpeedGrassRT::ParseBsfFile(const char* pFilename, unsigned int nNumBlades, unsigned int uiRows, unsigned int uiCols, float fCollisionDistance)
{
	bool bSuccess = false;

	m_nNumRegionCols = int(uiCols);
	m_nNumRegionRows = int(uiRows);

	m_afBoundingBox[0] = m_afBoundingBox[1] = m_afBoundingBox[2] = FLT_MAX;
	m_afBoundingBox[3] = m_afBoundingBox[4] = m_afBoundingBox[5] = -FLT_MAX;

	CBoundaryShapeManager cManager;
	vector<SBlade> vSceneBlades;

	if (cManager.LoadBsfFile(pFilename))
	{
		for (unsigned int i = 0; i < nNumBlades; ++i)
		{
			SBlade sBlade;

			if (cManager.RandomPoint(sBlade.m_afPos[0], sBlade.m_afPos[1]))
			{
				sBlade.m_afPos[2] = Height(sBlade.m_afPos[0], sBlade.m_afPos[1], sBlade.m_afNormal);

				D3DXVECTOR3 v3Normal(sBlade.m_afNormal[0], sBlade.m_afNormal[1], sBlade.m_afNormal[2]);
				D3DXVec3Normalize(&v3Normal, &v3Normal);
				v3Normal.z = -v3Normal.z;
				sBlade.m_afNormal[0] = v3Normal.x;
				sBlade.m_afNormal[1] = v3Normal.y;
				sBlade.m_afNormal[2] = v3Normal.z;

				for (int nAxis = 0; nAxis < 3; ++nAxis)
				{
					m_afBoundingBox[nAxis] = min(m_afBoundingBox[nAxis], sBlade.m_afPos[nAxis]);
					m_afBoundingBox[nAxis + 3] = max(m_afBoundingBox[nAxis + 3], sBlade.m_afPos[nAxis]);
				}

				float fHeightPercent = Color(sBlade.m_afPos[0], sBlade.m_afPos[1], sBlade.m_afNormal, sBlade.m_afTopColor, sBlade.m_afBottomColor);
				sBlade.m_fSize = VecInterpolate(c_fMinBladeSize, c_fMaxBladeSize, fHeightPercent);

				sBlade.m_ucWhichTexture = GetRandom(0, c_nNumBladeMaps - 1);

				sBlade.m_fNoise = GetRandom(c_fMinBladeNoise, c_fMaxBladeNoise);
				sBlade.m_fThrow = GetRandom(c_fMinBladeThrow, c_fMaxBladeThrow);

				vSceneBlades.push_back(sBlade);
			}
		}

		bSuccess = true;
	}
	else
		fprintf(stderr, "%s\n", cManager.GetCurrentError( ).c_str( ));

	if (bSuccess)
		CreateRegions(vSceneBlades, fCollisionDistance);

	return bSuccess;
}



bool CSpeedGrassRT::CustomPlacement(unsigned int uiRows, unsigned int uiCols)
{
	m_nNumRegionCols = int(uiCols);
	m_nNumRegionRows = int(uiRows);

	m_afBoundingBox[0] = m_afBoundingBox[1] = m_afBoundingBox[2] = FLT_MAX;
	m_afBoundingBox[3] = m_afBoundingBox[4] = m_afBoundingBox[5] = -FLT_MAX;

	vector<SBlade> vSceneBlades;

	SBlade sBlade;

	sBlade.m_afPos[0] = 0.0f;
	sBlade.m_afPos[1] = 0.0f;
	sBlade.m_afPos[2] = 0.0f;

	sBlade.m_afNormal[0] = 0.0f;
	sBlade.m_afNormal[1] = 0.0f;
	sBlade.m_afNormal[2] = 1.0f;

	for (int nAxis = 0; nAxis < 3; ++nAxis)
	{
		m_afBoundingBox[nAxis] = min(m_afBoundingBox[nAxis], sBlade.m_afPos[nAxis]);
		m_afBoundingBox[nAxis + 3] = max(m_afBoundingBox[nAxis + 3], sBlade.m_afPos[nAxis]);
	}

	memcpy(sBlade.m_afBottomColor, sBlade.m_afNormal, 12);
	memcpy(sBlade.m_afTopColor, sBlade.m_afNormal, 12);

	sBlade.m_ucWhichTexture = GetRandom(0, c_nNumBladeMaps - 1);

	sBlade.m_fNoise = GetRandom(c_fMinBladeNoise, c_fMaxBladeNoise);
	sBlade.m_fThrow = GetRandom(c_fMinBladeThrow, c_fMaxBladeThrow);

	sBlade.m_fSize = GetRandom(c_fMinBladeSize, c_fMaxBladeSize);

	vSceneBlades.push_back(sBlade);

	CreateRegions(vSceneBlades);

	return true;
}


 
void CSpeedGrassRT::GetLodParams(float& fFarDistance, float& fTransitionLength)
{
	fFarDistance = m_fLodFarDistance;
	fTransitionLength = m_fLodTransitionLength;
}



void CSpeedGrassRT::SetLodParams(float fFarDistance, float fTransitionLength)
{
	m_fLodFarDistance = fFarDistance;
	m_fLodTransitionLength = fTransitionLength;
}



void CSpeedGrassRT::Cull(void)
{
	int anFrustumCellsMin[2], anFrustumCellsMax[2];
	ConvertCoordsToCell(m_afFrustumMin, anFrustumCellsMin);
	ConvertCoordsToCell(m_afFrustumMax, anFrustumCellsMax);

	for (int i = 0; i < m_nNumRegions; ++i)
		m_pRegions[i].m_bCulled = true;
	
	int nRegionsDrawn = 0;

	if ((anFrustumCellsMin[0] < 0 && anFrustumCellsMax[0] < 0) ||
		(anFrustumCellsMin[0] >= m_nNumRegionCols && anFrustumCellsMax[0] >= m_nNumRegionCols) ||
		(anFrustumCellsMin[1] < 0 && anFrustumCellsMax[1] < 0) ||
		(anFrustumCellsMin[1] >= m_nNumRegionRows && anFrustumCellsMax[1] >= m_nNumRegionRows))
		m_bAllRegionsCulled = true;
	else
	{
		anFrustumCellsMin[0] = max(anFrustumCellsMin[0], 0);
		anFrustumCellsMin[1] = max(anFrustumCellsMin[1], 0);
		anFrustumCellsMax[0] = min(anFrustumCellsMax[0], m_nNumRegionCols - 1);
		anFrustumCellsMax[1] = min(anFrustumCellsMax[1], m_nNumRegionRows - 1);

		for (i = anFrustumCellsMin[0]; i <= anFrustumCellsMax[0]; ++i)
			for (int j = anFrustumCellsMin[1]; j <= anFrustumCellsMax[1]; ++j)
			{
				SRegion* pRegion = m_pRegions + GetRegionIndex(j, i);
				pRegion->m_bCulled = OutsideFrustum(pRegion);
			}

		m_bAllRegionsCulled = false;
	}
}



void CSpeedGrassRT::SetWindDirection(const float* pWindDir)
{
	memcpy(m_afWindDir, pWindDir, 3 * sizeof(float));
	m_afWindDir[3] = 0.0f;
}



const float* CSpeedGrassRT::GetWindDirection(void)
{
	return m_afWindDir;
}



const float* CSpeedGrassRT::GetCameraPos(void)
{
	return m_afCameraPos;
}



void CSpeedGrassRT::SetCamera(const float* pPosition, const double* pModelviewMatrix)
{
	memcpy(m_afCameraPos, pPosition, 3 * sizeof(float));

	m_afCameraRight[0] = pModelviewMatrix[0];
	m_afCameraRight[1] = pModelviewMatrix[4];
	m_afCameraRight[2] = pModelviewMatrix[8];

	m_afCameraUp[0] = pModelviewMatrix[1];
	m_afCameraUp[1] = pModelviewMatrix[5];
	m_afCameraUp[2] = pModelviewMatrix[9];

	m_afCameraOut[0] = pModelviewMatrix[2];
	m_afCameraOut[1] = pModelviewMatrix[6];
	m_afCameraOut[2] = pModelviewMatrix[10];

	ComputeUnitBillboard( );

	ComputeFrustum( );
}



void CSpeedGrassRT::SetPerspective(float fAspectRatio, float fFieldOfView)
{
	m_fAspectRatio = fAspectRatio;
	m_fFieldOfView = D3DXToRadian(fAspectRatio * fFieldOfView);
}



void CSpeedGrassRT::CreateRegions(const vector<SBlade>& vSceneBlades, float fCollisionDistance)
{
	DeleteRegions( );
	m_nNumRegions = int(m_nNumRegionRows * m_nNumRegionCols);
	m_pRegions = new SRegion[m_nNumRegions];

	float fCellWidth = (m_afBoundingBox[3] - m_afBoundingBox[0]) / m_nNumRegionCols;
	float fCellHeight = (m_afBoundingBox[4] - m_afBoundingBox[1]) / m_nNumRegionRows;

	float fY = m_afBoundingBox[1];
	for (int nRow = 0; nRow < m_nNumRegionRows; ++nRow)
	{
		float fX = m_afBoundingBox[0];
		for (int nCol = 0; nCol < m_nNumRegionCols; ++nCol)
		{
			SRegion* pRegion = m_pRegions + GetRegionIndex(nRow, nCol);

			pRegion->m_afMin[0] = fX;
			pRegion->m_afMax[0] = fX + fCellWidth;
			pRegion->m_afMin[1] = fY;
			pRegion->m_afMax[1] = fY + fCellHeight;
		
			pRegion->m_afCenter[0] = 0.5f * (pRegion->m_afMin[0] + pRegion->m_afMax[0]);
			pRegion->m_afCenter[1] = 0.5f * (pRegion->m_afMin[1] + pRegion->m_afMax[1]);

			pRegion->m_fCullingRadius = 1.1f * sqrt(
				((pRegion->m_afMax[0] - pRegion->m_afCenter[0]) * (pRegion->m_afMax[0] - pRegion->m_afCenter[0])) +
				((pRegion->m_afMax[1] - pRegion->m_afCenter[1]) * (pRegion->m_afMax[1] - pRegion->m_afCenter[1]))
				);

			fX += fCellWidth;
		}

		fY += fCellHeight;
	}

	for (vector<SBlade>::const_iterator iBlade = vSceneBlades.begin( ); iBlade != vSceneBlades.end( ); ++iBlade)
	{
		float fPercentAlongX = (iBlade->m_afPos[0] - m_afBoundingBox[0]) / (m_afBoundingBox[3] - m_afBoundingBox[0]);
		float fPercentAlongY = (iBlade->m_afPos[1] - m_afBoundingBox[1]) / (m_afBoundingBox[4] - m_afBoundingBox[1]);

		unsigned int uiCol = min(fPercentAlongX * m_nNumRegionCols, m_nNumRegionCols - 1);
		unsigned int uiRow = min(fPercentAlongY * m_nNumRegionRows, m_nNumRegionRows - 1);

		m_pRegions[GetRegionIndex(uiRow, uiCol)].m_vBlades.push_back(*iBlade);
	}

	for (int i = 0; i < m_nNumRegions; ++i)
	{
		SRegion* pRegion = m_pRegions + i;

		pRegion->m_afMin[2] = FLT_MAX;
		pRegion->m_afMax[2] = -FLT_MAX;
		for (vector<SBlade>::iterator iBlade = pRegion->m_vBlades.begin( ); iBlade != pRegion->m_vBlades.end( ); ++iBlade)
		{
			pRegion->m_afMin[2] = min(pRegion->m_afMin[2], iBlade->m_afPos[2]);
			pRegion->m_afMax[2] = max(pRegion->m_afMax[2], iBlade->m_afPos[2] + iBlade->m_fSize);
		}

		pRegion->m_afCenter[0] = 0.5f * (pRegion->m_afMin[0] + pRegion->m_afMax[0]);
		pRegion->m_afCenter[1] = 0.5f * (pRegion->m_afMin[1] + pRegion->m_afMax[1]);
		pRegion->m_afCenter[2] = 0.5f * (pRegion->m_afMin[2] + pRegion->m_afMax[2]);

		pRegion->m_fCullingRadius = 1.1f * sqrt(
			((pRegion->m_afMax[0] - pRegion->m_afCenter[0]) * (pRegion->m_afMax[0] - pRegion->m_afCenter[0])) +
			((pRegion->m_afMax[1] - pRegion->m_afCenter[1]) * (pRegion->m_afMax[1] - pRegion->m_afCenter[1])) +
			((pRegion->m_afMax[2] - pRegion->m_afCenter[2]) * (pRegion->m_afMax[2] - pRegion->m_afCenter[2])) 
			);
	}

	if (fCollisionDistance > 0.0f)
	{
		fCollisionDistance *= fCollisionDistance;
		for (int nRow = 0; nRow < m_nNumRegionRows; ++nRow)
		{
			float fX = m_afBoundingBox[0];
			for (int nCol = 0; nCol < m_nNumRegionCols; ++nCol)
			{
				SRegion* pRegion = m_pRegions + GetRegionIndex(nRow, nCol);

				for (DWORD i = 0; i < pRegion->m_vBlades.size( ); ++i)
				{
					float fX = pRegion->m_vBlades[i].m_afPos[0];
					float fY = pRegion->m_vBlades[i].m_afPos[1];
					bool bCollision = false;
					for (DWORD j = 0; j < pRegion->m_vBlades.size( ) && !bCollision; ++j)
					{
						if (i != j)
						{
							float fDistance = (fX - pRegion->m_vBlades[j].m_afPos[0]) * (fX - pRegion->m_vBlades[j].m_afPos[0]) + (fY - pRegion->m_vBlades[j].m_afPos[1]) * (fY - pRegion->m_vBlades[j].m_afPos[1]);
							if (fDistance < fCollisionDistance)
								bCollision = true;
						}
					}
					
					if (bCollision)
						pRegion->m_vBlades.erase(pRegion->m_vBlades.begin( ) + i--);
				}
			}
		}
	}
}


__forceinline void CSpeedGrassRT::RotateAxisFromIdentity(D3DXMATRIX * pMat, const float & c_fAngle, const D3DXVECTOR3 & c_rv3Axis)
{
    float s = VectorSinD(c_fAngle);
    float c = VectorCosD(c_fAngle);
    float t = 1.0 - c;
	
    float x = c_rv3Axis.x;
    float y = c_rv3Axis.y;
    float z = c_rv3Axis.z;
	
    pMat->_11 = t * x * x + c;
    pMat->_12 = t * x * y + s * z;
    pMat->_13 = t * x * z - s * y;
    pMat->_21 = t * x * y - s * z;
    pMat->_22 = t * y * y + c;
    pMat->_23 = t * y * z + s * x;
    pMat->_31 = t * x * z + s * y;
    pMat->_32 = t * y * z - s * x;
    pMat->_33 = t * z * z + c;
}



void CSpeedGrassRT::ComputeFrustum(void)
{
	D3DXVECTOR3 cCameraIn(-m_afCameraOut[0], -m_afCameraOut[1], -m_afCameraOut[2]);
	D3DXVECTOR3 cCameraUp(m_afCameraUp[0], m_afCameraUp[1], m_afCameraUp[2]);
	D3DXVECTOR3 cCameraRight(m_afCameraRight[0], m_afCameraRight[1], m_afCameraRight[2]);
	D3DXVECTOR3 cCameraPos(m_afCameraPos[0], m_afCameraPos[1], m_afCameraPos[2]);
	D3DXVECTOR3 cFarPoint = cCameraPos + cCameraIn * (m_fLodFarDistance + m_fLodTransitionLength);

	m_afFrustumPlanes[0][0] = cCameraIn.x;
	m_afFrustumPlanes[0][1] = cCameraIn.y;
	m_afFrustumPlanes[0][2] = cCameraIn.z;

	D3DXMATRIX cRotate;
	D3DXMatrixIdentity(&cRotate);
	D3DXVECTOR3 cNormal;





	RotateAxisFromIdentity(&cRotate, D3DXToDegree(0.5f * m_fFieldOfView * m_fAspectRatio + c_fHalfPi), cCameraRight);
	D3DXVec3TransformCoord(&cNormal, &cCameraIn, &cRotate);
	D3DXVec3Normalize(&cNormal, &cNormal);
	m_afFrustumPlanes[1][0] = cNormal.x;
	m_afFrustumPlanes[1][1] = cNormal.y;
	m_afFrustumPlanes[1][2] = cNormal.z;

	RotateAxisFromIdentity(&cRotate, D3DXToDegree(0.5f * m_fFieldOfView + c_fHalfPi), cCameraUp);
	D3DXVec3TransformCoord(&cNormal, &cCameraIn, &cRotate);
	D3DXVec3Normalize(&cNormal, &cNormal);
	m_afFrustumPlanes[2][0] = cNormal.x;
	m_afFrustumPlanes[2][1] = cNormal.y;
	m_afFrustumPlanes[2][2] = cNormal.z;

	RotateAxisFromIdentity(&cRotate, -D3DXToDegree(0.5f * m_fFieldOfView * m_fAspectRatio + c_fHalfPi), cCameraRight);
	D3DXVec3TransformCoord(&cNormal, &cCameraIn, &cRotate);
	D3DXVec3Normalize(&cNormal, &cNormal);
	m_afFrustumPlanes[3][0] = cNormal.x;
	m_afFrustumPlanes[3][1] = cNormal.y;
	m_afFrustumPlanes[3][2] = cNormal.z;

	RotateAxisFromIdentity(&cRotate, -D3DXToDegree(0.5f * m_fFieldOfView + c_fHalfPi), cCameraUp);
	D3DXVec3TransformCoord(&cNormal, &cCameraIn, &cRotate);
	D3DXVec3Normalize(&cNormal, &cNormal);
	m_afFrustumPlanes[4][0] = cNormal.x;
	m_afFrustumPlanes[4][1] = cNormal.y;
	m_afFrustumPlanes[4][2] = cNormal.z;

	float fFrustumHeight = (m_fLodFarDistance + m_fLodTransitionLength) * tanf(0.5f * m_fFieldOfView);
	float fFrustumWidth = (m_fLodFarDistance + m_fLodTransitionLength) * tanf(0.5f * m_fFieldOfView * m_fAspectRatio);

	D3DXVECTOR3 acFrustum[5];
	acFrustum[0] = cCameraPos;
	acFrustum[1] = cFarPoint + cCameraRight * fFrustumWidth + cCameraUp * fFrustumHeight;
	acFrustum[2] = cFarPoint - cCameraRight * fFrustumWidth + cCameraUp * fFrustumHeight;
	acFrustum[3] = cFarPoint - cCameraRight * fFrustumWidth - cCameraUp * fFrustumHeight;
	acFrustum[4] = cFarPoint + cCameraRight * fFrustumWidth - cCameraUp * fFrustumHeight;

	m_afFrustumMin[0] = m_afFrustumMin[1] = FLT_MAX;
	m_afFrustumMax[0] = m_afFrustumMax[1] = -FLT_MAX;
	for (int i = 0; i < 5; ++i)
	{
		m_afFrustumMin[0] = min(m_afFrustumMin[0], acFrustum[i][0]);
		m_afFrustumMax[0] = max(m_afFrustumMax[0], acFrustum[i][0]);
		m_afFrustumMin[1] = min(m_afFrustumMin[1], acFrustum[i][1]);
		m_afFrustumMax[1] = max(m_afFrustumMax[1], acFrustum[i][1]);
	}
}



void CSpeedGrassRT::ComputeUnitBillboard(void)
{
	float fAzimuth = atan2(-m_afCameraOut[1], -m_afCameraOut[0]);


	D3DXMATRIX cTrans;
	D3DXMatrixRotationZ(&cTrans, fAzimuth);

	static D3DXVECTOR3 afCorner1(0.0f, 0.5f, 1.0f);
	static D3DXVECTOR3 afCorner2(0.0f, -0.5f, 1.0f);
	static D3DXVECTOR3 afCorner3(0.0f, -0.5f, 0.0f);
	static D3DXVECTOR3 afCorner4(0.0f, 0.5f, 0.0f);

	D3DXVECTOR3 afNewCorner1;
	D3DXVECTOR3 afNewCorner2;
	D3DXVECTOR3 afNewCorner3;
	D3DXVECTOR3 afNewCorner4;

	D3DXVec3TransformCoord(&afNewCorner1, &afCorner1, &cTrans);
	D3DXVec3TransformCoord(&afNewCorner2, &afCorner2, &cTrans);
	D3DXVec3TransformCoord(&afNewCorner3, &afCorner3, &cTrans);
	D3DXVec3TransformCoord(&afNewCorner4, &afCorner4, &cTrans);

	m_afUnitBillboard[0] = afNewCorner1.x;
	m_afUnitBillboard[1] = afNewCorner1.y;
	m_afUnitBillboard[2] = afNewCorner1.z;
	m_afUnitBillboard[3] = afNewCorner2.x;
	m_afUnitBillboard[4] = afNewCorner2.y;
	m_afUnitBillboard[5] = afNewCorner2.z;
	m_afUnitBillboard[6] = afNewCorner3.x;
	m_afUnitBillboard[7] = afNewCorner3.y;
	m_afUnitBillboard[8] = afNewCorner3.z;
	m_afUnitBillboard[9] = afNewCorner4.x;
	m_afUnitBillboard[10] = afNewCorner4.y;
	m_afUnitBillboard[11] = afNewCorner4.z;
}



void CSpeedGrassRT::ConvertCoordsToCell(const float* pCoords, int* pGridCoords) const
{
    float fPercentAlongX = (pCoords[0] - m_afBoundingBox[0]) / (m_afBoundingBox[3] - m_afBoundingBox[0]);
    float fPercentAlongY = (pCoords[1] - m_afBoundingBox[1]) / (m_afBoundingBox[4] - m_afBoundingBox[1]);

    if (fPercentAlongX < 0.0f)
		pGridCoords[0] = -1;
	else if (fPercentAlongX > 1.0f)
		pGridCoords[0] = m_nNumRegionCols;
	else
	    pGridCoords[0] = fPercentAlongX * m_nNumRegionCols;

	if (fPercentAlongY < 0.0f)
		pGridCoords[1] = -1;
	else if (fPercentAlongY > 1.0f)
		pGridCoords[1] = m_nNumRegionRows;
	else
	    pGridCoords[1] = fPercentAlongY * m_nNumRegionRows;
}



__forceinline bool CSpeedGrassRT::OutsideFrustum(CSpeedGrassRT::SRegion* pRegion)
{
	bool bOutside = false;

	for (int i = 0; i < 5 && !bOutside; ++i)
		if (m_afFrustumPlanes[i][0] * pRegion->m_afCenter[0] + 
			m_afFrustumPlanes[i][1] * pRegion->m_afCenter[1] + 
			m_afFrustumPlanes[i][2] * pRegion->m_afCenter[2] +
			m_afFrustumPlanes[i][3] > pRegion->m_fCullingRadius)
			bOutside = true;

	return bOutside;
}
