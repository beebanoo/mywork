#include "StdAfx.h"
#include "PythonApplication.h"
#include "ProcessScanner.h"
#include "PythonExceptionSender.h"
#include "resource.h"
#include "Version.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include "../eterPack/EterPackManager.h"
#include "../eterLib/Util.h"
#ifdef CEF_BROWSER
#include "CefWebBrowser.h"
#else
#include "../CWebBrowser/CWebBrowser.h"
#endif
#include "../eterBase/CPostIt.h"

#include "CheckLatestFiles.h"

#include "Hackshield.h"
#include "NProtectGameGuard.h"
#include "WiseLogicXTrap.h"
#ifdef ENABLE_CPP_PSM
#include "PythonPlayerSettingsModule.h"
#endif
#ifdef ENABLE_HWID
#include "CHwidManager.h"
#endif

extern "C" {  
extern int _fltused;  
volatile int _AVOID_FLOATING_POINT_LIBRARY_BUG = _fltused;  
};  
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
#pragma comment(linker, "/NODEFAULTLIB:libci.lib")

#pragma comment( lib, "version.lib" )
#pragma comment( lib, "python27.lib" )
#pragma comment( lib, "imagehlp.lib" )
#pragma comment( lib, "devil.lib" )
#pragma comment( lib, "granny2.lib" )
#pragma comment( lib, "mss32.lib" )
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "imm32.lib" )
#pragma comment( lib, "oldnames.lib" )
#pragma comment( lib, "SpeedTreeRT.lib" )
#pragma comment( lib, "dinput8.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "strmiids.lib" )
#pragma comment( lib, "ddraw.lib" )
#pragma comment( lib, "dmoguids.lib" )
#ifdef ENABLE_FOX_FS
#include <iphlpapi.h>

#pragma comment( lib, "iphlpapi.lib" )
#pragma comment( lib, "lz4.lib" )
#pragma comment( lib, "xxhash.lib" )
#pragma comment( lib, "FoxFS.lib" )
#endif
#include <stdlib.h>
#include <cryptopp/cryptoppLibLink.h>
bool __IS_TEST_SERVER_MODE__=false;

extern bool SetDefaultCodePage(DWORD codePage);

#ifdef USE_OPENID
extern int openid_test;
#endif

static const char * sc_apszPythonLibraryFilenames[] =
{
	"UserDict.pyc",
	"__future__.pyc",
	"copy_reg.pyc",
	"linecache.pyc",
	"ntpath.pyc",
	"os.pyc",
	"site.pyc",
	"stat.pyc",
	"string.pyc",
	"traceback.pyc",
	"types.pyc",
	"\n",
};

char gs_szErrorString[512] = "";

void ApplicationSetErrorString(const char* szErrorString)
{
	strcpy(gs_szErrorString, szErrorString);
}

bool CheckPythonLibraryFilenames()
{
	for (int i = 0; *sc_apszPythonLibraryFilenames[i] != '\n'; ++i)
	{
		std::string stFilename = "lib\\";
		stFilename += sc_apszPythonLibraryFilenames[i];

		if (_access(stFilename.c_str(), 0) != 0)
		{
			return false;
		}

		MoveFile(stFilename.c_str(), stFilename.c_str());
	}

	return true;
}

struct ApplicationStringTable 
{
	HINSTANCE m_hInstance;
	std::map<DWORD, std::string> m_kMap_dwID_stLocale;
} gs_kAppStrTable;

void ApplicationStringTable_Initialize(HINSTANCE hInstance)
{
	gs_kAppStrTable.m_hInstance=hInstance;
}

const std::string& ApplicationStringTable_GetString(DWORD dwID, LPCSTR szKey)
{
	char szBuffer[512];
	char szIniFileName[256];
	char szLocale[256];

	::GetCurrentDirectory(sizeof(szIniFileName), szIniFileName);
	if(szIniFileName[lstrlen(szIniFileName)-1] != '\\')
		strcat(szIniFileName, "\\");
	strcat(szIniFileName, "metin2client.dat");

	strcpy(szLocale, LocaleService_GetLocalePath());
	if(strnicmp(szLocale, "locale/", strlen("locale/")) == 0)
		strcpy(szLocale, LocaleService_GetLocalePath() + strlen("locale/"));
	::GetPrivateProfileString(szLocale, szKey, NULL, szBuffer, sizeof(szBuffer)-1, szIniFileName);
	if(szBuffer[0] == '\0')
		LoadString(gs_kAppStrTable.m_hInstance, dwID, szBuffer, sizeof(szBuffer)-1);
	if(szBuffer[0] == '\0')
		::GetPrivateProfileString("en", szKey, NULL, szBuffer, sizeof(szBuffer)-1, szIniFileName);
	if(szBuffer[0] == '\0')
		strcpy(szBuffer, szKey);

	std::string& rstLocale=gs_kAppStrTable.m_kMap_dwID_stLocale[dwID];
	rstLocale=szBuffer;

	return rstLocale;
}

