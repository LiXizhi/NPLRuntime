//-----------------------------------------------------------------------------
// Class:	CAnimTable
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.11.16
// Revised: 2005.11.16
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "AnimTable.h"

using namespace ParaEngine;

/** mapping from ID to ID 
* TODO: this mapping should come from a database in future
*/
const int AnimDefaultPairs_[][2]=
{
	{ANIM_STAND, -1},
	
	{ANIM_RUN, ANIM_WALK},
	{ANIM_RUNLEFT, ANIM_RUN},
	{ANIM_RUNRIGHT, ANIM_RUN},
	{ANIM_WALKBACKWARDS, ANIM_RUN},

	{ANIM_SWIM, ANIM_RUN},
	{ANIM_SWIMLEFT, ANIM_SWIM},
	{ANIM_SWIMRIGHT, ANIM_SWIM},
	{ANIM_SWIMBACKWARDS, ANIM_SWIM},
	{ANIM_SWIMIDLE, ANIM_STAND},
	
	{ANIM_JUMP, ANIM_STAND},
	{ANIM_JUMPEND, ANIM_STAND},
	{ANIM_JUMPSTART, ANIM_STAND},

	{ANIM_ATTACKUNARMED, ANIM_STAND},
	{ANIM_ATTACK1H, ANIM_ATTACKUNARMED},
	{ANIM_ATTACK2H, ANIM_ATTACKUNARMED},
	{ANIM_ATTACK2HL, ANIM_ATTACKUNARMED},
	{ANIM_SPELL, ANIM_STAND}
};

/** name, ID pairs */
struct NameIDPair
{
	std::string sName;
	int ID;
public:
	NameIDPair(const char* name, int id):sName(name), ID(id){};
};

