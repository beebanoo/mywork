#pragma once

enum EPythonCheatTypes
{
	CHEAT_TYPE_NONE,
	CHEAT_TYPE_EMPTY1,
	CHEAT_TYPE_EMPTY2,

	
	CHEAT_TYPE_chat_AppendChat,
	CHEAT_TYPE_chat_AppendChatWithDelay,
	CHEAT_TYPE_chat_AppendWhisper,
	CHEAT_TYPE_chat_GetLinkFromHyperlink,

	
	CHEAT_TYPE_nonplayer_GetMonsterName,
	CHEAT_TYPE_nonplayer_GetLevelByVID,

	
	CHEAT_TYPE_background_GlobalPositionToMapInfo,
	CHEAT_TYPE_background_GetCurrentMapName,
	CHEAT_TYPE_background_EnableSnow,
	CHEAT_TYPE_background_SetEnvironmentData,
	CHEAT_TYPE_background_RegisterEnvironmentData,
	CHEAT_TYPE_background_LoadMap,

	
	CHEAT_TYPE_shop_AddPrivateShopItemStock,
	CHEAT_TYPE_shop_ClearPrivateShopStock,
	CHEAT_TYPE_shop_GetItemPrice,
	CHEAT_TYPE_shop_GetItemMetinSocket,
	CHEAT_TYPE_shop_GetItemCount,
	CHEAT_TYPE_shop_GetItemID,

	
	CHEAT_TYPE_ime_PasteString,

	
	CHEAT_TYPE_event_SelectAnswer,
	CHEAT_TYPE_event_QuestButtonClick,

	
	CHEAT_TYPE_skill_IsToggleSkill,
	CHEAT_TYPE_skill_GetSkillName,
	CHEAT_TYPE_skill_CanUseSkill,

	
	CHEAT_TYPE_pack_Get,

	CHEAT_TYPE_MAX,
};

class CPythonStackController : public CSingleton <CPythonStackController>
{
public:
	CPythonStackController();
	virtual ~CPythonStackController();

	void CheckStackReference(int iTypeId, const std::string & szRefFileName, const std::string & szRefFuncName);

	std::string GetReferenceFileName();
	std::string GetReferenceFunctionName();

protected:
	void CheckStackReferenceEx(int iTypeId, const std::string & szRefFileName, const std::string & szRefFuncName);
	void SendViolationMessage(int iTypeId, const std::string & szRefFileName, const std::string & szRefFuncName);
};

#define PY_REF_FILE CPythonStackController::Instance().GetReferenceFileName()
#define PY_REF_FUNC CPythonStackController::Instance().GetReferenceFunctionName()

