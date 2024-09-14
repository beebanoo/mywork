﻿#include "stdafx.h"
#include "StringCodec_Vietnamese.h"


static wchar_t cp1258_to_unicode[256] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 
	0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 
	0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f, 
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
	0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
	0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f, 
};

static wchar_t cp1258_composed_table[][5] = {
	{ 0x00c1, 0x00c0, 0x1ea2, 0x00c3, 0x1ea0 },
	{ 0x00e1, 0x00e0, 0x1ea3, 0x00e3, 0x1ea1 },
	{ 0x1eae, 0x1eb0, 0x1eb2, 0x1eb4, 0x1eb6 },
	{ 0x1eaf, 0x1eb1, 0x1eb3, 0x1eb5, 0x1eb7 },
	{ 0x1ea4, 0x1ea6, 0x1ea8, 0x1eaa, 0x1eac },
	{ 0x1ea5, 0x1ea7, 0x1ea9, 0x1eab, 0x1ead },
	{ 0x00c9, 0x00c8, 0x1eba, 0x1ebc, 0x1eb8 },
	{ 0x00e9, 0x00e8, 0x1ebb, 0x1ebd, 0x1eb9 },
	{ 0x1ebe, 0x1ec0, 0x1ec2, 0x1ec4, 0x1ec6 },
	{ 0x1ebf, 0x1ec1, 0x1ec3, 0x1ec5, 0x1ec7 },
	{ 0x00cd, 0x00cc, 0x1ec8, 0x0128, 0x1eca },
	{ 0x00ed, 0x00ec, 0x1ec9, 0x0129, 0x1ecb },
	{ 0x00d3, 0x00d2, 0x1ece, 0x00d5, 0x1ecc },
	{ 0x00f3, 0x00f2, 0x1ecf, 0x00f5, 0x1ecd },
	{ 0x1ed0, 0x1ed2, 0x1ed4, 0x1ed6, 0x1ed8 },
	{ 0x1ed1, 0x1ed3, 0x1ed5, 0x1ed7, 0x1ed9 },
	{ 0x1eda, 0x1edc, 0x1ede, 0x1ee0, 0x1ee2 },
	{ 0x1edb, 0x1edd, 0x1edf, 0x1ee1, 0x1ee3 },
	{ 0x00da, 0x00d9, 0x1ee6, 0x0168, 0x1ee4 },
	{ 0x00fa, 0x00f9, 0x1ee7, 0x0169, 0x1ee5 },
	{ 0x1ee8, 0x1eea, 0x1eec, 0x1eee, 0x1ef0 },
	{ 0x1ee9, 0x1eeb, 0x1eed, 0x1eef, 0x1ef1 },
	{ 0x00dd, 0x1ef2, 0x1ef6, 0x1ef8, 0x1ef4 },
	{ 0x00fd, 0x1ef3, 0x1ef7, 0x1ef9, 0x1ef5 },
};

static bool IsTone(wchar_t tone)
{
	switch(tone)
	{
	case 0x0300:
	case 0x0301:
	case 0x0309:
	case 0x0303:
	case 0x0323:
		return true;
	default:
		return false;
	}
}

static wchar_t ComposeTone(wchar_t prev, wchar_t tone)
{
	int col, row;

	switch(tone)
	{
	case 0x0301:	col = 0;	break;
	case 0x0300:	col = 1;	break;
	case 0x0309:	col = 2;	break;
	case 0x0303:	col = 3;	break;
	case 0x0323:	col = 4;	break;
	default:
		return prev;
	}

	switch(prev)
	{
	case 0x0041:	row = 0;	break;
	case 0x0061:	row = 1;	break;
	case 0x0102:	row = 2;	break;
	case 0x0103:	row = 3;	break;
	case 0x00C2:	row = 4;	break;
	case 0x00E2:	row = 5;	break;
	case 0x0045:	row = 6;	break;
	case 0x0065:	row = 7;	break;
	case 0x00CA:	row = 8;	break;
	case 0x00EA:	row = 9;	break;
	case 0x0049:	row = 10;	break;
	case 0x0069:	row = 11;	break;
	case 0x004F:	row = 12;	break;
	case 0x006F:	row = 13;	break;
	case 0x00D4:	row = 14;	break;
	case 0x00F4:	row = 15;	break;
	case 0x01A0:	row = 16;	break;
	case 0x01A1:	row = 17;	break;
	case 0x0055:	row = 18;	break;
	case 0x0075:	row = 19;	break;
	case 0x01AF:	row = 20;	break;
	case 0x01B0:	row = 21;	break;
	case 0x0059:	row = 22;	break;
	case 0x0079:	row = 23;	break;
	default:
		return prev;
	}

	return cp1258_composed_table[row][col];
}

