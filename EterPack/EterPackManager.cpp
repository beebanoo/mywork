#include "StdAfx.h"

#include <io.h>
#include <assert.h>

#include "EterPackManager.h"
#include "EterPackPolicy_CSHybridCrypt.h"

#include "../eterBase/Debug.h"
#include "../eterBase/CRC32.h"

#define PATH_ABSOLUTE_YMIRWORK1	"d:/ymir work/"
#define PATH_ABSOLUTE_YMIRWORK2	"d:\\ymir work\\"

#ifdef __THEMIDA__
#include <ThemidaSDK.h>
#endif

#ifdef ENABLE_FOX_FS
namespace FoxFS
{
	enum
	{
		ERROR_OK = 0,
		ERROR_BASE_CODE = 0,
		ERROR_FILE_WAS_NOT_FOUND = ERROR_BASE_CODE + 1,
		ERROR_CORRUPTED_FILE = ERROR_BASE_CODE + 2,
		ERROR_MISSING_KEY = ERROR_BASE_CODE + 3,
		ERROR_MISSING_IV = ERROR_BASE_CODE + 4,
		ERROR_DECRYPTION_HAS_FAILED = ERROR_BASE_CODE + 5,
		ERROR_DECOMPRESSION_FAILED = ERROR_BASE_CODE + 6,
		ERROR_ARCHIVE_NOT_FOUND = ERROR_BASE_CODE + 7,
		ERROR_ARCHIVE_NOT_READABLE = ERROR_BASE_CODE + 8,
		ERROR_ARCHIVE_INVALID = ERROR_BASE_CODE + 9,
		ERROR_ARCHIVE_ACCESS_DENIED = ERROR_BASE_CODE + 10,
		ERROR_KEYSERVER_SOCKET = ERROR_BASE_CODE + 11,
		ERROR_KEYSERVER_CONNECTION = ERROR_BASE_CODE + 12,
		ERROR_KEYSERVER_RESPONSE = ERROR_BASE_CODE + 13,
		ERROR_KEYSERVER_TIMEOUT = ERROR_BASE_CODE + 14,
		ERROR_UNKNOWN = ERROR_BASE_CODE + 15
	};
}

const char* white_file_list[] = { "logininfo.xml", "mark\10_0.tga" "mark\250_0.tga" };

bool isWhiteFile(const char* c_szFileName)
{
	for (int i = 0; i < ARRAYSIZE(white_file_list); i++)
	{
		if (stricmp(c_szFileName, white_file_list[i]) == 0)
		{
			return true;
		}
	}
	return false;
}

const char* white_file_list_ext[] = { "xml", "tga", "png", "bmp", "mp3", "jpg" };

bool isWhiteFileExt(const char* c_szFileName)
{
	for (int i = 0; i < ARRAYSIZE(white_file_list_ext); i++)
	{
		std::string fileName = c_szFileName;
		std::string ext = CFileNameHelper::GetExtension(fileName);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		std::string whiteExt = std::string(white_file_list_ext[i]);

		if (ext == whiteExt)
			return true;
	}
	return false;
}
#endif


CEterPack* CEterPackManager::FindPack(const char* c_szPathName)
{
	std::string strFileName;

	if (0 == ConvertFileName(c_szPathName, strFileName))
	{
		return &m_RootPack;
	}
	else
	{
		for (TEterPackMap::iterator itor = m_DirPackMap.begin(); itor != m_DirPackMap.end(); ++itor)
		{
			const std::string & c_rstrName = itor->first;
			CEterPack * pEterPack = itor->second;

			if (CompareName(c_rstrName.c_str(), c_rstrName.length(), strFileName.c_str()))
			{
				return pEterPack;
			}
		}
	}

	return NULL;
}

void CEterPackManager::SetCacheMode()
{
	m_isCacheMode=true;
}

void CEterPackManager::SetRelativePathMode()
{
	m_bTryRelativePath = true;
}


int CEterPackManager::ConvertFileName(const char * c_szFileName, std::string & rstrFileName)
{
	rstrFileName = c_szFileName;
	stl_lowers(rstrFileName);

	int iCount = 0;

	for (DWORD i = 0; i < rstrFileName.length(); ++i)
	{
		if (rstrFileName[i] == '/')
			++iCount;
		else if (rstrFileName[i] == '\\')
		{
			rstrFileName[i] = '/';
			++iCount;
		}
	}

	return iCount;
}

bool CEterPackManager::CompareName(const char * c_szDirectoryName, DWORD , const char * c_szFileName)
{
	const char * c_pszSrc = c_szDirectoryName;
	const char * c_pszCmp = c_szFileName;

	while (*c_pszSrc)
	{
		if (*(c_pszSrc++) != *(c_pszCmp++))
			return false;

		if (!*c_pszCmp)
			return false;
	}

	return true;
}