const std::string& ApplicationStringTable_GetString(DWORD dwID)
{
	char szBuffer[512];

	LoadString(gs_kAppStrTable.m_hInstance, dwID, szBuffer, sizeof(szBuffer)-1);
	std::string& rstLocale=gs_kAppStrTable.m_kMap_dwID_stLocale[dwID];
	rstLocale=szBuffer;

	return rstLocale;
}

const char* ApplicationStringTable_GetStringz(DWORD dwID, LPCSTR szKey)
{
	return ApplicationStringTable_GetString(dwID, szKey).c_str();
}

const char* ApplicationStringTable_GetStringz(DWORD dwID)
{
	return ApplicationStringTable_GetString(dwID).c_str();
}



bool PackInitialize(const char * c_pszFolder)
{
	NANOBEGIN

	struct stat st;
	if (stat( "D:\\ymir work", &st) == 0 ) {
		MessageBox(NULL, "Please remove the folder D:\Ymir Work!", "Timeless2", MB_ICONSTOP);
		return false;
	}

	if (_access(c_pszFolder, 0) != 0)
		return false;

	std::string stFolder(c_pszFolder);
	stFolder += "/";

	CTextFileLoader::SetCacheMode();

#ifdef ENABLE_FOX_FS
	CEterPackManager::Instance().SetCacheMode();
	CEterPackManager::Instance().SetSearchMode(CEterPackManager::SEARCH_PACK);

	CSoundData::SetPackMode();
	CEterPackManager::Instance().RegisterPack("pack/bgm.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/effect.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/etc.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/guild.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/icon.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/item.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/locale.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/maps.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/monster.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/monster2.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/npc.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/npc2.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/patch1.timeless", "d:/ymir work");
	CEterPackManager::Instance().RegisterPack("pack/patch2.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/patch3.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/stuffs.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/pc1_a.timeless", "d:/ymir work/pc/assassin");
	CEterPackManager::Instance().RegisterPack("pack/pc1_s.timeless", "d:/ymir work/pc/sura");
	CEterPackManager::Instance().RegisterPack("pack/pc1_sh.timeless", "d:/ymir work/pc/shaman");
	CEterPackManager::Instance().RegisterPack("pack/pc1_w.timeless", "d:/ymir work/pc/warrior");
	CEterPackManager::Instance().RegisterPack("pack/pc2_a.timeless", "d:/ymir work/pc2/assassin");
	CEterPackManager::Instance().RegisterPack("pack/pc2_s.timeless", "d:/ymir work/pc2/sura");
	CEterPackManager::Instance().RegisterPack("pack/pc2_sh.timeless", "d:/ymir work/pc2/shaman");
	CEterPackManager::Instance().RegisterPack("pack/pc2_w.timeless", "d:/ymir work/pc2/warrior");
	CEterPackManager::Instance().RegisterPack("pack/stfs_a_f.timeless", "d:/ymir work/pc/assassin");
	CEterPackManager::Instance().RegisterPack("pack/stfs_a_m.timeless", "d:/ymir work/pc2/assassin");
	CEterPackManager::Instance().RegisterPack("pack/stfs_s_f.timeless", "d:/ymir work/pc2/sura");
	CEterPackManager::Instance().RegisterPack("pack/stfs_s_m.timeless", "d:/ymir work/pc/sura");
	CEterPackManager::Instance().RegisterPack("pack/stfs_sh_f.timeless", "d:/ymir work/pc/shaman");
	CEterPackManager::Instance().RegisterPack("pack/stfs_sh_m.timeless", "d:/ymir work/pc2/shaman");
	CEterPackManager::Instance().RegisterPack("pack/stfs_w_f.timeless", "d:/ymir work/pc2/warrior");
	CEterPackManager::Instance().RegisterPack("pack/stfs_w_m.timeless", "d:/ymir work/pc/warrior");
	CEterPackManager::Instance().RegisterPack("pack/property.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/seasons.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/sound.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/terrain.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/textureset.timeless", "");
	CEterPackManager::Instance().RegisterPack("pack/tree1.timeless", "d:/ymir work/tree");
	CEterPackManager::Instance().RegisterPack("pack/tree2.timeless", "d:/ymir work/tree");
	CEterPackManager::Instance().RegisterPack("pack/tree3.timeless", "d:/ymir work/tree");
	CEterPackManager::Instance().RegisterPack("pack/tree4.timeless", "d:/ymir work/tree");
	CEterPackManager::Instance().RegisterPack("pack/tree5.timeless", "d:/ymir work/tree");
	CEterPackManager::Instance().RegisterPack("pack/tree6.timeless", "d:/ymir work/tree");
	CEterPackManager::Instance().RegisterPack("pack/tree7.timeless", "d:/ymir work/tree");
	CEterPackManager::Instance().RegisterPack("pack/tree8.timeless", "d:/ymir work/tree");
	CEterPackManager::Instance().RegisterPack("pack/tree9.timeless", "d:/ymir work/tree");
	CEterPackManager::Instance().RegisterPack("pack/tree10.timeless", "d:/ymir work/tree");
	CEterPackManager::Instance().RegisterPack("pack/tree11.timeless", "d:/ymir work/tree");
	CEterPackManager::Instance().RegisterPack("pack/zone_a.timeless", "d:/ymir work/zone");
	CEterPackManager::Instance().RegisterPack("pack/zone_b.timeless", "d:/ymir work/zone");
	CEterPackManager::Instance().RegisterPack("pack/zone_c.timeless", "d:/ymir work/zone");
	CEterPackManager::Instance().RegisterPack("pack/zone1.timeless", "d:/ymir work/zone");
	CEterPackManager::Instance().RegisterPack("pack/zone2.timeless", "d:/ymir work/zone");
	CEterPackManager::Instance().RegisterPack("pack/zone3.timeless", "d:/ymir work/zone");
	CEterPackManager::Instance().RegisterPack("pack/zone4.timeless", "d:/ymir work/zone");
	CEterPackManager::Instance().RegisterPack("pack/zone5.timeless", "d:/ymir work/zone");
	CEterPackManager::Instance().RegisterPack("pack/zone6.timeless", "d:/ymir work/zone");
	CEterPackManager::Instance().RegisterPack("pack/zone7.timeless", "d:/ymir work/zone");
	CEterPackManager::Instance().RegisterRootPack((stFolder + std::string("root.timeless")).c_str());
#else

	CMemoryTextFileLoader TextLoader;
	TextLoader.Bind(file.Size(), pvData);

	bool bPackFirst = TRUE;

	const std::string& strPackType = TextLoader.GetLineString(0);

	if (strPackType.compare("FILE") && strPackType.compare("PACK"))
	{
		TraceError("Pack/Index has invalid syntax. First line must be 'PACK' or 'FILE'");
		return false;
	}

	Tracef("Note: PackFirst mode enabled. [pack]\n");

#else
	bPackFirst = FALSE;
	Tracef("Note: PackFirst mode not enabled. [file]\n");
#endif

#if defined(USE_RELATIVE_PATH)
	CEterPackManager::Instance().SetRelativePathMode();
#endif
	CEterPackManager::Instance().SetCacheMode();
	CEterPackManager::Instance().SetSearchMode(bPackFirst);


	std::string strPackName, strTexCachePackName;
	for (DWORD i = 1; i < TextLoader.GetLineCount() - 1; i += 2)
	{
		const std::string & c_rstFolder = TextLoader.GetLineString(i);
		const std::string & c_rstName = TextLoader.GetLineString(i + 1);

		strPackName = stFolder + c_rstName;
		strTexCachePackName = strPackName + "_texcache";

		CEterPackManager::Instance().RegisterPack(strPackName.c_str(), c_rstFolder.c_str());
		CEterPackManager::Instance().RegisterPack(strTexCachePackName.c_str(), c_rstFolder.c_str());
	}
	CEterPackManager::Instance().RegisterRootPack((stFolder + std::string("root")).c_str());
#endif
	NANOEND
	return true;
}

