#include "stdafx.h"

#include "CHddData.h"

#include <functional>
#include <algorithm>
#include <sstream>

CHddData::CHddData()
{
	bool done = false;

	done = ReadPhysicalDriveInNTWithAdminRights();

	if (!done)
		done = ReadIdeDriveAsScsiDriveInNT();

	if (!done)
		done = ReadPhysicalDriveInNTWithZeroRights();

	if (!done)
		done = ReadPhysicalDriveInNTUsingSmart();
}

CHddData::~CHddData()
{
}

bool CHddData::ReadPhysicalDriveInNTWithAdminRights()
{
	bool done = false;

	HANDLE hPhysicalDriveIOCTL = 0;
	char driveName[256];

	sprintf_s(driveName, sizeof(driveName), "\\\\.\\PhysicalDrive%d", drive);


	if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
		return done;

	GETVERSIONOUTPARAMS VersionParams;
	DWORD               cbBytesReturned = 0;

	memset((void*)&VersionParams, 0, sizeof(VersionParams));
		return done;

	if (VersionParams.bIDEDeviceMap <= 0)
		return done;

	BYTE             bIDCmd = 0;
	SENDCMDINPARAMS  scip;

	bIDCmd = (VersionParams.bIDEDeviceMap >> drive & 0x10) ? \
	IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;

	memset(&scip, 0, sizeof(scip));
	memset(IdOutCmd, 0, sizeof(IdOutCmd));

	if (!DoIDENTIFY(hPhysicalDriveIOCTL, &scip, (PSENDCMDOUTPARAMS)&IdOutCmd, (BYTE)bIDCmd, (BYTE)drive, &cbBytesReturned))
		return done;

	DWORD diskdata[256];
	int ijk = 0;
	USHORT *pIdSector = (USHORT *)((PSENDCMDOUTPARAMS)IdOutCmd)->bBuffer;

	for (ijk = 0; ijk < 256; ijk++)
		diskdata[ijk] = pIdSector[ijk];

	SetDiskData(drive, diskdata);

	done = true;

	CloseHandle(hPhysicalDriveIOCTL);

	return done;
}

