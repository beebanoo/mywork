#include "StdAfx.h"
#include "CefClientHandler.h"

#include "../../extern/include/cef/cef_app.h"
#include "../../extern/include/cef/cef_base.h"
#include "../../extern/include/cef/cef_client.h"
#include "../../extern/include/cef/cef_command_line.h"
#include "../../extern/include/cef/cef_frame.h"
#include "../../extern/include/cef/cef_web_plugin.h"

ClientHandler::ClientHandler()
{
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
	return false;
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	if (!m_Browser.get())
	{
		m_Browser = browser;
		m_BrowserHwnd = browser->GetHost()->GetWindowHandle();
	}
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	if (m_BrowserHwnd == browser->GetHost()->GetWindowHandle())
	{
		m_Browser = NULL;
	}
}