bool RunMainScript(CPythonLauncher& pyLauncher, const char* lpCmdLine)
{
	initpack();
	initdbg();
	initime();
	initgrp();
	initgrpImage();
	initgrpText();
	initwndMgr();
	initudp();
	initapp();
	initsystem();
	initchr();
	initchrmgr();
	initPlayer();
	initItem();
	initNonPlayer();
	initTrade();
	initChat();
	initTextTail();
	initnet();
	initMiniMap();
	initProfiler();
	initEvent();
	initeffect();
	initfly();
	initsnd();
	initeventmgr();
	initshop();
	initskill();
	initquest();
	initBackground();
	initMessenger();
	initsafebox();
#if defined(__BL_MAILBOX__)
	initmail();
#endif
	initguild();
	initServerStateChecker();
	initRenderTarget();
#ifdef ENABLE_CPP_PSM
	initplayersettingsmodule();
#endif

	NANOBEGIN

	{
		std::string stRegisterDebugFlag;

#ifdef _DISTRIBUTE 
		stRegisterDebugFlag ="__DEBUG__ = 0";
#else
		stRegisterDebugFlag ="__DEBUG__ = 1"; 
#endif

		if (!pyLauncher.RunLine(stRegisterDebugFlag.c_str()))
		{
			TraceError("RegisterDebugFlag Error");
			return false;
		}
	}

	{
		std::string stRegisterCmdLine;

		const char * loginMark = "-cs";
		const char * loginMark_NonEncode = "-ncs";
		const char * seperator = " ";

		std::string stCmdLine;
		const int CmdSize = 3;
		std::vector<std::string> stVec;
		SplitLine(lpCmdLine,seperator,&stVec);
		if (CmdSize == stVec.size() && stVec[0]==loginMark)
		{
			base64_decode(stVec[2].c_str(),buf);
			stVec[2] = buf;
			string_join(seperator,stVec,&stCmdLine);
		}
		else if (CmdSize <= stVec.size() && stVec[0]==loginMark_NonEncode)
		{
			stVec[0] = loginMark;
			string_join(" ",stVec,&stCmdLine);
		}
		else
			stCmdLine = lpCmdLine;

		stRegisterCmdLine ="__COMMAND_LINE__ = ";
		stRegisterCmdLine+='"';
		stRegisterCmdLine+=stCmdLine;
		stRegisterCmdLine+='"';

		const CHAR* c_szRegisterCmdLine=stRegisterCmdLine.c_str();
		if (!pyLauncher.RunLine(c_szRegisterCmdLine))
		{
			TraceError("RegisterCommandLine Error");
			return false;
		}
	}
	{
		std::vector<std::string> stVec;
		SplitLine(lpCmdLine," " ,&stVec);

		if (stVec.size() != 0 && "--pause-before-create-window" == stVec[0])
		{
#ifdef XTRAP_CLIENT_ENABLE
			if (!XTrap_CheckInit())
				return false;
#endif
			system("pause");
		}
		if (!pyLauncher.RunFile("system.py"))
		{
			TraceError("RunMain Error");
			return false;
		}
	}

	NANOEND
	return true;
}