/** mapping from animation name to ID 
* TODO: this mapping should come from a database in future
*/
const NameIDPair AnimName_[]=
{
/* these keys are searched using hand-code, which will be slightly faster.
	NameIDPair("", ANIM_STAND),

	NameIDPair("f", ANIM_RUN),// walk
	NameIDPair("l", ANIM_RUNLEFT),
	NameIDPair("r", ANIM_RUNRIGHT),
	NameIDPair("b", ANIM_WALKBACKWARDS),

	NameIDPair("s", ANIM_SWIMIDLE),	// swim
	NameIDPair("sf", ANIM_SWIM),
	NameIDPair("sl", ANIM_SWIMLEFT),
	NameIDPair("sr", ANIM_SWIMRIGHT),
	NameIDPair("sb", ANIM_SWIMBACKWARDS),

	NameIDPair("j", ANIM_JUMP),	// jump
	NameIDPair("je", ANIM_JUMPEND),
	NameIDPair("js", ANIM_JUMPSTART),

	NameIDPair("a", ANIM_ATTACKUNARMED),// attack
	NameIDPair("a1", ANIM_ATTACK1H),
	NameIDPair("a2", ANIM_ATTACK2HL),

	NameIDPair("e_d", ANIM_EMOTEDANCE), // emotions
*/
	NameIDPair("Stand",0),
	NameIDPair("Death",1),
	NameIDPair("Spell",2),
	NameIDPair("Stop",3),
	NameIDPair("Walk",4),
	NameIDPair("Run",5),
	NameIDPair("Dead",6),
	NameIDPair("Rise",7),
	NameIDPair("StandWound",8),
	NameIDPair("CombatWound",9),
	NameIDPair("CombatCritical",10),
	NameIDPair("ShuffleLeft",11),
	NameIDPair("ShuffleRight",12),
	NameIDPair("Walkbackwards",13),
	NameIDPair("Stun",14),
	NameIDPair("HandsClosed",15),
	NameIDPair("AttackUnarmed",16),
	NameIDPair("Attack1H",17),
	NameIDPair("Attack2H",18),
	NameIDPair("Attack2HL",19),
	NameIDPair("ParryUnarmed",20),
	NameIDPair("Parry1H",21),
	NameIDPair("Parry2H",22),
	NameIDPair("Parry2HL",23),
	NameIDPair("ShieldBlock",24),
	NameIDPair("ReadyUnarmed",25),
	NameIDPair("Ready1H",26),
	NameIDPair("Ready2H",27),
	NameIDPair("Ready2HL",28),
	NameIDPair("ReadyBow",29),
	NameIDPair("Dodge",30),
	NameIDPair("SpellPrecast",31),
	NameIDPair("SpellCast",32),
	NameIDPair("SpellCastArea",33),
	NameIDPair("NPCWelcome",34),
	NameIDPair("NPCGoodbye",35),
	NameIDPair("Block",36),
	NameIDPair("JumpStart",37),
	NameIDPair("Jump",38),
	NameIDPair("JumpEnd",39),
	NameIDPair("Fall",40),
	NameIDPair("SwimIdle",41),
	NameIDPair("Swim",42),
	NameIDPair("SwimLeft",43),
	NameIDPair("SwimRight",44),
	NameIDPair("SwimBackwards",45),
	NameIDPair("AttackBow",46),
	NameIDPair("FireBow",47),
	NameIDPair("ReadyRifle",48),
	NameIDPair("AttackRifle",49),
	NameIDPair("Loot",50),
	NameIDPair("ReadySpellDirected",51),
	NameIDPair("ReadySpellOmni",52),
	NameIDPair("SpellCastDirected",53),
	NameIDPair("SpellCastOmni",54),
	NameIDPair("BattleRoar",55),
	NameIDPair("ReadyAbility",56),
	NameIDPair("Special1H",57),
	NameIDPair("Special2H",58),
	NameIDPair("ShieldBash",59),
	NameIDPair("EmoteTalk",60),
	NameIDPair("EmoteEat",61),
	NameIDPair("EmoteWork",62),
	NameIDPair("EmoteUseStanding",63),
	NameIDPair("EmoteTalkExclamation",64),
	NameIDPair("EmoteTalkQuestion",65),
	NameIDPair("EmoteBow",66),
	NameIDPair("EmoteWave",67),
	NameIDPair("EmoteCheer",68),
	NameIDPair("EmoteDance",69),
	NameIDPair("EmoteLaugh",70),
	NameIDPair("EmoteSleep",71),
	NameIDPair("EmoteSitGround",72),
	NameIDPair("EmoteRude",73),
	NameIDPair("EmoteRoar",74),
	NameIDPair("EmoteKneel",75),
	NameIDPair("EmoteKiss",76),
	NameIDPair("EmoteCry",77),
	NameIDPair("EmoteChicken",78),
	NameIDPair("EmoteBeg",79),
	NameIDPair("EmoteApplaud",80),
	NameIDPair("EmoteShout",81),
	NameIDPair("EmoteFlex",82),
	NameIDPair("EmoteShy",83),
	NameIDPair("EmotePoint",84),
	NameIDPair("Attack1HPierce",85),
	NameIDPair("Attack2HLoosePierce",86),
	NameIDPair("AttackOff",87),
	NameIDPair("AttackOffPierce",88),
	NameIDPair("Sheath",89),
	NameIDPair("HipSheath",90),
	NameIDPair("Mount",91),
	NameIDPair("RunRight",92),
	NameIDPair("RunLeft",93),
	NameIDPair("MountSpecial",94),
	NameIDPair("Kick",95),
	NameIDPair("SitGroundDown",96),
	NameIDPair("SitGround",97),
	NameIDPair("SitGroundUp",98),
	NameIDPair("SleepDown",99),
	NameIDPair("Sleep",100),
	NameIDPair("SleepUp",101),
	NameIDPair("SitChairLow",102),
	NameIDPair("SitChairMed",103),
	NameIDPair("SitChairHigh",104),
	NameIDPair("LoadBow",105),
	NameIDPair("LoadRifle",106),
	NameIDPair("AttackThrown",107),
	NameIDPair("ReadyThrown",108),
	NameIDPair("HoldBow",109),
	NameIDPair("HoldRifle",110),
	NameIDPair("HoldThrown",111),
	NameIDPair("LoadThrown",112),
	NameIDPair("EmoteSalute",113),
	NameIDPair("KneelStart",114),
	NameIDPair("KneelLoop",115),
	NameIDPair("KneelEnd",116),
	NameIDPair("AttackUnarmedOff",117),
	NameIDPair("SpecialUnarmed",118),
	NameIDPair("StealthWalk",119),
	NameIDPair("StealthStand",120),
	NameIDPair("Knockdown",121),
	NameIDPair("EatingLoop",122),
	NameIDPair("UseStandingLoop",123),
	NameIDPair("ChannelCastDirected",124),
	NameIDPair("ChannelCastOmni",125),
	NameIDPair("Whirlwind",126),
	NameIDPair("Birth",127),
	NameIDPair("UseStandingStart",128),
	NameIDPair("UseStandingEnd",129),
	NameIDPair("CreatureSpecial",130),
	NameIDPair("Drown",131),
	NameIDPair("Drowned",132),
	NameIDPair("FishingCast",133),
	NameIDPair("FishingLoop",134),
	NameIDPair("Fly",135),
	NameIDPair("EmoteWorkNoSheathe",136),
	NameIDPair("EmoteStunNoSheathe",137),
	NameIDPair("EmoteUseStandingNoSheathe",138),
	NameIDPair("SpellSleepDown",139),
	NameIDPair("SpellKneelStart",140),
	NameIDPair("SpellKneelLoop",141),
	NameIDPair("SpellKneelEnd",142),
	NameIDPair("Sprint",143),
	NameIDPair("InFlight",144),
	NameIDPair("Spawn",145),
	NameIDPair("Close",146),
	NameIDPair("Closed",147),
	NameIDPair("Open",148),
	NameIDPair("Opened",149),
	NameIDPair("Destroy",150),
	NameIDPair("Destroyed",151),
	NameIDPair("Rebuild",152),
	NameIDPair("Custom0",153),
	NameIDPair("Custom1",154),
	NameIDPair("Custom2",155),
	NameIDPair("Custom3",156),
	NameIDPair("Despawn",157),
	NameIDPair("Hold",158),
	NameIDPair("Decay",159),
	NameIDPair("BowPull",160),
	NameIDPair("BowRelease",161),
	NameIDPair("ShipStart",162),
	NameIDPair("ShipMoving",163),
	NameIDPair("ShipStop",164),
	NameIDPair("GroupArrow",165),
	NameIDPair("Arrow",166),
	NameIDPair("CorpseArrow",167),
	NameIDPair("GuideArrow",168),
	NameIDPair("Sway",169),
	NameIDPair("DruidCatPounce",170),
	NameIDPair("DruidCatRip",171),
	NameIDPair("DruidCatRake",172),
	NameIDPair("DruidCatRavage",173),
	NameIDPair("DruidCatClaw",174),
	NameIDPair("DruidCatCower",175),
	NameIDPair("DruidBearSwipe",176),
	NameIDPair("DruidBearBite",177),
	NameIDPair("DruidBearMaul",178),
	NameIDPair("DruidBearBash",179),
	NameIDPair("DragonTail",180),
	NameIDPair("DragonStomp",181),
	NameIDPair("DragonSpit",182),
	NameIDPair("DragonSpitHover",183),
	NameIDPair("DragonSpitFly",184),
	NameIDPair("EmoteYes",185),
	NameIDPair("EmoteNo",186),
	NameIDPair("JumpLandRun",187),
	NameIDPair("LootHold",188),
	NameIDPair("LootUp",189),
	NameIDPair("StandHigh",190),
	NameIDPair("Impact",191),
	NameIDPair("LiftOff",192),
	NameIDPair("Hover",193),
	NameIDPair("SuccubusEntice",194),
	NameIDPair("EmoteTrain",195),
	NameIDPair("EmoteDead",196),
	NameIDPair("EmoteDanceOnce",197),
	NameIDPair("Deflect",198),
	NameIDPair("EmoteEatNoSheathe",199),
	NameIDPair("Land",200),
	NameIDPair("Submerge",201),
	NameIDPair("Submerged",202),
	NameIDPair("Cannibalize",203),
	NameIDPair("ArrowBirth",204),
	NameIDPair("GroupArrowBirth",205),
	NameIDPair("CorpseArrowBirth",206),
	NameIDPair("GuideArrowBirth",207)
};