int EL_String_Decode_Vietnamese(const char* multi, int multiLen, wchar_t* wide, int wideLen)
{
	if(multiLen < 0)
		multiLen = (int)strlen(multi) + 1;

	int src = 0;
	int dest = 0;

	if(multiLen > 0)
	{
		 
		wchar_t prev = cp1258_to_unicode[(BYTE)multi[src++]];

		while(src < multiLen)
		{
			wchar_t unicode = cp1258_to_unicode[(BYTE)multi[src]];

			 
			if(IsTone(unicode))
			{
				 
				prev = ComposeTone(prev, unicode);
			}
			else
			{
				 
				if(dest < wideLen)
					wide[dest++] = prev;
				prev = unicode;
			}
			++src;
		}

		if(dest < wideLen)
			wide[dest++] = prev;
	}

	return dest;
}

static bool DecomposeLetter(wchar_t input, char* letter)
{
	switch(input)
	{
		*letter = 'A';
		return true;
		*letter = 'a';
		return true;
		*letter = (char)0xc3;
		return true;
		*letter = (char)0xe3;
		return true;
		*letter = (char)0xc2;
		return true;
		*letter = (char)0xe2;
		return true;
		*letter = (char)'E';
		return true;
		*letter = (char)'e';
		return true;
		*letter = (char)0xca;
		return true;
		*letter = (char)0xea;
		return true;
		*letter = (char)'I';
		return true;
		*letter = (char)'i';
		return true;
		*letter = (char)'O';
		return true;
		*letter = (char)'o';
		return true;
		*letter = (char)0xd4;
		return true;
		*letter = (char)0xf4;
		return true;
		*letter = (char)0xd5;
		return true;
		*letter = (char)0xf5;
		return true;
		*letter = (char)'U';
		return true;
		*letter = (char)'u';
		return true;
		*letter = (char)0xdd;
		return true;
		*letter = (char)0xfd;
		return true;
		*letter = (char)'Y';
		return true;
		*letter = (char)'y';
		return true;
		*letter = (char)0xc3;
		return true;
		*letter = (char)0xe3;
		return true;
		*letter = (char)0xd0;
		return true;
		*letter = (char)0xf0;
		return true;
		*letter = (char)0xd5;
		return true;
		*letter = (char)0xf5;
		return true;
		*letter = (char)0xdd;
		return true;
		*letter = (char)0xfd;
		return true;
		*letter = (char)0xfe;
		return true;
		*letter = (char)'"';
		return true;
		*letter = (char)'"';
		return true;
	}

	if(input < 256)
	{
		*letter = (char)input;
		return true;
	}
    return false;
}

static bool DecomposeTone(wchar_t input, char* tone)
{
	switch(input)
	{
		*tone = (char)0xec;
		return true;
		*tone = (char)0xcc;
		return true;
		*tone = (char)0xd2;
		return true;
		*tone = (char)0xde;
		return true;
		*tone = (char)0xf2;
		return true;
	}

	return false;
}

int EL_String_Encode_Vietnamese(const wchar_t* wide, int wideLen, char* multi, int multiLen)
{
	if(wideLen < 0)
		wideLen = (int)wcslen(wide) + 1;

	int src = 0;
	int dest = 0;

	if(wideLen > 0)
	{
		while(src < wideLen && dest < multiLen)
		{
			char letter;
			if(DecomposeLetter(wide[src], &letter))
			{
				multi[dest++] = letter;
			}

			char tone;
			if(DecomposeTone(wide[src], &tone) && dest < multiLen)
			{
				multi[dest++] = tone;
			}

			++src;
		}
	}

	return dest;
}
