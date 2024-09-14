#ifndef __INC_METIN_II_MARKIMAGE_H__
#define __INC_METIN_II_MARKIMAGE_H__

#include <IL/il.h>

typedef unsigned long Pixel;

struct SGuildMark
{
	enum
	{
		WIDTH = 16,
		HEIGHT = 12,
		SIZE = WIDTH * HEIGHT,
	};


	void Clear();
	bool IsEmpty();
};

struct SGuildMarkBlock
{
	enum
	{
		MARK_PER_BLOCK_WIDTH = 4,
		MARK_PER_BLOCK_HEIGHT = 4,

		WIDTH = SGuildMark::WIDTH * MARK_PER_BLOCK_WIDTH,
		HEIGHT = SGuildMark::HEIGHT * MARK_PER_BLOCK_HEIGHT,

		SIZE = WIDTH * HEIGHT,
		MAX_COMP_SIZE = (SIZE * sizeof(Pixel)) + ((SIZE * sizeof(Pixel)) >> 4) + 64 + 3 
	};



	DWORD	GetCRC() const;

	void	CopyFrom(const BYTE * pbCompBuf, DWORD dwCompSize, DWORD crc);
	void	Compress(const Pixel * pxBuf);
};

class CGuildMarkImage
{
	public:
		enum
		{
			WIDTH = 512,
			HEIGHT = 512,





			INVALID_MARK_POSITION = 0xffffffff,
		};

		CGuildMarkImage();
		virtual ~CGuildMarkImage();

		void Create();
		void Destroy();

		bool Build(const char * c_szFileName);
		bool Save(const char* c_szFileName);
		bool Load(const char* c_szFileName);

		void PutData(UINT x, UINT y, UINT width, UINT height, void* data);
		void GetData(UINT x, UINT y, UINT width, UINT height, void* data);

		bool SaveMark(DWORD posMark, BYTE * pbMarkImage);
		bool DeleteMark(DWORD posMark);


		void GetBlockCRCList(DWORD * crcList);
		void GetDiffBlocks(const DWORD * crcList, std::map<BYTE, const SGuildMarkBlock *> & mapDiffBlocks);

	private:
		enum
		{
			INVALID_HANDLE = 0xffffffff,
		};

		void	BuildAllBlocks();

		SGuildMarkBlock	m_aakBlock[BLOCK_ROW_COUNT][BLOCK_COL_COUNT];
		Pixel m_apxImage[WIDTH * HEIGHT * sizeof(Pixel)];

		ILuint m_uImg;
};

#endif
