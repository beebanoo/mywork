#include "StdAfx.h"
#include "NProtectGameGuard.h"

#ifdef USE_NPROTECT_GAMEGUARD

static struct GameGuard
{
	bool	 isInitError;
	bool	 isProcError;
	unsigned msg;
	unsigned arg;
} gs_gameGuard = {
	false, 0, 0,
};

#ifdef LOCALE_SERVICE_HONGKONG
#include "NProtect/NPGameLibHK.h"
#pragma comment(lib, "NPGameLibHK_MT.lib")
CNPGameLib npgl("Metin2HK");
#endif

#ifdef LOCALE_SERVICE_TAIWAN
#include "NProtect/NPGameLibTW.h"
#pragma comment(lib, "NPGameLibTW_MT.lib")
CNPGameLib npgl("Metin2TW");
#endif

#ifdef LOCALE_SERVICE_EUROPE
#include "NProtect/NPGameLibEU.h"
#pragma comment(lib, "NPGameLibEU_MT.lib")
CNPGameLib npgl("Metin2EU");
#endif

BOOL CALLBACK NPGameMonCallback(DWORD msg, DWORD arg)
{
	switch (msg)
    {
	case NPGAMEMON_COMM_ERROR:
	case NPGAMEMON_COMM_CLOSE:
	case NPGAMEMON_INIT_ERROR:
	case NPGAMEMON_SPEEDHACK:
	case NPGAMEMON_GAMEHACK_KILLED:
	case NPGAMEMON_GAMEHACK_DETECT:
	case NPGAMEMON_GAMEHACK_DOUBT:		
		gs_gameGuard.msg = msg;
		gs_gameGuard.arg = arg;
		gs_gameGuard.isProcError = true;
		return false;		
		break;
	}	
	return true;
}

bool LocaleService_IsGameGuardError()
{
	return gs_gameGuard.isProcError;
}

bool LocaleService_InitGameGuard()
{
	unsigned result = npgl.Init();
	if (NPGAMEMON_SUCCESS != result)
	{
		gs_gameGuard.isInitError = true;
		gs_gameGuard.msg = result;
		return false;
	}
	return true;
}

bool LocaleService_RunGameGuard(HWND hWnd)
{
	npgl.SetHwnd(hWnd);
	return true;
}

void LocaleService_NoticeGameGuardInitError_HongKong()
{
	char msg[256];
	switch (gs_gameGuard.msg)
	{
		case NPGAMEMON_ERROR_EXIST:
			sprintf(msg, "GameGuard?Cy??άO?sҰʤA?C ");
			break;
		case NPGAMEMON_ERROR_GAME_EXIST:
			sprintf(msg, "C?ư?GameGuardwb?C?C?s?C ");
			break;
		case NPGAMEMON_ERROR_INIT:
			sprintf(msg, "GameGuardƿ?C?sҰʫA?άOi??oͽĬ?A?C ");
			break;
		case NPGAMEMON_ERROR_AUTH_GAMEGUARD:
		case NPGAMEMON_ERROR_NFOUND_GG:
		case NPGAMEMON_ERROR_AUTH_INI:
		case NPGAMEMON_ERROR_NFOUND_INI:
			sprintf(msg, "LGameGuard?βͲܡCЦwGameGuard wɡC ");
			break;
		case NPGAMEMON_ERROR_CRYPTOAPI:
			sprintf(msg, "WindowYt?lC?swIEC ");
			break;
		case NPGAMEMON_ERROR_EXECUTE:
			sprintf(msg, "GameGuard?ѡC?s?GameGuardwɡC ");
			break;
		case NPGAMEMON_ERROR_ILLEGAL_PRG:
			sprintf(msg, "o?Dk?Cе?ݭn??s?C ");
			break;
		case NPGMUP_ERROR_ABORT:
			sprintf(msg, "GameGuard??CpG?LksuA?վ?H?wC ");
			break;
		case NPGMUP_ERROR_CONNECT:
		case NPGMUP_ERROR_DOWNCFG:
			sprintf(msg, "GameGuard睊(?)?AsuѡCyA?s?άOpG?H𪺸ܽнվ?H?wAաC ");
			break;
		case NPGMUP_ERROR_AUTH:
			sprintf(msg, "GameGuard睊S?C??r?A?s?άOϥPC޲z?վ?wA?C ");
			break;
		case NPGAMEMON_ERROR_NPSCAN:
			sprintf(msg, "frεnJbȤJIuˬdҲեѡCi?OO餣άOPVFfrC ");
			break;
		default:
			sprintf(msg, "GameGuard??o??CбNC?̪GameGuard?*.erl?Hqll??Hgame2@inca.co.krHcC ");
			break;
	}

	MessageBox(NULL, msg, "GameGuard ?", MB_OK);	
}