bool CHddData::ReadPhysicalDriveInNTUsingSmart()
{
	bool done = false;

	HANDLE hPhysicalDriveIOCTL = 0;
	char driveName[256];

	sprintf_s(driveName, sizeof(driveName), "\\\\.\\PhysicalDrive%d", drive);

	hPhysicalDriveIOCTL = CreateFile(driveName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
		return done;

	GETVERSIONINPARAMS GetVersionParams;
	DWORD cbBytesReturned = 0;


	if (!DeviceIoControl(hPhysicalDriveIOCTL, SMART_GET_VERSION, NULL, 0, &GetVersionParams, sizeof(GETVERSIONINPARAMS), &cbBytesReturned, NULL))
		return done;

	ULONG CommandSize = sizeof(SENDCMDINPARAMS) + IDENTIFY_BUFFER_SIZE;
	PSENDCMDINPARAMS Command = (PSENDCMDINPARAMS)malloc(CommandSize);
	Command->irDriveRegs.bCommandReg = ID_CMD;
	DWORD BytesReturned = 0;

	if (!DeviceIoControl(hPhysicalDriveIOCTL, SMART_RCV_DRIVE_DATA, Command, sizeof(SENDCMDINPARAMS), Command, CommandSize, &BytesReturned, NULL))
		return done;

	DWORD diskdata[256];
	USHORT *pIdSector = (USHORT *)(PIDENTIFY_DATA)((PSENDCMDOUTPARAMS)Command)->bBuffer;

	for (int ijk = 0; ijk < 256; ijk++)
		diskdata[ijk] = pIdSector[ijk];

	SetDiskData(drive, diskdata);
	done = true;

	CloseHandle(hPhysicalDriveIOCTL);
	free(Command);

	return done;
}

bool CHddData::ReadPhysicalDriveInNTWithZeroRights()
{
	bool done = false;

	HANDLE hPhysicalDriveIOCTL = 0;

	char driveName[256];

	sprintf_s(driveName, sizeof(driveName), "\\\\.\\PhysicalDrive%d", drive);

	hPhysicalDriveIOCTL = CreateFile(driveName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
		return done;

	char local_buffer[10000];
	STORAGE_PROPERTY_QUERY query;
	DWORD cbBytesReturned = 0;

	memset((void *)& query, 0, sizeof(query));
	query.PropertyId = StorageDeviceProperty;
	query.QueryType = PropertyStandardQuery;

	memset(local_buffer, 0, sizeof(local_buffer));

	if (!DeviceIoControl(hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &local_buffer[0], sizeof(local_buffer), &cbBytesReturned, NULL))
		return done;

	STORAGE_DEVICE_DESCRIPTOR * descrip = (STORAGE_DEVICE_DESCRIPTOR *)& local_buffer;
	char serialNumber[1000];
	char modelNumber[1000];

	int swapNeeded = 0;


	unsigned int i = 1;
	char *c = (char*)&i;
	if (*c)
	else

	flipAndCodeBytes(local_buffer, descrip->SerialNumberOffset, swapNeeded, serialNumber);
	flipAndCodeBytes(local_buffer, descrip->ProductIdOffset, swapNeeded, modelNumber);

	if (0 == m_HDDSerialNumber[0] && (isalnum(serialNumber[0]) || isalnum(serialNumber[19])))
	{
		std::string s_SerialNumber(serialNumber);
		std::string s_ModelNumber(modelNumber);

		SetDiskData(s_ModelNumber, s_SerialNumber);
		done = true;
	}

	CloseHandle(hPhysicalDriveIOCTL);

	return done;
}

bool CHddData::DoIDENTIFY(HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum, PDWORD lpcbBytesReturned) {
	pSCIP->irDriveRegs.bFeaturesReg = 0;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = 0;
	pSCIP->irDriveRegs.bCylHighReg = 0;


	pSCIP->bDriveNumber = bDriveNum;
	pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;

	return (DeviceIoControl(hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA, (LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1, (LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1, lpcbBytesReturned, NULL) != 0);
}

bool CHddData::ReadIdeDriveAsScsiDriveInNT()
{
	bool done = false;
	int controller = 0;

	{
		HANDLE hScsiDriveIOCTL = 0;
		char   driveName[256];


		hScsiDriveIOCTL = CreateFile(driveName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (hScsiDriveIOCTL == INVALID_HANDLE_VALUE)
			continue;

		int drive = 0;

		char buffer[sizeof(SRB_IO_CONTROL) + SENDIDLENGTH];
		SRB_IO_CONTROL *p = (SRB_IO_CONTROL *)buffer;
		SENDCMDINPARAMS *pin = (SENDCMDINPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
		DWORD dummy;

		memset(buffer, 0, sizeof(buffer));
		p->HeaderLength = sizeof(SRB_IO_CONTROL);
		p->Timeout = 10000;
		p->Length = SENDIDLENGTH;
		p->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
		strncpy_s((char *)p->Signature, 8 + 1, "SCSIDISK", 8 + 1);

		pin->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
		pin->bDriveNumber = drive;

		if (!DeviceIoControl(hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT, buffer, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1, buffer, sizeof(SRB_IO_CONTROL) + SENDIDLENGTH, &dummy, NULL))
			continue;

		SENDCMDOUTPARAMS *pOut = (SENDCMDOUTPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
		IDSECTOR *pId = (IDSECTOR *)(pOut->bBuffer);
		if (!pId->sModelNumber[0])
			continue;

		DWORD diskdata[256];
		int ijk = 0;
		USHORT *pIdSector = (USHORT *)pId;

		for (ijk = 0; ijk < 256; ijk++)
			diskdata[ijk] = pIdSector[ijk];

		SetDiskData(controller * 2 + drive, diskdata);

		done = true;

		CloseHandle(hScsiDriveIOCTL);
		break;
	}

	return done;
}

void CHddData::SetDiskData(int drive, DWORD diskdata[256])
{
	std::string serialNumber;
	std::string modelNumber;


	SetDiskData(modelNumber, serialNumber);
}

void CHddData::SetDiskData(std::string model, std::string serial)
{
	if (m_HDDSerialNumber.empty() && (isalnum(serial[0]) || isalnum(serial[19])) && !serial[0] == 0)
	{
		CleanWhitespaces(model);
		CleanWhitespaces(serial);

		m_HDDModelNumber = model;
		m_HDDSerialNumber = serial;
	}
}

void CHddData::ConvertToString(DWORD diskdata[256], int firstIndex, int lastIndex, std::string& buf) {
	std::stringstream ss;
	for (int index = firstIndex; index <= lastIndex; index++)
	{
	}
	buf = ss.str();
	CleanWhitespaces(buf);
}

void CHddData::CleanWhitespaces(std::string& buf)
{
}

char * CHddData::flipAndCodeBytes(const char * str, int pos, int flip, char * buf)
{
	int i;
	int j = 0;
	int k = 0;

	buf[0] = '\0';
	if (pos <= 0)
		return buf;

	if (!j)
	{
		char p = 0;

		j = 1;
		k = 0;
		buf[k] = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			char c = tolower(str[i]);

			if (isspace(c))
				c = '0';

			++p;
			buf[k] <<= 4;

			if (c >= '0' && c <= '9')
				buf[k] |= (unsigned char)(c - '0');
			else if (c >= 'a' && c <= 'f')
				buf[k] |= (unsigned char)(c - 'a' + 10);
			else
			{
				j = 0;
				break;
			}

			if (p == 2)
			{
				if (buf[k] != '\0' && !isprint(buf[k]))
				{
					j = 0;
					break;
				}
				++k;
				p = 0;
				buf[k] = 0;
			}
		}
	}

	if (!j)
	{
		j = 1;
		k = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			char c = str[i];

			if (!isprint(c))
			{
				j = 0;
				break;
			}

			buf[k++] = c;
		}
	}

	if (!j)
	{
		k = 0;
	}

	buf[k] = '\0';

	if (flip)
		for (j = 0; j < k; j += 2)
		{
			char t = buf[j];
			buf[j] = buf[j + 1];
			buf[j + 1] = t;
		}
	}

	i = j = -1;
	for (k = 0; buf[k] != '\0'; ++k)
	{
		if (!isspace(buf[k]))
		{
			if (i < 0)
				i = k;
			j = k;
		}
	}

	if ((i >= 0) && (j >= 0))
	{
		for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
			buf[k - i] = buf[k];
		buf[k - i] = '\0';
	}

	return buf;
}