void CEterPackManager::LoadStaticCache(const char* c_szFileName)
{
	if (!m_isCacheMode)
		return;

	std::string strFileName;
	if (0 == ConvertFileName(c_szFileName, strFileName))
	{
		return;
	}
	
	DWORD dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());

	std::unordered_map<DWORD, SCache>::iterator f = m_kMap_dwNameKey_kCache.find(dwFileNameHash);
	if (m_kMap_dwNameKey_kCache.end() != f)
		return;	
	
	CMappedFile kMapFile;
	const void* c_pvData;
	if (!Get(kMapFile, c_szFileName, &c_pvData))
		return;

	SCache kNewCache;
	kNewCache.m_dwBufSize = kMapFile.Size();
	kNewCache.m_abBufData = new BYTE[kNewCache.m_dwBufSize];
	memcpy(kNewCache.m_abBufData, c_pvData, kNewCache.m_dwBufSize);
	m_kMap_dwNameKey_kCache.insert(std::unordered_map<DWORD, SCache>::value_type(dwFileNameHash, kNewCache));
}

CEterPackManager::SCache* CEterPackManager::__FindCache(DWORD dwFileNameHash)
{	
	std::unordered_map<DWORD, SCache>::iterator f=m_kMap_dwNameKey_kCache.find(dwFileNameHash);
	if (m_kMap_dwNameKey_kCache.end()==f)
		return NULL;

	return &f->second;
}

void	CEterPackManager::__ClearCacheMap()
{
	std::unordered_map<DWORD, SCache>::iterator i;

	for (i = m_kMap_dwNameKey_kCache.begin(); i != m_kMap_dwNameKey_kCache.end(); ++i)
		delete [] i->second.m_abBufData;

	m_kMap_dwNameKey_kCache.clear();
}

struct TimeChecker
{
	TimeChecker(const char* name) : name(name)
	{
		baseTime = timeGetTime();
	}
	~TimeChecker()
	{
		printf("load %s (%d)\n", name, timeGetTime() - baseTime);
	}

	const char* name;
	DWORD baseTime;
};

bool CEterPackManager::Get(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData)
{
#if defined(ENABLE_FOX_FS)
	if (m_iSearchMode == SEARCH_FILE)
	{
		if (GetFromFile(rMappedFile, c_szFileName, pData))
		{
			return true;
		}

		return GetFromPack(rMappedFile, c_szFileName, pData);
	}
	else
	{
		if (isExistInPack(c_szFileName))
		{
			return GetFromPack(rMappedFile, c_szFileName, pData);
		}
		else if (isExist(c_szFileName))
		{
			return GetFromFile(rMappedFile, c_szFileName, pData);
		}
	}
	return false;
#else
	if (m_iSearchMode == SEARCH_PACK_FIRST)
	{
		if (GetFromPack(rMappedFile, c_szFileName, pData))
			return true;
		
		if (c_szFileName[1] != ':' && GetFromFile(rMappedFile, c_szFileName, pData))
		{
			return true;
		}
	}
	
	if (m_iSearchMode == SEARCH_FILE_FIRST)
	{
		if (GetFromFile(rMappedFile, c_szFileName, pData))
			return true;
		
		return GetFromPack(rMappedFile, c_szFileName, pData);
	}
	
	return false;
#endif
}

struct FinderLock
{
	FinderLock(CRITICAL_SECTION& cs) : p_cs(&cs)
	{
		EnterCriticalSection(p_cs);
	}

	~FinderLock()
	{
		LeaveCriticalSection(p_cs);
	}

	CRITICAL_SECTION* p_cs;
};

