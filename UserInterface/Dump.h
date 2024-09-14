

#ifndef Duno_Dump_H
#define Dump_Dump_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <urlmon.h>
#include <wininet.h>
#include <iostream>
#include <fstream>
#pragma comment(lib,"wininet.lib")
#pragma comment(lib, "Urlmon.lib")
#include <algorithm>
#include <time.h>
#include <conio.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <vector>
#include <rpc.h>
#include <rpcdce.h>
#include <stdio.h>
#include <iomanip>
#include <cstdlib>
#include <tlhelp32.h>
#include <process.h>
#include <shellapi.h>
#include <fstream>
#include <string>

#define MAX_DUMP_OFFSETS 170 
#define MAX_DUMP_SIZE 16     
#define MAX_PROCESS_DUMP 170  

typedef struct ANITHACK_PROCDUMP {
	unsigned int m_aOffset;
	unsigned char m_aMemDump[MAX_DUMP_SIZE];
} *PANITHACK_PROCDUMP;

extern ANITHACK_PROCDUMP g_ProcessesDumps[MAX_PROCESS_DUMP];

void SystemProcessesScan();
void ProtectionMain();
bool ScanProcessMemory(HANDLE hProcess);
#endif
