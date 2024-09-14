#ifndef __CEF3Handles_ClientHandler__
#define __CEF3Handles_ClientHandler__

#include "../../extern/include/cef/cef_render_process_handler.h"
#include "../../extern/include/cef/cef_client.h"
#include "../../extern/include/cef/cef_v8.h"
#include "../../extern/include/cef/cef_browser.h"

class ClientHandler : public CefClient, public CefLifeSpanHandler {
public:
	ClientHandler();

	CefRefPtr<CefBrowser> GetBrowser()
	{
		return m_Browser;
	}

	CefWindowHandle GetBrowserHwnd()
	{
		return m_BrowserHwnd;
	}

	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
	{
		return this;
	}

	virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

protected:
	CefRefPtr<CefBrowser> m_Browser;

	CefWindowHandle m_BrowserHwnd;

	IMPLEMENT_REFCOUNTING(ClientHandler);
};

#endif 
