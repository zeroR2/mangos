/*
 * Copyright (C) 2005-2012 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#ifndef WORLD_PVP_WG
#define WORLD_PVP_WG

#include "../Player.h"
#include "../Creature.h"
#include "../GameObject.h"
#include "../ObjectMgr.h"

struct WorldPvPWGGameObjectBuilding;
typedef std::set<WorldPvPWGGameObjectBuilding*> GameObjectBuilding;

struct WorldPvPWGWorkShopData;
typedef std::set<WorldPvPWGWorkShopData*> WorkShop;

struct WorldPvPGraveYardWG;
typedef std::set<WorldPvPGraveYardWG*> GraveYard;

enum
{
    MAPID_ID_WINTERGRASP                = 571,
    // GraveYard
    GRAVEYARD_ID_ALLIANCE               = 1332,
    GRAVEYARD_ID_HORDE                  = 1331,
    GRAVEYARD_ID_KEEP                   = 1285,
    // Go factions
    GO_FACTION_A                        = 1732,
    GO_FACTION_H                        = 1735,
    // Npc factions
    NPC_FACTION_A                       = 1891,
    NPC_FACTION_H                       = 1979,
    // World States
    WS_BATTLE_ACTIVE_POS                = 3710,
    WS_BATTLE_ACTIVE_NEG                = 3801,
    WS_CLOCK_1                          = 3781,
    WS_CLOCK_2                          = 4354,
    WS_VEHICLE_COUNT_H                  = 3490,
    WS_VEHICLE_COUNT_MAX_H              = 3491,
    WS_VEHICLE_COUNT_A                  = 3680,
    WS_VEHICLE_COUNT_MAX_A              = 3681,
    WS_DEFENDER_TEAM                    = 3802,
    WS_ATTACKER_TEAM                    = 3803,
};

enum GameObjectArtKits
{
    GO_ARTKIT_BANNER_ALLIANCE               = 2,
    GO_ARTKIT_BANNER_HORDE                  = 1,
    GO_ARTKIT_BANNER_NEUTRAL                = 21,
};

enum GameObjectId
{
    GO_TITAN_RELIC                              = 192829,
    GO_TELEPORT                                 = 190763,
    GO_VEHICLE_TELEPORT                         = 192951,
    GO_FACTORY_BANNER_NE                        = 190475,
    GO_FACTORY_BANNER_NW                        = 190487,
    GO_FACTORY_BANNER_SE                        = 194959,
    GO_FACTORY_BANNER_SW                        = 194962,
    GO_BUILDING_NE                              = 192031,
    GO_BUILDING_NW                              = 192030,
    GO_BUILDING_SE                              = 192033,
    GO_BUILDING_SW                              = 192032,
    GO_BUILDING_KEEP_WEST                       = 192028,
    GO_BUILDING_KEEP_EAST                       = 192029,
};

enum eWGEvents
{
    EVENT_FACTORY_NE_PROGRESS_ALLIANCE = 19610,
    EVENT_FACTORY_NE_PROGRESS_HORDE = 19609,
    EVENT_FACTORY_NW_PROGRESS_ALLIANCE = 19612,
    EVENT_FACTORY_NW_PROGRESS_HORDE = 19611,
    EVENT_FACTORY_SE_PROGRESS_ALLIANCE = 21565,
    EVENT_FACTORY_SE_PROGRESS_HORDE = 21563,
    EVENT_FACTORY_SW_PROGRESS_ALLIANCE = 21562,
    EVENT_FACTORY_SW_PROGRESS_HORDE = 21560,
};

enum eWGSpell
{
    SPELL_TELEPORT_DALARAN                       = 53360,
    SPELL_TOWER_CONTROL                          = 62064,
    SPELL_RECRUIT                                = 37795,
    SPELL_CORPORAL                               = 33280,
    SPELL_LIEUTENANT                             = 55629,
    SPELL_SPIRITUAL_IMMUNITY                     = 58729,
    SPELL_TENACITY                               = 58549,
    SPELL_TENACITY_VEHICLE                       = 59911,
    SPELL_VEHICLE_TELEPORT                       = 49759,
};

enum eWGAchievements
{
    ACHIEVEMENTS_WIN_WG                          = 1717,
    ACHIEVEMENTS_WIN_WG_100                      = 1718,
    ACHIEVEMENTS_WG_TOWER_DESTROY                = 1727,
};

enum eWGGameObjectBuildingType
{
    WORLD_PVP_WG_OBJECTTYPE_DOOR,
    WORLD_PVP_WG_OBJECTTYPE_TITANRELIC,
    WORLD_PVP_WG_OBJECTTYPE_WALL,
    WORLD_PVP_WG_OBJECTTYPE_DOOR_LAST,
    WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER,
    WORLD_PVP_WG_OBJECTTYPE_TOWER,
};

enum eWGGameObjectState
{
    WORLD_PVP_WG_OBJECTSTATE_NONE,
    WORLD_PVP_WG_OBJECTSTATE_NEUTRAL_INTACT,
    WORLD_PVP_WG_OBJECTSTATE_NEUTRAL_DAMAGE,
    WORLD_PVP_WG_OBJECTSTATE_NEUTRAL_DESTROY,
    WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT,
    WORLD_PVP_WG_OBJECTSTATE_HORDE_DAMAGE,
    WORLD_PVP_WG_OBJECTSTATE_HORDE_DESTROY,
    WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT,
    WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_DAMAGE,
    WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_DESTROY,
};

enum eWGWorkShopType
{
    WORLD_PVP_WG_WORKSHOP_NE,
    WORLD_PVP_WG_WORKSHOP_NW,
    WORLD_PVP_WG_WORKSHOP_SE,
    WORLD_PVP_WG_WORKSHOP_SW,
    WORLD_PVP_WG_WORKSHOP_KEEP_WEST,
    WORLD_PVP_WG_WORKSHOP_KEEP_EAST,
};

enum eWGNpc
{
    NPC_DEMOLISHER_ENGINEER_A        = 30499,
    NPC_DEMOLISHER_ENGINEER_H        = 30400,
    NPC_SPIRIT_GUIDE_A               = 31842,
    NPC_SPIRIT_GUIDE_H               = 31841,
    NPC_TURRER                       = 28366,
    NPC_GUARD_H                      = 30739,
    NPC_GUARD_A                      = 30740,
    NPC_VIERON_BLAZEFEATHER          = 31102,
    NPC_BOWYER_RANDOLPH              = 31052,
    NPC_HOODOO_MASTER_FU_JIN         = 31101,
    NPC_SORCERESS_KAYLANA            = 31051,
    NPC_CHAMPION_ROS_SLAI            = 39173,
    NPC_MARSHAL_MAGRUDER             = 39172,
    NPC_COMMANDER_DARDOSH            = 31091,
    NPC_COMMANDER_ZANNETH            = 31036,
    NPC_TACTICAL_OFFICER_KILRATH     = 31151,
    NPC_TACTICAL_OFFICER_AHBRAMIS    = 31153,
    NPC_SIEGESMITH_STRONGHOOF        = 31106,
    NPC_SIEGE_MASTER_STOUTHANDLE     = 31108,
    NPC_PRIMALIST_MULFORT            = 31053,
    NPC_ANCHORITE_TESSA              = 31054,
    NPC_LIEUTENANT_MURP              = 31107,
    NPC_SENIOR_DEMOLITIONIST_LEGOSO  = 31109,
    //Vehicles
    NPC_CATAPULT                     = 27881,
    NPC_DEMOLISHER                   = 28094,
    NPC_SIEGE_ENGINE_A               = 28312,
    NPC_SIEGE_ENGINE_H               = 32627,
};

struct WorldPvPWGBuildingSpawnData
{
    uint32 entry;
    uint32 WorldState;
    uint32 type;
    uint32 textid;
};

#define WG_MAX_OBJ 32
const WorldPvPWGBuildingSpawnData WGGameObjectBuillding[WG_MAX_OBJ] = {
    // Wall
    // Entry WS       type                        NameID
    { 190219, 3749, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190220, 3750, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191795, 3764, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191796, 3772, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191799, 3762, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191800, 3766, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191801, 3770, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191802, 3751, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191803, 3752, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191804, 3767, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191806, 3769, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191807, 3759, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191808, 3760, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191809, 3761, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190369, 3753, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190370, 3758, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190371, 3754, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190372, 3757, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190374, 3755, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190376, 3756, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    // Tower of keep
    { 190221, 3711, WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER, 0 },
    { 190373, 3713, WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER, 0 },
    { 190377, 3714, WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER, 0 },
    { 190378, 3712, WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER, 0 },
    // Wall (with passage)
    { 191797, 3765, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191798, 3771, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191805, 3768, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    // South tower
    { 190356, 3704, WORLD_PVP_WG_OBJECTTYPE_TOWER, 0 },
    { 190357, 3705, WORLD_PVP_WG_OBJECTTYPE_TOWER, 0 },
    { 190358, 3706, WORLD_PVP_WG_OBJECTTYPE_TOWER, 0 },
    // Door of forteress
    { 190375, 3763, WORLD_PVP_WG_OBJECTTYPE_DOOR, 0 },
    // Last door
    { 191810, 3773, WORLD_PVP_WG_OBJECTTYPE_DOOR_LAST, 0 },
};

struct WorldPvPWGWorkShopDataBase
{
    uint32 entry;
    uint32 worldstate;
    uint32 type;
    uint32 textid;
    uint32 GraveYardId;
};

#define WG_MAX_WORKSHOP  6

const WorldPvPWGWorkShopDataBase WGWorkShopDataBase[WG_MAX_WORKSHOP] = {
    { 192031,3701,WORLD_PVP_WG_WORKSHOP_NE,0,1329 },
    { 192030,3700,WORLD_PVP_WG_WORKSHOP_NW,0,1330 },
    { 192033,3703,WORLD_PVP_WG_WORKSHOP_SE,0,1333 },
    { 192032,3702,WORLD_PVP_WG_WORKSHOP_SW,0,1334   },
    { 192028,3698,WORLD_PVP_WG_WORKSHOP_KEEP_WEST,0,0 },
    { 192029,3699,WORLD_PVP_WG_WORKSHOP_KEEP_EAST,0,0 }
};

struct VehicleSummonDataWG
{
    uint32 guida;
    uint32 guidh;
    float x;
    float y;
    float z;
    float o;
};

const VehicleSummonDataWG CoordVehicleSummon[6] = {
    { 530040, 530041, 5391.12f , 2986.23f , 413.298f , 3.18008f },
    { 530042, 530043, 5389.81f , 2719.13f , 413.133f , 3.14081f },
    { 530044, 530045, 4950.19f , 2389.96f , 324.269f , 1.41764f },
    { 530046, 530047, 4954.49f , 3385.17f , 380.993f , 4.35896f },
    { 530048, 530049, 4359.86f , 2347.38f , 380.103f , 6.12532f },
    { 530050, 530051, 4359.86f , 2347.38f , 380.103f , 6.12532f },
};

class MANGOS_DLL_SPEC OutdoorPvPWG : public OutdoorPvP
{
    public:
        OutdoorPvPWG();

        bool InitWorldPvPArea();
        bool InitBattlefield();

        void HandleCreatureCreate(Creature* pCreature);
        void HandleGameObjectCreate(GameObject* pGo);
        void HandleCreatureDeath(Creature* pCreature);

        virtual void Update(uint32 uiDiff);

        void HandlePlayerEnterZone(Player* pPlayer, bool isMainZone);
        void HandlePlayerLeaveZone(Player* pPlayer, bool isMainZone);
        void HandlePlayerKillInsideArea(Player* pPlayer, Unit* pVictim);

        bool HandleGameObjectUse(Player* pPlayer, GameObject* pGo);
        bool HandleEvent(uint32 uiEventId,GameObject* pGo);
        void EventPlayerDamageGO(Player *player, GameObject* target_obj, uint32 eventId, uint32 spellId);

        uint32 GetDefender()  { return defender; }
        uint32 GetAttacker()  { return attacker; }
        bool m_bIsWarTime() { return m_bIsBattleStarted; }
        uint32 GetTimer() const { return m_Timer; };

        void SetBannerArtKit(GameObject* Go,uint32 uiArtkit);
        void UpdateCounterVehicle(bool init);

        ObjectGuid gBanner[4];
        ObjectGuid gBuilding[6];
        ObjectGuid gBandT[32];

        bool CanUseRelic;
        uint8 GetCountBuildingAndTowers(uint32 id);

        //World states
        void UpdateVehicleCountWG();
        void UpdateWSBuilding();
        void UpdateWSWorkShop();

        //Auras
        void UpdateAura(Player* pPlayer);
        void UpdateTenacityStack();
        void AddAuraResurrect(Player* pPlayer);

        //Towers
        void AddDamagedTower(uint32 team);
        void AddBrokenTower(uint32 team, Player* pPlayer);
        void BrokenWallOrTower(uint32 team,Player* pPlayer);

        //Others
        void DoCompleteOrIncrementAchievement(uint32 achievement, Player *player);
        void CompleOneObjetiveQuest(Player* pPlayer,uint32 id);
        void AddDataWhenWin();
        void RecolatePlayers();

        //Vehicles
        void CreateVehicle(Creature* pCreature,uint32 npc_entry);
        void AddVehicle(Creature* pCreature,uint32 team);
        void DeleteVehicle(Creature* pCreature,uint32 team);

		//Players
		Player* GetPlayerInZone();
		Player* GetPlayersAlliance();
		Player* GetPlayersHorde();
		uint32 CountPlayersAlliance();
		uint32 CountPlayersHorde();

    protected:
        uint32 m_Timer; // In second

    private:
        bool activate;
        uint32 defender;
        uint32 attacker;
        bool m_bIsBattleStarted;

        void Install();
        void UpdateTeleport(GameObject* pGo);
        void UpdateKeepTurret(Creature* pTurret);
        void PrepareKeepNpc(Creature* pCreature,uint32 team);
        void PrepareKeepGo(GameObject* pGo,uint32 team);
        void PrepareOutKeepNpc(Creature* pCreature,uint32 team);
        bool get_map;
        Map* m_Map;
        void NewRound(bool titan); //if titan = true the attackers win

        uint32 m_tenacityStack;
        uint32 VehicleCountA;
        uint32 VehicleCountH;
        uint32 VehicleCountMaxA;
        uint32 VehicleCountMaxH;

        GuidSet m_sZonePlayers;
        GuidSet m_sZonePlayersAlliance;
        GuidSet m_sZonePlayersHorde;

        //Timers
        uint32 TimeBattle;
        uint32 UpdateTimer;

        //OjectGuid GameObjects and Creatures
        ObjectGuid Relic;
        ObjectGuid VehcileTeleport1;
        ObjectGuid VehcileTeleport2;
        std::list<ObjectGuid> KeepCreatureA;
        std::list<ObjectGuid> KeepCreatureH;
        std::list<ObjectGuid> m_KeepGameObjectA;
        std::list<ObjectGuid> m_KeepGameObjectH;
        std::list<ObjectGuid> OutKeepCreatureA;
        std::list<ObjectGuid> OutKeepCreatureH;
        std::list<ObjectGuid> TeleportGameObject;
        std::list<ObjectGuid> TurretCreature;
        std::list<ObjectGuid> m_vehicleA;
        std::list<ObjectGuid> m_vehicleH;


        //Graveyard
        WorldPvPGraveYardWG* gyNe;
        WorldPvPGraveYardWG* gyNw;
        WorldPvPGraveYardWG* gySe;
        WorldPvPGraveYardWG* gySw;
        WorldPvPGraveYardWG* gyBuilding;
        WorldPvPGraveYardWG* gyAlliance;
        WorldPvPGraveYardWG* gyHorde;

        //Keep and towers
        GameObjectBuilding BuildingsInZone;
        WorldPvPWGGameObjectBuilding* Building[32];
        uint32 DamageTowerAtt;
        uint32 DamageTowerDef;
        uint32 DestroyTowerAtt;
        uint32 DestroyTowerDef;

        //WorkShop
        std::list<WorldPvPWGWorkShopData*>WorkShopList;
        WorldPvPWGWorkShopData* wsNe;
        WorldPvPWGWorkShopData* wsNw;
        WorldPvPWGWorkShopData* wsSe;
        WorldPvPWGWorkShopData* wsSw;
        WorldPvPWGWorkShopData* wsKe;
        WorldPvPWGWorkShopData* wsKw;
};

struct WorldPvPWGGameObjectBuilding
{

    uint32 m_Team;
    OutdoorPvPWG *m_WG;
    uint8 count;
    uint32 m_Type;
    uint32 m_WorldState;
    uint32 m_State;
    uint32 m_TextId;
    uint32 TowerCount;
    Map* Mmap;
    bool map;

    std::list<ObjectGuid> TurretCreature;
    std::list<ObjectGuid> m_GameObjectA;
    std::list<ObjectGuid> m_GameObjectH;
    std::list<ObjectGuid> m_CreatureA;
    std::list<ObjectGuid> m_CreatureH;

    WorldPvPWGGameObjectBuilding(OutdoorPvPWG *WG)
    {
        m_WG = WG;
        m_Team = 0;
        count = 0;
        TowerCount = 0;
        m_Type = 0;
        m_WorldState = 0;
        m_State = 0;
        m_TextId = 0;
        Mmap = 0;
        map = false;
    }

    void Damaged()
    {

        if(m_Team == ALLIANCE)
            m_State = WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_DAMAGE;
        else if(m_Team == HORDE)
            m_State = WORLD_PVP_WG_OBJECTSTATE_HORDE_DAMAGE;

        m_WG->SendUpdateWorldState(m_WorldState, m_State);

        if (m_Type == WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER)
            m_WG->AddDamagedTower(m_WG->GetDefender());
        else if (m_Type == WORLD_PVP_WG_OBJECTTYPE_TOWER)
            m_WG->AddDamagedTower(m_WG->GetAttacker());
    }

    void Destroyed(Player* pPlayer)
    {
        if(m_Team == ALLIANCE)
            m_State = WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_DESTROY;
        else if(m_Team == HORDE)
            m_State = WORLD_PVP_WG_OBJECTSTATE_HORDE_DESTROY;

        m_WG->SendUpdateWorldState(m_WorldState, m_State);

        switch (m_Type)
        {
            case WORLD_PVP_WG_OBJECTTYPE_TOWER:
            case WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER:
                m_WG->AddBrokenTower(m_Team,pPlayer);
                break;
            case WORLD_PVP_WG_OBJECTTYPE_DOOR_LAST:
                m_WG->CanUseRelic = true;
                break;
        }

        m_WG->BrokenWallOrTower(m_Team,pPlayer);
    }

    void Rebuild()
    {
      if(m_Team == ALLIANCE)
      {
            if(m_State != WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT)
                m_State = WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT;
      }
      else
      {
            if(m_State != WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT)
               m_State = WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT;
      }

      m_WG->SendUpdateWorldState(m_WorldState, m_State);

      if(map)
        RebuildGo();

        if(TowerCount > 0)
        {
            if(m_Team == ALLIANCE)
            {
               for (std::list<ObjectGuid>::iterator itr = m_CreatureA.begin(); itr != m_CreatureA.end(); ++itr)
               {
                  if(Creature* pCreature = Mmap->GetCreature((*itr)))
                     pCreature->Respawn();
               }
            }
            else if(m_Team == HORDE)
            {
               for (std::list<ObjectGuid>::iterator itr = m_CreatureH.begin(); itr != m_CreatureH.end(); ++itr)
               {
                  if(Creature* pCreature = Mmap->GetCreature((*itr)))
                     pCreature->Respawn();
               }
            }


           for (std::list<ObjectGuid>::iterator itr = TurretCreature.begin(); itr != TurretCreature.end(); ++itr)
           {
               if(Creature* pTurret = Mmap->GetCreature((*itr)))
                   pTurret->Respawn();
           }
        }
    }

    void ChangeTeam(uint32 team)
    {
        if(team != m_Team)
        {
            m_Team = team;

            if(team == ALLIANCE)
            {
              if(m_State != WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT)
                 m_State = WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT;
            }
            else if(team == HORDE)
            {
              if(m_State != WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT)
                  m_State = WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT;
            }

            if(map)
               Rebuild();

            if(TowerCount > 0)
            {
                if(team == ALLIANCE)
                {
                   for (std::list<ObjectGuid>::iterator itr = TurretCreature.begin(); itr != TurretCreature.end(); ++itr)
                   {
                     if(Creature* pCreature = Mmap->GetCreature((*itr)))
                         pCreature->setFaction(NPC_FACTION_A);
                   }

                   for (std::list<ObjectGuid>::iterator itr = m_CreatureH.begin(); itr != m_CreatureH.end(); ++itr)
                   {
                     if(Creature* pCreature = Mmap->GetCreature((*itr)))
                     {
                        pCreature->setFaction(35);
                        pCreature->SetVisibility(VISIBILITY_OFF);
                     }
                   }

                   for (std::list<ObjectGuid>::iterator itr = m_CreatureA.begin(); itr != m_CreatureA.end(); ++itr)
                   {
                     if(Creature* pCreature = Mmap->GetCreature((*itr)))
                     {
                        pCreature->Respawn();
                        pCreature->setFaction(NPC_FACTION_A);
                        pCreature->SetVisibility(VISIBILITY_ON);
                     }
                   }

                   for (std::list<ObjectGuid>::iterator itr = m_GameObjectA.begin(); itr != m_GameObjectA.end(); ++itr)
                   {
                       if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                          pGo->SetPhaseMask(1,true);
                   }

                   for (std::list<ObjectGuid>::iterator itr = m_GameObjectH.begin(); itr != m_GameObjectH.end(); ++itr)
                   {
                       if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                          pGo->SetPhaseMask(100,true);
                   }
                }
                else if(team == HORDE)
                {
                   for (std::list<ObjectGuid>::iterator itr = TurretCreature.begin(); itr != TurretCreature.end(); ++itr)
                   {
                     if(Creature* pCreature = Mmap->GetCreature((*itr)))
                         pCreature->setFaction(NPC_FACTION_H);
                   }

                   for (std::list<ObjectGuid>::iterator itr = m_CreatureA.begin(); itr != m_CreatureA.end(); ++itr)
                   {
                     if(Creature* pCreature = Mmap->GetCreature((*itr)))
                     {
                        pCreature->setFaction(35);
                        pCreature->SetVisibility(VISIBILITY_OFF);
                     }
                   }

                   for (std::list<ObjectGuid>::iterator itr = m_CreatureH.begin(); itr != m_CreatureH.end(); ++itr)
                   {
                     if(Creature* pCreature = Mmap->GetCreature((*itr)))
                     {
                        pCreature->Respawn();
                        pCreature->setFaction(NPC_FACTION_H);
                        pCreature->SetVisibility(VISIBILITY_ON);
                     }
                   }

                   for (std::list<ObjectGuid>::iterator itr = m_GameObjectH.begin(); itr != m_GameObjectH.end(); ++itr)
                   {
                       if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                          pGo->SetPhaseMask(1,true);
                   }

                   for (std::list<ObjectGuid>::iterator itr = m_GameObjectA.begin(); itr != m_GameObjectA.end(); ++itr)
                   {
                       if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                          pGo->SetPhaseMask(100,true);
                   }
                }
            }
        }
    }

    void Init(uint32 i, uint32 type, uint32 worldstate, uint32 textid)
    {
        count = i;

        m_Type = type;

        m_WorldState = worldstate;

        m_TextId = textid;

        switch (m_Type)
        {
            case WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER:
            case WORLD_PVP_WG_OBJECTTYPE_DOOR_LAST:
            case WORLD_PVP_WG_OBJECTTYPE_DOOR:
            case WORLD_PVP_WG_OBJECTTYPE_WALL:
                m_Team = m_WG->GetDefender();
                break;
            case WORLD_PVP_WG_OBJECTTYPE_TOWER:
                m_Team = m_WG->GetAttacker();
                break;
            default:
                m_Team = TEAM_NONE;
                break;
        }

        switch(m_Team)
        {
           case ALLIANCE:
                m_State = WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT;
                break;
           case HORDE:
                m_State = WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT;
                break;

        }

        switch (count)
        {
            case 27:
                TowerCount = 1;
                break;
            case 28:
                TowerCount = 2;
                break;
            case 29:
                TowerCount = 3;
                break;
        }
    }

    void RebuildGo()
    {
        if(Mmap->GetGameObject(m_WG->gBandT[count]))
           Mmap->GetGameObject(m_WG->gBandT[count])->Rebuild(((Unit*)m_WG->GetPlayerInZone()));
    }

    void AddTurret(Creature* pTurret)
    {
        if(!map)
        {
            Mmap = pTurret->GetMap();
            map = true;
        }

        if(TowerCount > 0)
        {
            if(m_Team == ALLIANCE)
               pTurret->setFaction(NPC_FACTION_A);
            else if(m_Team == HORDE)
               pTurret->setFaction(NPC_FACTION_H);

            TurretCreature.push_back(pTurret->GetObjectGuid());
        }
    }

    void AddCreature(Creature* pCreature, uint32 team)
    {
        if(!map)
        {
            Mmap = pCreature->GetMap();
            map = true;
        }

        if(TowerCount > 0)
        {
            if(m_Team == team)
            {
                if(m_Team == ALLIANCE)
                   m_CreatureA.push_back(pCreature->GetObjectGuid());
                else if(m_Team == HORDE)
                   m_CreatureH.push_back(pCreature->GetObjectGuid());
            }
            else if(m_Team != team)
            {
                pCreature->setFaction(35);
                pCreature->SetVisibility(VISIBILITY_OFF);

                if(m_Team == ALLIANCE)
                   m_CreatureH.push_back(pCreature->GetObjectGuid());
                else if(m_Team == HORDE)
                   m_CreatureA.push_back(pCreature->GetObjectGuid());
            }
        }
    }

    void AddGameObject(GameObject* pGo,uint32 team)
    {
        if(!map)
        {
            Mmap = pGo->GetMap();
            map = true;
        }

        if(TowerCount > 0)
        {
            if(m_Team == team)
            {
                if(m_Team == ALLIANCE)
                   m_GameObjectA.push_back(pGo->GetObjectGuid());
                else if(m_Team == HORDE)
                   m_GameObjectH.push_back(pGo->GetObjectGuid());
            }
            else if(m_Team != team)
            {
                pGo->SetPhaseMask(100,true);

                if(m_Team == ALLIANCE)
                   m_GameObjectH.push_back(pGo->GetObjectGuid());
                else if(m_Team == HORDE)
                   m_GameObjectA.push_back(pGo->GetObjectGuid());
            }
        }
    }
};

struct WorldPvPGraveYardWG
{
    OutdoorPvPWG* m_WG;
    uint32 m_team;
    uint32 m_id;
    Map* Mmap;
    bool map;

    ObjectGuid SpiritGuieA;
    ObjectGuid SpiritGuieH;

    WorldPvPGraveYardWG(OutdoorPvPWG * WG)
    {
        m_WG = WG;
        m_team = 0;
        m_id = 0;
        Mmap = 0;
        map = false;
    }

    void Init(uint32 id,uint32 team)
    {
       m_id = id;
       m_team = team;
    }

    uint32 GetId()
    {
      return m_id;
    }

    void AddSpiritGuide(Creature* pCreature,uint32 team)
    {
        if (!pCreature)
            return;

        if(!map)
        {
            Mmap = pCreature->GetMap();
            map = true;
        }

       if(m_team == team)
       {
           if (m_team == ALLIANCE)
              SpiritGuieA = pCreature->GetObjectGuid();
           else if (m_team = HORDE)
              SpiritGuieH = pCreature->GetObjectGuid();
       }
       else if(m_team != team)
       {
           pCreature->SetVisibility(VISIBILITY_OFF);

           if(m_team == ALLIANCE)
              SpiritGuieH = pCreature->GetObjectGuid();
           else if(m_team = HORDE)
              SpiritGuieA = pCreature->GetObjectGuid();
       }
    }

    void ChangeTeam(uint32 team)
    {
       if(m_team != team)
       {
         m_team = team;

           if(team == ALLIANCE)
           {
              if(map && Mmap)
              {
                  if (Creature* c1 = Mmap->GetCreature(SpiritGuieH))
                      c1->SetVisibility(VISIBILITY_OFF);
                  if (Creature* c2 = Mmap->GetCreature(SpiritGuieA))
                      c2->SetVisibility(VISIBILITY_ON);
              }
           }
           else if(team = HORDE)
           {
              if(map && Mmap)
              {
                  if (Creature* c1 = Mmap->GetCreature(SpiritGuieA))
                      c1->SetVisibility(VISIBILITY_OFF);
                  if (Creature* c2 = Mmap->GetCreature(SpiritGuieH))
                      c2->SetVisibility(VISIBILITY_ON);
              }
           }
       }
    }

    uint32 GetTeam()
    {
      return m_team;
    }
};

struct WorldPvPWGWorkShopData
{
    WorldPvPGraveYardWG* m_GY;
    OutdoorPvPWG *m_WG;
    uint32 m_Type;
    uint32 m_State;
    uint32 m_WorldState;
    uint32 m_TeamControl;
    uint32 m_TextId;
    uint8 number;
    Map* Mmap;
    bool map;

    std::list<ObjectGuid> m_CreatureA;
    std::list<ObjectGuid> m_CreatureH;
    std::list<ObjectGuid> m_GameObjectA;
    std::list<ObjectGuid> m_GameObjectH;

    WorldPvPWGWorkShopData(OutdoorPvPWG * WG)
    {
        m_WG = WG;
        m_GY = 0;
        m_Type = 0;
        m_State = 0;
        m_WorldState = 0;
        m_TeamControl = 0;
        m_TextId = 0;
        number = 0;
        Mmap = 0;
        map = false;
    }

    uint32 GetType()
    {
       return m_Type;
    }

    void SetGraveYard(WorldPvPGraveYardWG* GY)
    {
      m_GY = GY;
    }

    void AddCreature(Creature* pCreature, uint32 team)
    {
        if(!map)
        {
           Mmap = pCreature->GetMap();
           map = true;
        }

        if(m_TeamControl == team)
        {
            if(m_TeamControl == ALLIANCE)
              m_CreatureA.push_back(pCreature->GetObjectGuid());
            else if(m_TeamControl == HORDE)
              m_CreatureH.push_back(pCreature->GetObjectGuid());
        }
        else if(m_TeamControl != team)
        {
             pCreature->setFaction(35);
             pCreature->SetVisibility(VISIBILITY_OFF);

            if(m_TeamControl == ALLIANCE)
              m_CreatureH.push_back(pCreature->GetObjectGuid());
            else if(m_TeamControl == HORDE)
              m_CreatureA.push_back(pCreature->GetObjectGuid());
        }
    }

    void AddGameObject(GameObject* pGo, uint32 team)
    {
        if(!map)
        {
           Mmap = pGo->GetMap();
           map = true;
        }

        if(m_TeamControl == team)
        {
            if(m_TeamControl == ALLIANCE)
              m_GameObjectA.push_back(pGo->GetObjectGuid());
            else if(m_TeamControl == HORDE)
              m_GameObjectH.push_back(pGo->GetObjectGuid());
        }
        else if(m_TeamControl != team)
        {
            pGo->SetPhaseMask(100,true);

            if(m_TeamControl == ALLIANCE)
              m_GameObjectH.push_back(pGo->GetObjectGuid());
            else if(m_TeamControl == HORDE)
              m_GameObjectA.push_back(pGo->GetObjectGuid());
        }
    }

    void Init(uint8 n,uint32 worldstate, uint32 type, uint32 text)
    {
        number = n;
        m_WorldState = worldstate;
        m_Type = type;
        m_TextId = text;

         if(m_Type < WORLD_PVP_WG_WORKSHOP_KEEP_WEST)
         {
             m_TeamControl = m_WG->GetAttacker();
             switch(m_TeamControl)
             {
                case ALLIANCE:
                    m_State = WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT;
                    m_WG->SendUpdateWorldState(m_WorldState, m_State);
                    break;
                case HORDE:
                    m_State = WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT;
                    m_WG->SendUpdateWorldState(m_WorldState, m_State);
                    break;
             }
         }
         else
         {
             m_TeamControl = m_WG->GetDefender();
             switch(m_TeamControl)
             {
                case ALLIANCE:
                    m_State = WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT;
                    m_WG->SendUpdateWorldState(m_WorldState, m_State);
                    break;
                case HORDE:
                    m_State = WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT;
                    m_WG->SendUpdateWorldState(m_WorldState, m_State);
                    break;
             }
         }
    }

    void Rebuild()
    {

        if(m_TeamControl == ALLIANCE)
        {
           for (std::list<ObjectGuid>::iterator itr = m_CreatureA.begin(); itr != m_CreatureA.end(); ++itr)
           {
             if(Creature* pCreature = Mmap->GetCreature((*itr)))
                pCreature->Respawn();
           }
        }
        else if(m_TeamControl == HORDE)
        {
           for (std::list<ObjectGuid>::iterator itr = m_CreatureH.begin(); itr != m_CreatureH.end(); ++itr)
           {
             if(Creature* pCreature = Mmap->GetCreature((*itr)))
                pCreature->Respawn();
           }
        }

        if(map)
        {
           if(GameObject* gBanner = Mmap->GetGameObject(m_WG->gBanner[number]))
           {
              gBanner->Respawn();
              if(m_TeamControl == ALLIANCE)
                 m_WG->SetBannerArtKit(gBanner,GO_ARTKIT_BANNER_ALLIANCE);
              else
                 m_WG->SetBannerArtKit(gBanner,GO_ARTKIT_BANNER_HORDE);
           }

          if(GameObject* pGo =  Mmap->GetGameObject(m_WG->gBuilding[number]))
             pGo->Rebuild(((Unit*)m_WG->GetPlayerInZone()));
        }

    }

    void ChangeControl(uint32 team)
    {
        switch (team)
        {
            case ALLIANCE:
                  if(m_TeamControl != ALLIANCE)
                  {
                    m_TeamControl = ALLIANCE;

                    if(map)
                    {
                       for (std::list<ObjectGuid>::iterator itr = m_CreatureH.begin(); itr != m_CreatureH.end(); ++itr)
                       {
                          if(Creature* pCreature = Mmap->GetCreature((*itr)))
                          {
                              pCreature->setFaction(35);
                              pCreature->SetVisibility(VISIBILITY_OFF);
                          }
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_CreatureA.begin(); itr != m_CreatureA.end(); ++itr)
                       {
                          if(Creature* pCreature = Mmap->GetCreature((*itr)))
                          {
                             pCreature->SetVisibility(VISIBILITY_ON);
                             pCreature->setFaction(NPC_FACTION_A);
                          }
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_GameObjectH.begin(); itr != m_GameObjectH.end(); ++itr)
                       {
                          if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                          {
                             pGo->SetPhaseMask(100,true);
                          }
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_GameObjectA.begin(); itr != m_GameObjectA.end(); ++itr)
                       {
                          if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                          {
                             pGo->SetPhaseMask(1,true);
                          }
                       }

                      if(GameObject* gBanner = Mmap->GetGameObject(m_WG->gBanner[number]))
                          m_WG->SetBannerArtKit(gBanner,GO_ARTKIT_BANNER_ALLIANCE);

                      if(GameObject* pGo =  Mmap->GetGameObject(m_WG->gBuilding[number]))
                         pGo->Rebuild(((Unit*)m_WG->GetPlayerInZone()));
                    }

                    m_State = WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT;
                    m_WG->SendUpdateWorldState(m_WorldState, m_State);

                    if(m_Type < WORLD_PVP_WG_WORKSHOP_KEEP_WEST)
                    {
                      m_GY->ChangeTeam(m_TeamControl);
                      sObjectMgr.RemoveGraveYardLink(m_GY->GetId(), ZONE_ID_WINTERGRASP, HORDE);
                      sObjectMgr.SetGraveYardLinkTeam(m_GY->GetId(), ZONE_ID_WINTERGRASP, ALLIANCE);
                    }
                  }
                  else
                    Rebuild();
                  break;
            case HORDE:
                  if(m_TeamControl != HORDE)
                  {
                    m_TeamControl = HORDE;

                    if(map)
                    {
                       for (std::list<ObjectGuid>::iterator itr = m_CreatureA.begin(); itr != m_CreatureA.end(); ++itr)
                       {
                          if(Creature* pCreature = Mmap->GetCreature((*itr)))
                          {
                              pCreature->setFaction(35);
                              pCreature->SetVisibility(VISIBILITY_OFF);
                          }
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_CreatureH.begin(); itr != m_CreatureH.end(); ++itr)
                       {
                          if(Creature* pCreature = Mmap->GetCreature((*itr)))
                          {
                             pCreature->SetVisibility(VISIBILITY_ON);
                             pCreature->setFaction(NPC_FACTION_H);
                          }
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_GameObjectA.begin(); itr != m_GameObjectA.end(); ++itr)
                       {
                          if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                          {
                             pGo->SetPhaseMask(100,true);
                          }
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_GameObjectH.begin(); itr != m_GameObjectH.end(); ++itr)
                       {
                          if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                          {
                             pGo->SetPhaseMask(1,true);
                          }
                       }


                      if(GameObject* gBanner = Mmap->GetGameObject(m_WG->gBanner[number]))
                          m_WG->SetBannerArtKit(gBanner,GO_ARTKIT_BANNER_HORDE);

                      if(GameObject* pGo =  Mmap->GetGameObject(m_WG->gBuilding[number]))
                         pGo->Rebuild(((Unit*)m_WG->GetPlayerInZone()));
                    }


                      m_State = WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT;
                      m_WG->SendUpdateWorldState(m_WorldState, m_State);

                    if(m_Type < WORLD_PVP_WG_WORKSHOP_KEEP_WEST)
                    {
                      m_GY->ChangeTeam(m_TeamControl);
                      sObjectMgr.RemoveGraveYardLink(m_GY->GetId(), ZONE_ID_WINTERGRASP, ALLIANCE);
                      sObjectMgr.SetGraveYardLinkTeam(m_GY->GetId(), ZONE_ID_WINTERGRASP, HORDE);
                    }
                  }
                  else
                    Rebuild();
                  break;
        }
            m_WG->UpdateCounterVehicle(false);
    }

};
#endif