void SetInternetRegKey()
{
	LONG status;
	HKEY hKey;

	status = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION"), 0, KEY_ALL_ACCESS, &hKey);
	if ((status == ERROR_SUCCESS) && (hKey != NULL))
	{
		DWORD standard = 11000;
		DWORD version;
		DWORD size = sizeof(version);
		DWORD type = REG_DWORD;
		status = RegQueryValueEx(hKey, TEXT("t2.bin"), NULL, &type, (BYTE*)&version, &size);
		if (status != ERROR_SUCCESS)
		{
			status = RegSetValueEx(hKey, TEXT("t2.bin"), NULL, REG_DWORD, (BYTE*)&standard, sizeof(standard));
			if (status != ERROR_SUCCESS)
			{

			}
		}
		RegCloseKey(hKey);
	}
}

bool Main(HINSTANCE hInstance, LPSTR lpCmdLine)
{
	SetInternetRegKey();
#ifdef LOCALE_SERVICE_YMIR
	extern bool g_isScreenShotKey;
	g_isScreenShotKey = true;
#endif

	DWORD dwRandSeed=time(NULL)+DWORD(GetCurrentProcess());
	srandom(dwRandSeed);
	srand(random());

	SetLogLevel(1);

#ifdef LOCALE_SERVICE_VIETNAM_MILD
	extern BOOL USE_VIETNAM_CONVERT_WEAPON_VNUM;
	USE_VIETNAM_CONVERT_WEAPON_VNUM = true;
#endif

	if (_access("perf_game_update.txt", 0)==0)
	{
		DeleteFile("perf_game_update.txt");
	}

	if (_access("newpatch.exe", 0)==0)
	{		
		system("patchupdater.exe");
		return false;
	}
#ifndef __VTUNE__
	ilInit();
#endif
	if (!Setup(lpCmdLine))
		return false;

#ifdef _DEBUG
	OpenConsoleWindow();
#else
#endif

	static CLZO				lzo;
	static CEterPackManager	EterPackManager;

#ifdef ENABLE_HWID
	static CHwidManager		HwidManager;
#endif

	if (!PackInitialize("pack"))
	{
		LogBox("Pack Initialization failed. Check log.txt file..");
		return false;
	}

	if(LocaleService_LoadGlobal(hInstance))
		SetDefaultCodePage(LocaleService_GetCodePage());

	CPythonApplication * app = new CPythonApplication;

	app->Initialize(hInstance);

	bool ret=false;
	{
		CPythonLauncher pyLauncher;
		CPythonExceptionSender pyExceptionSender;
		SetExceptionSender(&pyExceptionSender);

		if (pyLauncher.Create())
		{
		}

		
		app->Clear();

		timeEndPeriod(1);
		pyLauncher.Clear();
	}

	app->Destroy();
	delete app;
	
	return ret;
}

