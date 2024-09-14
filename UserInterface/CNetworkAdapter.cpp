#include "stdafx.h"

#include "CNetworkAdapter.h"

#define ALLOCATE_FROM_PROCESS_HEAP( bytes )		::HeapAlloc( ::GetProcessHeap(), HEAP_ZERO_MEMORY, bytes )
#define DEALLOCATE_FROM_PROCESS_HEAP( ptr )		if( ptr ) ::HeapFree( ::GetProcessHeap(), 0, ptr )
#define REALLOC_FROM_PROCESS_HEAP( ptr, bytes )	::HeapReAlloc( ::GetProcessHeap(), HEAP_ZERO_MEMORY, ptr, bytes )

DWORD EnumNetworkAdapters(CNetworkAdapter* pAdapters, ULONG ulSzBuf, LPDWORD lpdwOutSzBuf) {
	IP_ADAPTER_INFO* pAdptInfo = NULL;
	IP_ADAPTER_INFO* pNextAd = NULL;
	ULONG ulLen = 0;
	int nCnt = 0;
	DWORD erradapt;

	erradapt = ::GetAdaptersInfo(pAdptInfo, &ulLen);
	if (erradapt == ERROR_BUFFER_OVERFLOW) {
		pAdptInfo = (IP_ADAPTER_INFO*)ALLOCATE_FROM_PROCESS_HEAP(ulLen);
		erradapt = ::GetAdaptersInfo(pAdptInfo, &ulLen);
	}

	pNextAd = pAdptInfo;
	while (pNextAd) {
		nCnt++;
		pNextAd = pNextAd->Next;
	}

	*lpdwOutSzBuf = nCnt * sizeof(CNetworkAdapter);
	if (ulSzBuf < *lpdwOutSzBuf) {
		DEALLOCATE_FROM_PROCESS_HEAP(pAdptInfo);
		return ERROR_INSUFFICIENT_BUFFER;
	}

	nCnt = 0;

	if (erradapt == ERROR_SUCCESS) {
		pNextAd = pAdptInfo;

		while (pNextAd) {
			pAdapters[nCnt].SetupAdapterInfo(pNextAd);
			pNextAd = pNextAd->Next;
			nCnt++;
		}
	}

	DEALLOCATE_FROM_PROCESS_HEAP(pAdptInfo);

	return ERROR_SUCCESS;
}

CNetworkAdapter::CNetworkAdapter(): m_tLeaseObtained(0), m_tLeaseExpires(0)
{
	m_bDhcpUsed = FALSE;
	m_bWinsUsed = FALSE;
	m_dwIndex = 0;
	m_nAdapterType = 0;

	::memset(&m_ucAddress.ucAddress, 0, MAX_ADAPTER_ADDRESS_LENGTH);
	m_ucAddress.nLen = 0;

	m_DnsAddresses.clear();
	m_IpAddresses.clear();
	m_GatewayList.clear();

	return;
}

CNetworkAdapter::~CNetworkAdapter() {
#ifdef _DEBUG
	::memset(&m_ucAddress.ucAddress, 0, MAX_ADAPTER_ADDRESS_LENGTH);
	m_ucAddress.nLen = 0;
	m_bDhcpUsed = FALSE;
	m_bWinsUsed = FALSE;
	m_dwIndex = 0;
	m_nAdapterType = 0;
#endif

	m_DnsAddresses.clear();
	m_IpAddresses.clear();
	m_GatewayList.clear();

	return;
}

BOOL CNetworkAdapter::SetupAdapterInfo(IP_ADAPTER_INFO* pAdaptInfo) {
	BOOL bSetupPassed = FALSE;
	IP_ADDR_STRING* pNext = NULL;
	IP_PER_ADAPTER_INFO* pPerAdapt = NULL;
	ULONG ulLen = 0;
	DWORD err;

	_IPINFO iphold;

	if (pAdaptInfo) {
#ifndef _UNICODE
		m_sName = pAdaptInfo->AdapterName;
		m_sDesc = pAdaptInfo->Description;
#else
		USES_CONVERSION;
		m_sName = A2W(pAdaptInfo->AdapterName);
		m_sDesc = A2W(pAdaptInfo->Description);
#endif

		m_sPriWins = pAdaptInfo->PrimaryWinsServer.IpAddress.String;
		m_sSecWins = pAdaptInfo->SecondaryWinsServer.IpAddress.String;
		m_dwIndex = pAdaptInfo->Index;
		m_nAdapterType = pAdaptInfo->Type;
		m_bDhcpUsed = pAdaptInfo->DhcpEnabled;
		m_bWinsUsed = pAdaptInfo->HaveWins;
		m_tLeaseObtained = pAdaptInfo->LeaseObtained;
		m_tLeaseExpires = pAdaptInfo->LeaseExpires;
		m_sDhcpAddr = pAdaptInfo->DhcpServer.IpAddress.String;

		m_ucAddress.nLen = pAdaptInfo->AddressLength;
		for (int i = 0; i < (int)m_ucAddress.nLen; i++)
		{
			m_ucAddress.ucAddress[i] = pAdaptInfo->Address[i];
		}

		if (pAdaptInfo->CurrentIpAddress) {
			m_sCurIpAddr.sIp = pAdaptInfo->CurrentIpAddress->IpAddress.String;
			m_sCurIpAddr.sSubnet = pAdaptInfo->CurrentIpAddress->IpMask.String;
		}
		else{
			m_sCurIpAddr.sIp = _T("0.0.0.0");
			m_sCurIpAddr.sSubnet = _T("0.0.0.0");
		}

		pNext = &(pAdaptInfo->IpAddressList);
		while (pNext) {
			iphold.sIp = pNext->IpAddress.String;
			iphold.sSubnet = pNext->IpMask.String;
			m_IpAddresses.push_back(iphold);
			pNext = pNext->Next;
		}

		pNext = &(pAdaptInfo->GatewayList);
		while (pNext) {
			m_GatewayList.push_back(pNext->IpAddress.String);
			pNext = pNext->Next;
		}

		err = ::GetPerAdapterInfo(m_dwIndex, pPerAdapt, &ulLen);
		if (err == ERROR_BUFFER_OVERFLOW) {
			pPerAdapt = (IP_PER_ADAPTER_INFO*)ALLOCATE_FROM_PROCESS_HEAP(ulLen);
			err = ::GetPerAdapterInfo(m_dwIndex, pPerAdapt, &ulLen);

			if (err == ERROR_SUCCESS) {
				pNext = &(pPerAdapt->DnsServerList);
				while (pNext) {
					m_DnsAddresses.push_back(pNext->IpAddress.String);
					pNext = pNext->Next;
				}
				bSetupPassed = TRUE;
			}

			DEALLOCATE_FROM_PROCESS_HEAP(pPerAdapt);
		}
	}

	return bSetupPassed;
}

