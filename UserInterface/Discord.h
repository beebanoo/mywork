#pragma once
#include "StdAfx.h"
#include "PythonCharacterManager.h"
#include "PythonBackground.h"
#include "PythonPlayer.h"
#include "PythonGuild.h"

namespace Discord
{
	inline constexpr auto DiscordClientID = "1176600910072389653";

	using DCDATA = std::pair<std::string, std::string>;
	

	
	inline DCDATA GetNameData()
	{
		
		auto WarpName = std::string(CPythonBackground::Instance().GetWarpMapName());

		static const std::map<std::string, std::string> m_MapName {
			{ "metin2_map_a1", "Yongan" },
			{ "metin2_map_b1", "Joan" },
			{ "metin2_map_c1", "Pyungmoo" },
		};

		auto MapName = "Location: " + (m_MapName.count(WarpName) ? m_MapName.at(WarpName) : WarpName);

		
		auto CHName = std::string(CPythonPlayer::Instance().GetName());
		if (CHName == "[Dev]WickR")
		{
			CHName = "DEVELOPER";
			MapName = "";
		}
		else if (CHName == "[Own]Lead")
		{
			CHName = "OWNER";
			MapName = "";
		}
		else
			CHName = "Name: " + std::string(CPythonPlayer::Instance().GetName());
		std::string GuildName;
		if (CPythonGuild::Instance().GetGuildName(CPythonPlayer::Instance().GetGuildID(), &GuildName))
			CHName += "-Guild: " + GuildName;

		return { MapName, CHName };
	}

	
	inline DCDATA GetRaceData()
	{
		auto pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (!pInstance)
			return { "","" };

		auto RACENUM = pInstance->GetRace();

		
		auto RaceImage = "race_" + std::to_string(RACENUM);

		
		auto RaceName = "Warrior";
		switch (RACENUM)
		{
		case NRaceData::JOB_ASSASSIN:
		case NRaceData::JOB_ASSASSIN + 4:
			RaceName = "Assassin";
			break;
		case NRaceData::JOB_SURA:
		case NRaceData::JOB_SURA + 4:
			RaceName = "Sura";
			break;
		case NRaceData::JOB_SHAMAN:
		case NRaceData::JOB_SHAMAN + 4:
			RaceName = "Shaman";
			break;
		}
		return { RaceImage , RaceName };
	}

	
	inline DCDATA GetEmpireData()
	{
		auto pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (!pInstance)
			return { "","" };

		auto EmpireID = pInstance->GetEmpireID();

		
		auto EmpireImage = "empire_" + std::to_string(EmpireID);

		
		auto EmpireName = "Shinsoo";
		switch (EmpireID)
		{
		case 2:
			EmpireName = "Chunjo";
			break;
		case 3:
			EmpireName = "Jinno";
		}
		return { EmpireImage, EmpireName};
	}
}
