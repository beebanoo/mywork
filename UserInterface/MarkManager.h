#ifndef __INC_METIN_II_GUILDLIB_MARK_MANAGER_H__
#define __INC_METIN_II_GUILDLIB_MARK_MANAGER_H__

#include "MarkImage.h"

class CGuildMarkManager : public singleton<CGuildMarkManager>
{
	public:
		enum
		{
			MAX_IMAGE_COUNT = 5,
			INVALID_MARK_ID = 0xffffffff,
		};

		struct TGuildSymbol
		{
			DWORD crc;
			std::vector<BYTE> raw;
		};

		CGuildMarkManager();
		virtual ~CGuildMarkManager();

		const TGuildSymbol * GetGuildSymbol(DWORD GID);
		bool LoadSymbol(const char* filename);
		void SaveSymbol(const char* filename);
		void UploadSymbol(DWORD guildID, int iSize, const BYTE* pbyData);

		void SetMarkPathPrefix(const char * prefix);



		bool GetMarkImageFilename(DWORD imgIdx, std::string & path) const;
		bool AddMarkIDByGuildID(DWORD guildID, DWORD markID);
		DWORD GetMarkImageCount() const;
		DWORD GetMarkCount() const;
		DWORD GetMarkID(DWORD guildID);

		void CopyMarkIdx(char * pcBuf) const;
		DWORD SaveMark(DWORD guildID, BYTE * pbMarkImage);
		void DeleteMark(DWORD guildID);
		void GetDiffBlocks(DWORD imgIdx, const DWORD * crcList, std::map<BYTE, const SGuildMarkBlock *> & mapDiffBlocks);

		bool SaveBlockFromCompressedData(DWORD imgIdx, DWORD idBlock, const BYTE * pbBlock, DWORD dwSize);
		bool GetBlockCRCList(DWORD imgIdx, DWORD * crcList);

	private:
		CGuildMarkImage * __NewImage();
		void __DeleteImage(CGuildMarkImage * pkImgDel);

		DWORD __AllocMarkID(DWORD guildID);

		CGuildMarkImage * __GetImage(DWORD imgIdx);
		CGuildMarkImage * __GetImagePtr(DWORD idMark);


		std::set<DWORD> m_setFreeMarkID;
		std::string		m_pathPrefix;

	private:
		std::map<DWORD, TGuildSymbol> m_mapSymbol;
};

#endif
