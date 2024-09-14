#ifndef _CHddData_h___
#define _CHddData_h___

#include <string>
#include <vector>
#include <windows.h>
#include <winioctl.h>

class CHddData
{
private:
	static int const IDE_ATA_IDENTIFY = 0xEC;
	static int const IDE_ATAPI_IDENTIFY = 0xA1;
	static int const DFP_GET_VERSION = 0x00074080;
	static int const DFP_RECEIVE_DRIVE_DATA = 0x0007c088;
	static int const IOCTL_SCSI_MINIPORT_IDENTIFY = ((0x0000001b << 16) + 0x0501);
	static int const SENDIDLENGTH = sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE;

public:
	CHddData();
	~CHddData();

	std::string getHDDSerialNumber(){ return m_HDDSerialNumber; };
	std::string getHDDModelNumber(){ return m_HDDModelNumber; };

private:
	bool ReadPhysicalDriveInNTWithZeroRights();
	bool ReadPhysicalDriveInNTUsingSmart();
	bool ReadPhysicalDriveInNTWithAdminRights();
	bool ReadIdeDriveAsScsiDriveInNT();

	void SetDiskData(int drive, DWORD diskdata[256]);
	void SetDiskData(std::string model, std::string serial);

	char *flipAndCodeBytes(const char * str, int pos, int flip, char * buf);
	void ConvertToString(DWORD diskdata[256], int firstIndex, int lastIndex, std::string& buf);
	void CleanWhitespaces(std::string& buf);

	bool DoIDENTIFY(HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum, PDWORD lpcbBytesReturned);

private:
	BYTE IdOutCmd[sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
	std::string m_HDDModelNumber;
	std::string m_HDDSerialNumber;

#pragma pack(1)
	typedef struct _IDENTIFY_DATA {
		USHORT Reserved3 : 15;
		USHORT SingleWordDMAActive : 8;
		USHORT MultiWordDMAActive : 8;
		USHORT Reserved4 : 8;
	} IDENTIFY_DATA, *PIDENTIFY_DATA;
#pragma pack()

	typedef struct _SRB_IO_CONTROL
	{
		ULONG HeaderLength;
		UCHAR Signature[8 + 1];
		ULONG Timeout;
		ULONG ControlCode;
		ULONG ReturnCode;
		ULONG Length;
	} SRB_IO_CONTROL, *PSRB_IO_CONTROL;

	typedef struct _IDSECTOR
	{
		USHORT  wGenConfig;
		USHORT  wNumCyls;
		USHORT  wReserved;
		USHORT  wNumHeads;
		USHORT  wBytesPerTrack;
		USHORT  wBytesPerSector;
		USHORT  wSectorsPerTrack;
		USHORT  wVendorUnique[3];
		CHAR    sSerialNumber[20];
		USHORT  wBufferType;
		USHORT  wBufferSize;
		USHORT  wECCSize;
		CHAR    sFirmwareRev[8];
		CHAR    sModelNumber[40];
		USHORT  wMoreVendorUnique;
		USHORT  wDoubleWordIO;
		USHORT  wCapabilities;
		USHORT  wReserved1;
		USHORT  wPIOTiming;
		USHORT  wDMATiming;
		USHORT  wBS;
		USHORT  wNumCurrentCyls;
		USHORT  wNumCurrentHeads;
		USHORT  wNumCurrentSectorsPerTrack;
		ULONG   ulCurrentSectorCapacity;
		USHORT  wMultSectorStuff;
		ULONG   ulTotalAddressableSectors;
		USHORT  wSingleWordDMA;
		USHORT  wMultiWordDMA;
		BYTE    bReserved[128];
	} IDSECTOR, *PIDSECTOR;

	typedef struct _GETVERSIONOUTPARAMS
	{
	} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;
};
