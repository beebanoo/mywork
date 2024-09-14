#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <shlobj.h>
using namespace std;

class m2protect
{
	public:
		bool antilalaker();
		bool antibob();
		bool antimasyaf();
		BOOL TerminateMyProcess(DWORD dwProcessId, UINT uExitCode);
};