HANDLE CreateMetin2GameMutex()
{
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
	sa.nLength				= sizeof(sa);
	sa.lpSecurityDescriptor	= NULL;
	sa.bInheritHandle		= FALSE;

	return CreateMutex(&sa, FALSE, "Metin2GameMutex");
}

void DestroyMetin2GameMutex(HANDLE hMutex)
{
	if (hMutex)
	{
		ReleaseMutex(hMutex);
		hMutex = NULL;
	}
}

void __ErrorPythonLibraryIsNotExist()
{
	LogBoxf("FATAL ERROR!! Python Library file not exist!");
}

bool __IsTimeStampOption(LPSTR lpCmdLine)
{
	const char* TIMESTAMP = "/timestamp";
	return (strncmp(lpCmdLine, TIMESTAMP, strlen(TIMESTAMP))==0);
}

void __PrintTimeStamp()
{
#ifdef	_DEBUG
	if (__IS_TEST_SERVER_MODE__)
		LogBoxf("METIN2 BINARY TEST DEBUG VERSION %s  ( MS C++ %d Compiled )", __TIMESTAMP__, _MSC_VER);
	else
		LogBoxf("METIN2 BINARY DEBUG VERSION %s ( MS C++ %d Compiled )", __TIMESTAMP__, _MSC_VER);
	
#else
	if (__IS_TEST_SERVER_MODE__)
		LogBoxf("METIN2 BINARY TEST VERSION %s  ( MS C++ %d Compiled )", __TIMESTAMP__, _MSC_VER);
	else
		LogBoxf("METIN2 BINARY DISTRIBUTE VERSION %s ( MS C++ %d Compiled )", __TIMESTAMP__, _MSC_VER);			
#endif			
}

bool __IsLocaleOption(LPSTR lpCmdLine)
{
	return (strcmp(lpCmdLine, "--locale") == 0);
}

bool __IsLocaleVersion(LPSTR lpCmdLine)
{
	return (strcmp(lpCmdLine, "--perforce-revision") == 0);
}

#ifdef USE_OPENID
bool __IsOpenIDAuthKeyOption(LPSTR lpCmdLine)
{
	return (strcmp(lpCmdLine, "--openid-authkey") == 0);
}

{
	return (strcmp(lpCmdLine, "--openid-test") == 0);
}
#endif 




int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	if (strstr(lpCmdLine, "--hackshield") != 0)
		return 0;

	

#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	ApplicationStringTable_Initialize(hInstance);


	LocaleService_LoadConfig("configs/locale.cfg");
	SetDefaultCodePage(LocaleService_GetCodePage());	

#ifdef XTRAP_CLIENT_ENABLE
	if (!XTrap_Init())
		return 0;
#endif

#ifdef USE_AHNLAB_HACKSHIELD
	if (!HackShield_Init())
		return 0;
#endif

#ifdef USE_NPROTECT_GAMEGUARD
	if (!GameGuard_Init())
		return 0;
#endif

#if defined(CHECK_LATEST_DATA_FILES)
	if (!CheckLatestFiles())
		return 0;