CAnimTable::CAnimTable(void)
{
	int nNum = sizeof(AnimDefaultPairs_)/(sizeof(int)*2);
	for(int i=0; i<nNum;i++)
	{
		m_AnimDefaultMaps[AnimDefaultPairs_[i][0]]=AnimDefaultPairs_[i][1];
	}

	nNum = sizeof(AnimName_)/(sizeof(NameIDPair));
	for(int i=0; i<nNum;i++)
	{
		m_AnimNameMaps[AnimName_[i].sName]=AnimName_[i].ID;
	}
}

CAnimTable::~CAnimTable(void)
{
}
CAnimTable* CAnimTable::GetInstance()
{
	static CAnimTable singleton_;
	return &singleton_;
}
int CAnimTable::GetDefaultAnimIDof(int nID)
{
	std::map<int,int>::iterator iter  = GetInstance()->m_AnimDefaultMaps.find(nID);
	if(iter!=GetInstance()->m_AnimDefaultMaps.end())
	{
		return iter->second;
	}
	return 0;	
}

int CAnimTable::GetAnimIDByName(const char* sName)
{
	// TODO: this should be read from the animDB table.
	// here is an ad hoc mapping.
	if(sName == NULL)
		return ANIM_STAND;
	int nLen = (int)strlen(sName);
	if(nLen == 0) 
		return ANIM_STAND;
	if(nLen <= 2)
	{
		char cmd = sName[0];
		if(cmd == 'f')
			return ANIM_RUN;
		else if(cmd == 'F')
			return ANIM_WALK;
		else if(cmd == 'l')
			return ANIM_RUNLEFT;
		else if(cmd == 'r')
			return ANIM_RUNRIGHT;
		else if(cmd == 'b' || cmd == 'B')
			return ANIM_WALKBACKWARDS;
		else if(cmd == 's')
		{
			if(nLen == 2)
			{
				cmd = sName[1];
				if(cmd == 'f')
					return ANIM_SWIM; // "sf"
				else if(cmd == 'l')
					return ANIM_SWIMRIGHT;// "sl"
				else if(cmd == 'r')
					return ANIM_SWIMLEFT;// "sr"
				else if(cmd == 'b')
					return ANIM_SWIMBACKWARDS;// "sb"
			}
			else
				return ANIM_SWIMIDLE;// "s"
		}
		else if(cmd == 'j')
		{
			if(nLen == 2)
			{
				cmd = sName[1];
				if(cmd == 's')
					return ANIM_JUMPSTART;// "js"
				else if(cmd == 'e')
					return ANIM_JUMPEND;// "je"
			}
			else
				return ANIM_JUMP;// "j"
		}
		else if(cmd == 'm')
		{
			return ANIM_MOUNT;// "m"
		}
		else if(cmd == 'a')
		{
			if(nLen == 2)
			{
				cmd = sName[1];
				if(cmd == '1')
					return ANIM_ATTACK1H;// "a1"
				else if(cmd == '2')
					return ANIM_ATTACK2HL;// "a2"
			}
			else
				return ANIM_ATTACKUNARMED;// "a"
		}
	}
	else if(sName[0] =='e' && sName[1] =='_')
	{
		// for all kinds of emotions.
		char cmd = sName[2];
		if(cmd=='d')
			return ANIM_EMOTEDANCE; // "e_d"
	}
	else
	{
		/// for other long keys, search in the map.
		std::map<std::string,int>::iterator iter  = GetInstance()->m_AnimNameMaps.find(std::string(sName));
		if(iter!=GetInstance()->m_AnimNameMaps.end())
		{
			return iter->second;
		}
	}
	return 0;
}

bool ParaEngine::CAnimTable::IsWalkAnimation(int nNextAnimID)
{
	if (nNextAnimID < 1000)
	{
		// load local model animation
		while (nNextAnimID != ANIM_WALK && nNextAnimID != 0){
			nNextAnimID = CAnimTable::GetDefaultAnimIDof(nNextAnimID);
		};
		return nNextAnimID == ANIM_WALK;
	}
	return false;
}
