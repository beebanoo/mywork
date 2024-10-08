#pragma once

#include "../milesLib/Type.h"
#include "RaceMotionDataEvent.h"

class CRaceMotionData
{
	public:
		enum EType
		{
			TYPE_NONE,
			TYPE_WAIT,
			TYPE_MOVE,
			TYPE_ATTACK,
			TYPE_COMBO,
			TYPE_DAMAGE,
			TYPE_KNOCKDOWN,
			TYPE_DIE,
			TYPE_SKILL,
			TYPE_STANDUP,
			TYPE_EVENT,
			TYPE_FISHING,
			TYPE_NUM,
		};

		enum
		{
			SKILL_NUM = 255,
		};

		enum EMode
		{
			MODE_RESERVED,
			MODE_GENERAL,

			MODE_ONEHAND_SWORD,
			MODE_TWOHAND_SWORD,
			MODE_DUALHAND_SWORD,
			MODE_BOW,
			MODE_FAN,
			MODE_BELL,
			MODE_FISHING,

			MODE_HORSE,
			MODE_HORSE_ONEHAND_SWORD,
			MODE_HORSE_TWOHAND_SWORD,
			MODE_HORSE_DUALHAND_SWORD,
			MODE_HORSE_BOW,
			MODE_HORSE_FAN,
			MODE_HORSE_BELL,

			MODE_WEDDING_DRESS,

			MODE_MAX_NUM,
		};

		enum EName
		{
			NAME_SPECIAL_3,
			NAME_SPECIAL_4,
			NAME_SPECIAL_5,
			NAME_SPECIAL_6,
			NAME_SKILL = 50,
			NAME_SKILL_END = NAME_SKILL+SKILL_NUM,

			NAME_CLAP,

			NAME_CHEERS_1,
			NAME_CHEERS_2,

			NAME_KISS_START,
			NAME_KISS_WITH_WARRIOR = NAME_KISS_START + 0,
			NAME_KISS_WITH_ASSASSIN = NAME_KISS_START + 1,
			NAME_KISS_WITH_SURA = NAME_KISS_START + 2,
			NAME_KISS_WITH_SHAMAN = NAME_KISS_START + 3,

			NAME_FRENCH_KISS_START,
			NAME_FRENCH_KISS_WITH_WARRIOR = NAME_FRENCH_KISS_START + 0,
			NAME_FRENCH_KISS_WITH_ASSASSIN = NAME_FRENCH_KISS_START + 1,
			NAME_FRENCH_KISS_WITH_SURA = NAME_FRENCH_KISS_START + 2,
			NAME_FRENCH_KISS_WITH_SHAMAN = NAME_FRENCH_KISS_START + 3,

			NAME_SLAP_HIT_START,
			NAME_SLAP_HIT_WITH_WARRIOR = NAME_SLAP_HIT_START + 0,
			NAME_SLAP_HIT_WITH_ASSASSIN = NAME_SLAP_HIT_START + 1,
			NAME_SLAP_HIT_WITH_SURA = NAME_SLAP_HIT_START + 2,
			NAME_SLAP_HIT_WITH_SHAMAN = NAME_SLAP_HIT_START + 3,

			NAME_SLAP_HURT_START,
			NAME_SLAP_HURT_WITH_WARRIOR = NAME_SLAP_HURT_START + 0,
			NAME_SLAP_HURT_WITH_ASSASSIN = NAME_SLAP_HURT_START + 1,
			NAME_SLAP_HURT_WITH_SURA = NAME_SLAP_HURT_START + 2,
			NAME_SLAP_HURT_WITH_SHAMAN = NAME_SLAP_HURT_START + 3,

			NAME_DIG,

			NAME_DANCE_1,
			NAME_DANCE_2,
			NAME_DANCE_3,
			NAME_DANCE_4,
			NAME_DANCE_5,
			
			NAME_DANCE_END = NAME_DANCE_1 + 16,

			NAME_CONGRATULATION,
			NAME_FORGIVE,
			NAME_ANGRY,
			NAME_ATTRACTIVE,
			NAME_SAD,
			NAME_SHY,
			NAME_CHEERUP,
			NAME_BANTER,
			NAME_JOY,

			NAME_MAX_NUM,
		};

		enum EMotionEventType
		{
			MOTION_EVENT_TYPE_NONE,

			MOTION_EVENT_TYPE_EFFECT,
			MOTION_EVENT_TYPE_SCREEN_WAVING,
			MOTION_EVENT_TYPE_SCREEN_FLASHING,
			MOTION_EVENT_TYPE_SPECIAL_ATTACKING,
			MOTION_EVENT_TYPE_SOUND,
			MOTION_EVENT_TYPE_FLY,
			MOTION_EVENT_TYPE_CHARACTER_SHOW,
			MOTION_EVENT_TYPE_CHARACTER_HIDE,
			MOTION_EVENT_TYPE_WARP,
			MOTION_EVENT_TYPE_EFFECT_TO_TARGET,