#endif

	bool bQuit = false;
	int nArgc = 0;
	PCHAR* szArgv = CommandLineToArgv( lpCmdLine, &nArgc );

	for( int i=0; i < nArgc; i++ ) {
		if(szArgv[i] == 0)
			continue;
		{
			char szModuleName[MAX_PATH];
			char szVersionPath[MAX_PATH];
			GetModuleFileName(NULL, szModuleName, sizeof(szModuleName));
			sprintf(szVersionPath, "%s.version", szModuleName);
			FILE* fp = fopen(szVersionPath, "wt");
			if (fp)
			{
				extern int METIN2_GET_VERSION();
				fprintf(fp, "r%d\n", METIN2_GET_VERSION());
				fclose(fp);
			}
			bQuit = true;
		} else if (__IsLocaleOption(szArgv[i]))
		{
			FILE* fp=fopen("locale.txt", "wt");
			fprintf(fp, "service[%s] code_page[%d]", 
				LocaleService_GetName(), LocaleService_GetCodePage());
			fclose(fp);
			bQuit = true;
		} else if (__IsTimeStampOption(szArgv[i]))
		{
			__PrintTimeStamp();
			bQuit = true;
		} else if ((strcmp(szArgv[i], "--force-set-locale") == 0))
		{
			if (nArgc <= i + 2)
			{
				MessageBox(NULL, "Invalid arguments", ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
				goto Clean;
			}

			const char* localeName = szArgv[++i];
			const char* localePath = szArgv[++i];

			LocaleService_ForceSetLocale(localeName, localePath);
		}
#ifdef USE_OPENID
		{
			if (nArgc <= i + 1)
			{
				MessageBox(NULL, "Invalid arguments", ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
				goto Clean;
			}

			const char* authKey = szArgv[++i];

			LocaleService_SetOpenIDAuthKey(authKey);

			bAuthKeyChecked = true;
		}
		else if (__IsOpenIDTestOption(szArgv[i]))
		{
			openid_test = 1;

		}
#endif 
	}

#ifdef USE_OPENID
	
	if (false == bAuthKeyChecked && !openid_test)
	{
		MessageBox(NULL, "Invalid execution", ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
		goto Clean;
	}
#endif 


	if(bQuit)
		goto Clean;

#if defined(NEEDED_COMMAND_ARGUMENT)
	if (strstr(lpCmdLine, NEEDED_COMMAND_ARGUMENT) == 0) {
		MessageBox(NULL, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, "ERR_MUST_LAUNCH_FROM_PATCHER"), ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
			goto Clean;
	}
#endif

#if defined(NEEDED_COMMAND_CLIPBOARD)
	{
		CHAR szSecKey[256];
		CPostIt cPostIt( "VOLUME1" );

		if( cPostIt.Get( "SEC_KEY", szSecKey, sizeof(szSecKey) ) == FALSE ) {
			MessageBox(NULL, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, "ERR_MUST_LAUNCH_FROM_PATCHER"), ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
			goto Clean;
		}
		if( strstr(szSecKey, NEEDED_COMMAND_CLIPBOARD) == 0 ) {
			MessageBox(NULL, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, "ERR_MUST_LAUNCH_FROM_PATCHER"), ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
			goto Clean;
		}
		cPostIt.Empty();
	}
#endif

#ifdef CEF_BROWSER
	CefWebBrowser_Startup(hInstance);
#else
	WebBrowser_Startup(hInstance);
#endif

	if (!CheckPythonLibraryFilenames())
	{
		__ErrorPythonLibraryIsNotExist();
		goto Clean;
	}

	Main(hInstance, lpCmdLine);

#ifdef USE_NPROTECT_GAMEGUARD
	GameGuard_NoticeMessage();
#endif

#ifdef CEF_BROWSER
	CefWebBrowser_Cleanup();
#else
	WebBrowser_Cleanup();
#endif

	::CoUninitialize();

	if(gs_szErrorString[0])
		MessageBox(NULL, gs_szErrorString, ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);

Clean:
#ifdef USE_AHNLAB_HACKSHIELD
	HackShield_Shutdown();
#endif
	SAFE_FREE_GLOBAL(szArgv);

	return 0;
}

static void GrannyError(granny_log_message_type Type,
	granny_log_message_origin Origin,
	char const* File,
	granny_int32x Line,
	char const* Message,
	void* UserData)
{
    TraceError("GRANNY: %s", Message);
}

int Setup(LPSTR lpCmdLine)
{
	
	TIMECAPS tc; 
	UINT wTimerRes; 

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) 
		return 0;

	wTimerRes = MINMAX(tc.wPeriodMin, 1, tc.wPeriodMax); 
	timeBeginPeriod(wTimerRes); 

	

	granny_log_callback Callback;
    Callback.Function = nullptr;
    Callback.UserData = 0;
    GrannySetLogCallback(&Callback);
	return 1;
}