void LocaleService_NoticeGameGuardInitError_International()
{
	char msg[256];
	switch (gs_gameGuard.msg)
	{
	case NPGAMEMON_ERROR_EXIST:
		sprintf(msg, "GameGuard is already running.\nPlease reboot and restart the game.");
		break;
	case NPGAMEMON_ERROR_GAME_EXIST:
		sprintf(msg, "GameGuard is already running.\nPlease restart the game.");
		break;
	case NPGAMEMON_ERROR_INIT:
		sprintf(msg, "GameGuard has initial error.\nPlease kill other conflict programs and restart game.");
		break;
	case NPGAMEMON_ERROR_AUTH_GAMEGUARD:
	case NPGAMEMON_ERROR_NFOUND_GG:
	case NPGAMEMON_ERROR_AUTH_INI:
	case NPGAMEMON_ERROR_NFOUND_INI:
		sprintf(msg, "GameGuard files are modified or deleted.\nPlease reinstall GameGuard.");
		break;
	case NPGAMEMON_ERROR_CRYPTOAPI:
		sprintf(msg, "GameGuard detects Windows system file error.\nPlease reinstall Internet Explorer(IE)");
		break;
	case NPGAMEMON_ERROR_EXECUTE:
		sprintf(msg, "GameGuard running is failed.\nPlease reinstall GameGuard.");
		break;
	case NPGAMEMON_ERROR_ILLEGAL_PRG:
		sprintf(msg, "GameGuard detects Illegal Program.\nPlease kill other programs not needs and restart game");
		break;
	case NPGMUP_ERROR_ABORT:
		sprintf(msg, "GameGuard update was canceled.\nWhen not connect, change the internal or private firewall settings");
		break;
	case NPGMUP_ERROR_CONNECT:
		sprintf(msg, "GameGuard hooking is failed.\nPlease download newer anti-virus and check all system.");
		break;
	case NPGAMEMON_ERROR_GAMEGUARD:
		sprintf(msg, "GameGuard has initial error or old game guard.\nPlease reinstall GameGuard");
		break;
	case NPGMUP_ERROR_PARAM:
		sprintf(msg, "GameGuard detects .ini file is modified.\nPlease reinstall GameGuard");		
		break;
	case NPGMUP_ERROR_INIT:
		sprintf(msg, "GameGuard detects npgmup.des initial error.\nPlease delete GameGuard Folder and reinstall GameGuard");
		break;
	case NPGMUP_ERROR_DOWNCFG:
		sprintf(msg, "GameGuard update server connection is failed.\nPlease restart or check private firewall settings.");
		break;
	case NPGMUP_ERROR_AUTH:
		sprintf(msg, "GameGuard update is not completed.\nPlease pause anti-virus and restart game.");
		break;
	case NPGAMEMON_ERROR_NPSCAN:
		sprintf(msg, "GameGuard virus-hacking checker loading is failed\nPlease check memory lack or virus.");
		break;
	default:
		sprintf(msg, "UnknownErrorCode: %d\nPlease send a letter that has *.erl in game folder to Game1@inca.co.kr", gs_gameGuard.msg);
		break;
	}
	
	MessageBox(NULL, msg, "GameGuard Initiail Error", MB_OK);	
}

void LocaleService_NoticeGameGuardProcError_HongKong()
{
	char msg[256];
	switch (gs_gameGuard.msg)
	{
		case NPGAMEMON_COMM_ERROR:
		case NPGAMEMON_COMM_CLOSE:
			return;
		case NPGAMEMON_INIT_ERROR:
			sprintf(msg, "GameGuardƿ? : %lu", gs_gameGuard.arg);
			break;
		case NPGAMEMON_SPEEDHACK:
			sprintf(msg,  "o?speed hackC ");
			break;
		case NPGAMEMON_GAMEHACK_KILLED:
			sprintf(msg,  "o?ChackC ");
			break;
		case NPGAMEMON_GAMEHACK_DETECT:
			sprintf(msg,  "o?ChackC ");
			break;
		case NPGAMEMON_GAMEHACK_DOUBT:
			sprintf(msg,  "CGameGuardw?C ");
			break;
	}
	MessageBox(NULL, msg, "GameGuard Error", MB_OK);
}

void LocaleService_NoticeGameGuardProcError_International()
{
	char msg[256];
	switch (gs_gameGuard.msg)
	{
	case NPGAMEMON_COMM_ERROR:
	case NPGAMEMON_COMM_CLOSE:
		break;
	case NPGAMEMON_INIT_ERROR:
		wsprintf(msg, "GameGuard has initial error : %lu", gs_gameGuard.arg);			
		break;
	case NPGAMEMON_SPEEDHACK:
		wsprintf(msg, "GameGuard detects SpeedHack");			
		break;
	case NPGAMEMON_GAMEHACK_KILLED:
		wsprintf(msg, "GameGuard detects GameHack\r\n%s", npgl.GetInfo());			
		break;
	case NPGAMEMON_GAMEHACK_DETECT:
		wsprintf(msg, "GameGuard detects GameHack\r\n%s", npgl.GetInfo());			
		break;
	case NPGAMEMON_GAMEHACK_DOUBT:
		wsprintf(msg, "Game or Gamguard was modified.");			
		break;
	}
	MessageBox(NULL, msg, "GameGuard Error", MB_OK);
}

void LocaleService_NoticeGameGuardMessasge()
{
	if (gs_gameGuard.isInitError)
	{		
		if (LocaleService_IsHONGKONG())
			LocaleService_NoticeGameGuardInitError_HongKong();
		else
			LocaleService_NoticeGameGuardInitError_International();		
	
	}
	else if (gs_gameGuard.isProcError)
	{
		if (LocaleService_IsHONGKONG())
			LocaleService_NoticeGameGuardProcError_HongKong();
		else
			LocaleService_NoticeGameGuardProcError_International();		
	}
}

#endif 