			MOTION_EVENT_TYPE_MAX_NUM,
		};

		typedef struct SComboInputData
		{
			float fInputStartTime;
			float fNextComboTime;
			float fInputEndTime;
		} TComboInputData;

		typedef struct NMotionEvent::SMotionEventData				TMotionEventData;
		typedef struct NMotionEvent::SMotionEventDataScreenWaving	TScreenWavingEventData;
		typedef struct NMotionEvent::SMotionEventDataScreenFlashing	TScreenFlashingEventData;
		typedef struct NMotionEvent::SMotionEventDataEffect			TMotionEffectEventData;
		typedef struct NMotionEvent::SMotionEventDataFly			TMotionFlyEventData;
		typedef struct NMotionEvent::SMotionEventDataAttack			TMotionAttackingEventData;
		typedef struct NMotionEvent::SMotionEventDataSound			TMotionSoundEventData;
		typedef struct NMotionEvent::SMotionEventDataCharacterShow	TMotionCharacterShowEventData;
		typedef struct NMotionEvent::SMotionEventDataCharacterHide	TMotionCharacterHideEventData;
		typedef struct NMotionEvent::SMotionEventDataWarp			TMotionWarpEventData;
		typedef struct NMotionEvent::SMotionEventDataEffectToTarget	TMotionEffectToTargetEventData;
		typedef std::vector<TMotionEventData*> TMotionEventDataVector;

	public:
		static CRaceMotionData* New();
		static void Delete(CRaceMotionData* pkData);

		static void CreateSystem(UINT uCapacity);
		static void DestroySystem();

	public:
		CRaceMotionData();
		virtual ~CRaceMotionData();

		void			Initialize();
		void			Destroy();

		void			SetName(UINT eName);


		int				GetLoopCount() const;

		const char *	GetMotionFileName() const;
		const char *	GetSoundScriptFileName() const;

		void			SetMotionDuration(float fDur);
		float			GetMotionDuration();

		bool			IsAccumulationMotion();
		void			SetAccumulationPosition(const TPixelPosition & c_rPos);
		const			TPixelPosition & GetAccumulationPosition() { return m_accumulationPosition; }


		float			GetComboInputStartTime() const;
		float			GetNextComboTime() const;
		float			GetComboInputEndTime() const;

		BOOL			isAttackingMotion() const;
		const NRaceData::TMotionAttackData * GetMotionAttackDataPointer() const;
		const NRaceData::TMotionAttackData & GetMotionAttackDataReference() const;
		BOOL			HasSplashMotionEvent() const;

		BOOL			IsCancelEnableSkill() const;

		BOOL			IsLoopMotion() const;
		float			GetLoopStartTime() const;
		float			GetLoopEndTime() const;

		DWORD			GetMotionEventDataCount() const;
		BOOL			GetMotionEventDataPointer(BYTE byIndex, const CRaceMotionData::TMotionEventData ** c_ppData) const;
		BOOL			GetMotionAttackingEventDataPointer(BYTE byIndex, const CRaceMotionData::TMotionAttackingEventData ** c_ppMotionEventData) const;
		int				GetEventType(DWORD dwIndex) const;
		float			GetEventStartTime(DWORD dwIndex) const;

		const NSound::TSoundInstanceVector * GetSoundInstanceVectorPointer() const;

#ifdef WORLD_EDITOR
		bool			SaveMotionData(const char * c_szFileName);
#endif
		bool			LoadMotionData(const char * c_szFileName);
		bool			LoadSoundScriptData(const char * c_szFileName);

	protected:
		void			SetType(UINT eType);

	protected:
		UINT							m_eType;
		UINT							m_eName;
		BOOL							m_isLock;
		int								m_iLoopCount;

		std::string						m_strMotionFileName;
		std::string						m_strSoundScriptDataFileName;
		float							m_fMotionDuration;

		BOOL							m_isAccumulationMotion;
		TPixelPosition					m_accumulationPosition;

		BOOL							m_isComboMotion;
		TComboInputData					m_ComboInputData;

		BOOL							m_isLoopMotion;
		float							m_fLoopStartTime;
		float							m_fLoopEndTime;

		BOOL							m_isAttackingMotion;
		NRaceData::TMotionAttackData	m_MotionAttackData;

		BOOL							m_bCancelEnableSkill;

		TMotionEventDataVector			m_MotionEventDataVector;
		NSound::TSoundInstanceVector	m_SoundInstanceVector;

	private:
		BOOL							m_hasSplashEvent;

	protected:
		static CDynamicPool<CRaceMotionData> ms_kPool;
};