bool CEterPackManager::GetFromPack(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData)
{
#ifdef ENABLE_FOX_FS
	assert(c_szFileName);

	FinderLock lock(m_csFinder);

	if (m_pFoxFS)
	{
		int errorCodeSize = 0;
		if ((errorCodeSize = FoxFS_ExistsA(m_pFoxFS, c_szFileName)) == FoxFS::ERROR_OK)
		{
			unsigned int dwSize = FoxFS_SizeA(m_pFoxFS, c_szFileName), dwReadSize = 0;
			BYTE* pbData = new BYTE[dwSize + 1];
			int errorCode = 0;
			if ((errorCode = FoxFS_GetA(m_pFoxFS, c_szFileName, pbData, dwSize, &dwReadSize)) == FoxFS::ERROR_OK)
			{
				pbData[dwReadSize] = 0;
				*pData = pbData;
				rMappedFile.Link(dwReadSize, pbData);
				return true;
			}
			else {
				TraceError("Could not get file %s Error Code %d", c_szFileName, errorCode);
			}
			delete[] pbData;
		}
		else {
			TraceError("File not existing %s Error Code %d", c_szFileName, errorCodeSize);

		}
	}
	else
	{
		TraceError("FoxFS: Not initialized!");
	}

	return false;
#else
	FinderLock lock(m_csFinder);

	static std::string strFileName;

	if (0 == ConvertFileName(c_szFileName, strFileName))
	{
		return m_RootPack.Get(rMappedFile, strFileName.c_str(), pData);
	}
	else
	{
		DWORD dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());
		SCache* pkCache = __FindCache(dwFileNameHash);

		if (pkCache)
		{
			rMappedFile.Link(pkCache->m_dwBufSize, pkCache->m_abBufData);
			return true;
		}

		CEterFileDict::Item* pkFileItem = m_FileDict.GetItem(dwFileNameHash, strFileName.c_str());

		if (pkFileItem)
			if (pkFileItem->pkPack)
			{
				bool r = pkFileItem->pkPack->Get2(rMappedFile, strFileName.c_str(), pkFileItem->pkInfo, pData);
				return r;
			}
	}
#ifdef _DEBUG
	TraceError("CANNOT_FIND_PACK_FILE [%s]", strFileName.c_str());
#endif

	return false;
#endif
}

void CEterPackManager::ArrangeMemoryMappedPack()
{
}

bool CEterPackManager::GetFromFile(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData)
{
#ifndef _DEBUG
#endif


	return rMappedFile.Create(c_szFileName, pData, 0, 0) ? true : false;
}

bool CEterPackManager::isExistInPack(const char * c_szFileName)
{
#ifdef ENABLE_FOX_FS
	assert(c_szFileName);

	if (m_pFoxFS)
	{
		int errorCodeSize = 0;
		if ((errorCodeSize = FoxFS_ExistsA(m_pFoxFS, c_szFileName)) == FoxFS::ERROR_OK) {
			return true;
		}
	}
	else
	{
		TraceError("FoxFS: Not initialized!");
	}

	return false;
#else
	std::string strFileName;

	if (0 == ConvertFileName(c_szFileName, strFileName))
	{
		return m_RootPack.IsExist(strFileName.c_str());
	}
	else
	{
		DWORD dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());
		CEterFileDict::Item* pkFileItem = m_FileDict.GetItem(dwFileNameHash, strFileName.c_str());

		if (pkFileItem)
			if (pkFileItem->pkPack)
				return pkFileItem->pkPack->IsExist(strFileName.c_str());
	}

	return false;
#endif
}

bool CEterPackManager::isExist(const char * c_szFileName)
{
#if defined(ENABLE_FOX_FS)
	if (m_iSearchMode == SEARCH_PACK)
	{
		if (isWhiteFile(c_szFileName) || isWhiteFileExt(c_szFileName))
		{
			return isExistInPack(c_szFileName) || (_access(c_szFileName, 0) == 0);
		}
		return isExistInPack(c_szFileName);
	}

	if (_access(c_szFileName, 0) == 0)
		return true;

	return isExistInPack(c_szFileName);
#else
	if (m_iSearchMode == SEARCH_PACK_FIRST)
	{
		if (isExistInPack(c_szFileName))
			return true;

		return _access(c_szFileName, 0) == 0 ? true : false;
	}


	if (_access(c_szFileName, 0) == 0)
		return true;

	return isExistInPack(c_szFileName);
#endif
}


void CEterPackManager::RegisterRootPack(const char * c_szName)
{
#ifdef ENABLE_FOX_FS
	assert(c_szName);
	if (m_pFoxFS)
	{
		int errorCode = 0;
		if ((errorCode = FoxFS_LoadA(m_pFoxFS, c_szName)) != FoxFS::ERROR_OK)
		{
			TraceError("%s: Error Code %d", c_szName, errorCode);
		}
	}
	else
	{
		TraceError("Pack: Not initialized!");
	}
#else
	if (!m_RootPack.Create(m_FileDict, c_szName, ""))
	{
		TraceError("%s: Pack file does not exist", c_szName);
	}
#endif
}

const char * CEterPackManager::GetRootPackFileName()
{
	return m_RootPack.GetDBName();
}

bool CEterPackManager::DecryptPackIV(DWORD dwPanamaKey)
{
	TEterPackMap::iterator itor = m_PackMap.begin();
	while (itor != m_PackMap.end())
	{
		itor->second->DecryptIV(dwPanamaKey);
		itor++;
	}
	return true;
}

