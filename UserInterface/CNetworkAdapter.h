#ifndef _ADAPTER_INFO_H____
#define _ADAPTER_INFO_H____
#include <windows.h>
#include <tchar.h>
#include <iphlpapi.h>
#include <ipifcons.h>
#include <vector>

using namespace std;

class CNetworkAdapter;

#ifndef tstring
#ifdef _UNICODE
#define tstring			std::wstring
#else
#define tstring			std::string
#endif
#endif

#pragma comment( lib, "iphlpapi.lib" )

#define DEFAULT_GATEWAY_ADDR	0

struct _IPINFO {
	tstring sIp;
	tstring sSubnet;
};

class CIpInfoArray : public vector < _IPINFO > {};
class StringArray : public vector < tstring > {};

DWORD EnumNetworkAdapters(CNetworkAdapter* lpBuffer, ULONG ulSzBuf, LPDWORD lpdwOutSzBuf);

class CNetworkAdapter {
public:
	CNetworkAdapter();
	~CNetworkAdapter();
	BOOL SetupAdapterInfo(IP_ADAPTER_INFO* pAdaptInfo);

	tstring GetAdapterName() const;
	tstring GetAdapterDescription() const;

	time_t	GetLeaseObtained() const;
	time_t	GetLeaseExpired() const;

	SIZE_T	GetNumIpAddrs() const;
	SIZE_T	GetNumDnsAddrs() const;
	tstring	GetIpAddr(int nIp = 0) const;
	tstring GetSubnetForIpAddr(int nIp = 0) const;
	tstring	GetDnsAddr(int nDns = 0) const;
	tstring GetCurrentIpAddress() const;

	BOOL	IsDhcpUsed() const;
	tstring	GetDchpAddr() const;

	BOOL	IsWinsUsed() const;
	tstring GetPrimaryWinsServer() const;
	tstring GetSecondaryWinsServer() const;

	tstring	GetGatewayAddr(int nGateway = DEFAULT_GATEWAY_ADDR) const;
	SIZE_T	GetNumGatewayAddrs() const;

	static	tstring GetAdapterTypeString(UINT nType);
	UINT	GetAdapterType() const;

	DWORD	GetAdapterIndex() const;

protected:
	tstring	GetStringFromArray(const StringArray* pPtr, int nIndex) const;

private:
	tstring			m_sDesc;
	tstring			m_sPriWins;
	tstring			m_sSecWins;
	tstring			m_sDefGateway;
	tstring			m_sDhcpAddr;
	UINT			m_nAdapterType;
	BOOL			m_bDhcpUsed;
	BOOL			m_bWinsUsed;
	StringArray		m_DnsAddresses;
	CIpInfoArray	m_IpAddresses;
	StringArray		m_GatewayList;
	time_t			m_tLeaseObtained;
	time_t			m_tLeaseExpires;

	struct UNNAMED {
		BYTE	ucAddress[MAX_ADAPTER_ADDRESS_LENGTH];
		UINT	nLen;
	} m_ucAddress;
public:
	tstring GetAdapterAddress(void);
};