tstring	CNetworkAdapter::GetStringFromArray(const StringArray* pPtr, int nIndex) const {
	tstring sStr = _T("");
	if (pPtr && ((SIZE_T)nIndex < pPtr->size())) {
		sStr = (*pPtr)[nIndex];
	}

	return sStr;
}

tstring CNetworkAdapter::GetAdapterTypeString(UINT nType) {
	tstring sType = _T("");
	switch (nType) {
	case MIB_IF_TYPE_OTHER:		sType = _T("Other");		break;
	case MIB_IF_TYPE_ETHERNET:	sType = _T("Ethernet");		break;
	case MIB_IF_TYPE_TOKENRING:	sType = _T("Token Ring");	break;
	case MIB_IF_TYPE_FDDI:		sType = _T("FDDI");			break;
	case MIB_IF_TYPE_PPP:		sType = _T("PPP");			break;
	case MIB_IF_TYPE_LOOPBACK:	sType = _T("Loopback");		break;
	case MIB_IF_TYPE_SLIP:		sType = _T("SLIP");			break;
	default: sType = _T("Invalid Adapter Type");			break;
	};

	return sType;
}


tstring CNetworkAdapter::GetAdapterName()				const { return m_sName; }
tstring CNetworkAdapter::GetAdapterDescription()		const { return m_sDesc; }
time_t CNetworkAdapter::GetLeaseObtained()				const { return m_tLeaseObtained; }
time_t CNetworkAdapter::GetLeaseExpired()				const { return m_tLeaseExpires; }
SIZE_T	CNetworkAdapter::GetNumIpAddrs()				const { return m_IpAddresses.size(); }
SIZE_T	CNetworkAdapter::GetNumDnsAddrs()				const { return m_DnsAddresses.size(); }
tstring	CNetworkAdapter::GetDnsAddr(int nDns)			const { return GetStringFromArray(&m_DnsAddresses, nDns); }
tstring CNetworkAdapter::GetCurrentIpAddress()			const { return m_sCurIpAddr.sIp; }
BOOL CNetworkAdapter::IsDhcpUsed()						const { return m_bDhcpUsed; }
tstring	CNetworkAdapter::GetDchpAddr()					const { return m_sDhcpAddr; }
BOOL CNetworkAdapter::IsWinsUsed()						const { return m_bWinsUsed; }
tstring CNetworkAdapter::GetPrimaryWinsServer()			const { return m_sPriWins; }
tstring CNetworkAdapter::GetSecondaryWinsServer()		const { return m_sSecWins; }
tstring	CNetworkAdapter::GetGatewayAddr(int nGateway) const { return GetStringFromArray(&m_GatewayList, nGateway); }
SIZE_T	CNetworkAdapter::GetNumGatewayAddrs()			const { return m_GatewayList.size(); }
DWORD CNetworkAdapter::GetAdapterIndex()				const { return m_dwIndex; }
UINT CNetworkAdapter::GetAdapterType()					const { return m_nAdapterType; }

tstring	CNetworkAdapter::GetIpAddr(int nIp) const {
	tstring sAddr = _T("");
	if ((SIZE_T)nIp < m_IpAddresses.size()) {
		sAddr = m_IpAddresses[nIp].sIp;
	}

	return sAddr;
}

tstring CNetworkAdapter::GetSubnetForIpAddr(int nIp) const {
	tstring sAddr = _T("");
	if ((SIZE_T)nIp < m_IpAddresses.size()) {
		sAddr = m_IpAddresses[nIp].sSubnet;
	}
	return sAddr;
}


tstring CNetworkAdapter::GetAdapterAddress(void)
{
	tstring sTemp = "";
	char macTemp[8];
	for (unsigned int i = 0; i < m_ucAddress.nLen; i++)
	{
		if (i > 0)
		{
			sTemp += "::";
		}

		sprintf_s(macTemp, sizeof(macTemp), "%02X", m_ucAddress.ucAddress[i]);
		sTemp += macTemp;
	}

	return sTemp;
}