bool CEterPackManager::RegisterPackWhenPackMaking(const char * c_szName, const char * c_szDirectory, CEterPack* pPack)
{
	m_PackMap.insert(TEterPackMap::value_type(c_szName, pPack));
	m_PackList.push_front(pPack);

	m_DirPackMap.insert(TEterPackMap::value_type(c_szDirectory, pPack));
	return true;
}


bool CEterPackManager::RegisterPack(const char * c_szName, const char * c_szDirectory, const BYTE* c_pbIV)
{
#ifdef ENABLE_FOX_FS
	assert(c_szName);
	if (m_pFoxFS)
	{
		int errorCode = 0;
		if ((errorCode = FoxFS_LoadA(m_pFoxFS, c_szName)) != FoxFS::ERROR_OK)
		{
			TraceError("%s: Error Code %d", c_szName, errorCode);
		}
	}
	else
	{
		TraceError("Pack: Not initialized!");
	}

	return false;
#else
	CEterPack * pEterPack = NULL;
	{
		TEterPackMap::iterator itor = m_PackMap.find(c_szName);

		if (m_PackMap.end() == itor)
		{
			bool bReadOnly = true;

			pEterPack = new CEterPack;
			if (pEterPack->Create(m_FileDict, c_szName, c_szDirectory, bReadOnly))
			{
				m_PackMap.insert(TEterPackMap::value_type(c_szName, pEterPack));
			}
			else
			{
#ifdef _DEBUG
				Tracef("The eterpack doesn't exist [%s]\n", c_szName);
#endif
				delete pEterPack;
				pEterPack = NULL;
				return false;
			}
		}
		else
		{
			pEterPack = itor->second;
		}
	}

	if (c_szDirectory && c_szDirectory[0] != '*')
	{
		TEterPackMap::iterator itor = m_DirPackMap.find(c_szDirectory);
		if (m_DirPackMap.end() == itor)
		{
			m_PackList.push_front(pEterPack);
			m_DirPackMap.insert(TEterPackMap::value_type(c_szDirectory, pEterPack));
		}
	}

	return true;
#endif
}

void CEterPackManager::SetSearchMode(bool bPackFirst)
{
	m_iSearchMode = bPackFirst ? SEARCH_PACK_FIRST : SEARCH_FILE_FIRST;
}

int CEterPackManager::GetSearchMode()
{
	return m_iSearchMode;
}

CEterPackManager::CEterPackManager() : m_bTryRelativePath(false), m_iSearchMode(SEARCH_FILE_FIRST), m_isCacheMode(false)
{
	InitializeCriticalSection(&m_csFinder);
#ifdef ENABLE_FOX_FS
	m_pFoxFS = FoxFS_Create();
#endif
}

CEterPackManager::~CEterPackManager()
{
	__ClearCacheMap();

	TEterPackMap::iterator i = m_PackMap.begin();
	TEterPackMap::iterator e = m_PackMap.end();
	while (i != e)
	{
		delete i->second;
		i++;
	}	
	DeleteCriticalSection(&m_csFinder);
#ifdef ENABLE_FOX_FS
	if (m_pFoxFS)
	{
		FoxFS_Destroy(m_pFoxFS);
	}
#endif
}

void CEterPackManager::RetrieveHybridCryptPackKeys(const BYTE *pStream)
{
	int iMemOffset = 0;

	int		iPackageCnt;
	DWORD	dwPackageNameHash;

	memcpy( &iPackageCnt, pStream + iMemOffset, sizeof(int) );
	iMemOffset += sizeof(iPackageCnt);

	for( int i = 0; i < iPackageCnt; ++i )
	{
		int iRecvedCryptKeySize = 0;
		memcpy( &iRecvedCryptKeySize, pStream + iMemOffset, sizeof(iRecvedCryptKeySize) );
		iMemOffset += sizeof(iRecvedCryptKeySize); 

		memcpy( &dwPackageNameHash, pStream + iMemOffset, sizeof(dwPackageNameHash) );
		iMemOffset += sizeof(dwPackageNameHash);

		TEterPackMap::const_iterator cit;
		for( cit = m_PackMap.begin(); cit != m_PackMap.end(); ++cit )
		{
			auto ssvv = std::string(cit->first);
			std::string noPathName = CFileNameHelper::NoPath(ssvv);
			if( dwPackageNameHash == stringhash().GetHash(noPathName) )
			{
				EterPackPolicy_CSHybridCrypt* pCryptPolicy = cit->second->GetPackPolicy_HybridCrypt();
				int iHavedCryptKeySize = pCryptPolicy->ReadCryptKeyInfoFromStream( pStream + iMemOffset );
				if (iRecvedCryptKeySize != iHavedCryptKeySize)
				{
					TraceError("CEterPackManager::RetrieveHybridCryptPackKeys	cryptokey length of file(%s) is not matched. received(%d) != haved(%d)", noPathName.c_str(), iRecvedCryptKeySize, iHavedCryptKeySize);
				}
				break;
			}
		}
		iMemOffset += iRecvedCryptKeySize;
	}
}

void CEterPackManager::RetrieveHybridCryptPackSDB( const BYTE* pStream )
{
	int iReadOffset = 0;
	int iSDBInfoCount = 0;

	memcpy( &iSDBInfoCount, pStream+iReadOffset, sizeof(int) );
	iReadOffset += sizeof(int);

	for( int i = 0; i < iSDBInfoCount; ++i )
	{
		DWORD dwPackgeIdentifier;
		memcpy( &dwPackgeIdentifier, pStream+iReadOffset, sizeof(DWORD) );
		iReadOffset += sizeof(DWORD);

		TEterPackMap::const_iterator cit;
		for( cit = m_PackMap.begin(); cit != m_PackMap.end(); ++cit )
		{
			auto ssvv = std::string(cit->first);
			std::string noPathName = CFileNameHelper::NoPath(ssvv);
			if( dwPackgeIdentifier == stringhash().GetHash(noPathName) )
			{
				EterPackPolicy_CSHybridCrypt* pCryptPolicy = cit->second->GetPackPolicy_HybridCrypt();
				iReadOffset += pCryptPolicy->ReadSupplementatyDataBlockFromStream( pStream+iReadOffset );
				break;
			}
		}
	}
}


void CEterPackManager::WriteHybridCryptPackInfo(const char* pFileName)
{





	CFileBase keyFile;

	if( !keyFile.Create( pFileName, CFileBase::FILEMODE_WRITE) )
	{
		return;
	}

	int iKeyPackageCount = 0;

	keyFile.SeekCur(2*sizeof(int));

	TEterPackMap::const_iterator cit;
	for( cit = m_PackMap.begin(); cit != m_PackMap.end(); ++cit )
	{
		EterPackPolicy_CSHybridCrypt* pPolicy = cit->second->GetPackPolicy_HybridCrypt();
		if( !pPolicy || !pPolicy->IsContainingCryptKey() )
			continue;

		iKeyPackageCount++;
		auto ssvv = std::string(cit->first);
		std::string noPathName = CFileNameHelper::NoPath(ssvv);

		DWORD dwPackNamehash = stringhash().GetHash(noPathName);

		CMakePackLog::GetSingleton().Writef("CEterPackManager::WriteHybridCryptPackInfo PackName : %s, Hash : %x", noPathName.c_str(), dwPackNamehash);
		keyFile.Write( &dwPackNamehash, sizeof(DWORD) );

		pPolicy->WriteCryptKeyToFile( keyFile );
	}

	int iSDBDataOffset = keyFile.GetPosition();
	int iSDBPackageCnt = 0;

	keyFile.SeekCur(sizeof(int));
	for( cit = m_PackMap.begin(); cit != m_PackMap.end(); ++cit )
	{
		EterPackPolicy_CSHybridCrypt* pPolicy = cit->second->GetPackPolicy_HybridCrypt();
		if( !pPolicy || !pPolicy->IsContainingSDBFile() )
			continue;

		iSDBPackageCnt++;

		auto ssvv1 = std::string(cit->first);
		auto noPathName = CFileNameHelper::NoPath(ssvv1);

		DWORD dwPackNamehash = stringhash().GetHash(noPathName);
		keyFile.Write( &dwPackNamehash, sizeof(DWORD) );

		int iSDBSizeWriteOffset = keyFile.GetPosition();
		keyFile.SeekCur(sizeof(int));

		pPolicy->WriteSupplementaryDataBlockToFile( keyFile );
		int iSDBSizeAfterWrite = keyFile.GetPosition();

		keyFile.Seek(iSDBSizeWriteOffset);

		int iSDBSize = iSDBSizeAfterWrite-(iSDBSizeWriteOffset+4);
		keyFile.Write( &iSDBSize, sizeof(int) );

		keyFile.Seek(iSDBSizeAfterWrite);
	}

	keyFile.Seek(0);
	keyFile.Write( &iSDBDataOffset, sizeof(int));
	keyFile.Write( &iKeyPackageCount, sizeof(int));

	keyFile.Seek(iSDBDataOffset);
	keyFile.Write( &iSDBPackageCnt, sizeof(int));

	keyFile.Close();
}
