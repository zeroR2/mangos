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

#ifndef BATTLEFIELD_H
#define BATTLEFIELD_H

#include "../Object.h"
#include "../ObjectGuid.h"
#include "../SharedDefines.h"
#include "BattleFieldMgr.h"

class Player;
class WorldPacket;
class Creature;
class GameObject;
class Unit;
class WorldObject;
class Group;

class BattleField
{
    friend class BattleFieldMgr;

    public:
        BattleField();
        ~BattleField() {}

        // called when the zone is initialized
        virtual void FillInitialWorldStates(WorldPacket& /*data*/, uint32& /*count*/) {}

        // Process Capture event
        virtual bool HandleEvent(uint32 /*eventId*/, GameObject* /*go*/) { return false; }

        // handle capture objective complete
        virtual void HandleObjectiveComplete(uint32 /*eventId*/, std::list<Player*> /*players*/, Team /*team*/) {}

        // Called when a creature or gameobject is created
        virtual void HandleCreatureCreate(Creature* /*creature*/) {}
        virtual void HandleGameObjectCreate(GameObject* /*go*/) {}

        virtual void EnterToBattle(Player* /*player*/) {}
        virtual void LeaveToBattle(Player* /*player*/) {}

        // Called on creature death
        virtual void HandleCreatureDeath(Creature* /*creature*/) {}

        // called when a player uses a gameobject related to outdoor pvp events
        virtual bool HandleGameObjectUse(Player* /*player*/, GameObject* /*go*/) { return false; }

        // called when a player triggers an areatrigger
        virtual bool HandleAreaTrigger(Player* /*player*/, uint32 /*triggerId*/) { return false; }

        // called when a player drops a flag
        virtual bool HandleDropFlag(Player* /*player*/, uint32 /*spellId*/) { return false; }

        // update - called by the OutdoorPvPMgr
        virtual void Update(uint32 /*diff*/);

        // handle npc/player kill
        void HandlePlayerKill(Player* killer, Unit* victim);

        // Damage GO
        virtual void EventPlayerDamageGO(Player* /*player*/, GameObject* /*target_obj*/, uint32 /*eventId*/, uint32 /*bySpellId*/) {}

         // send world state update to all players present
        void SendUpdateWorldState(uint32 field, uint32 value);

        // remove world states
        virtual void SendRemoveWorldStates(Player* /*player*/) {}

        // invite system
        void InvitePlayerToQueue(Player* player);
        void InvitePlayerToBattle(Player* player);
        void AcceptInvitePlayerToQueue(Player* player);
        void AcceptInvitePlayerToBattle(Player* player);
        void CancelInvitePlayerToBattle(Player* player);
        void InviteQueuePlayerToBattle();

        void KickFromBattleField(Player* player,uint32 spellid);

        bool InProgressBattle() {return ProgressBattle;}

        uint32 GetDefender() {return m_defenderTeam;}
        uint32 GetAttack() {return m_attackTeam;}

        uint32 GetZone() {return m_ZoneId;}

    protected:

        // Player related stuff
        virtual void HandlePlayerEnterZone(Player* /*player*/);
        virtual void HandlePlayerLeaveZone(Player* /*player*/);

        // handle npc/player kill
        virtual void HandlePlayerKillInsideArea(Player* /*killer*/, Unit* /*victim*/) {}

        // set banner visual
        void SetBannerVisual(const WorldObject* objRef, ObjectGuid goGuid, uint32 artKit, uint32 animId);
        void SetBannerVisual(GameObject* go, uint32 artKit, uint32 animId);

        // If it is true battle is in progress
        bool ProgressBattle;

        void StartTheBattle();
        void EndTheBattle();

        bool CanStratBattle;
        bool finishNonBattle;
        bool finishBattle;

        // Group system
        void AddGroup(Player* player);
        Group* m_group[2];

        // data
        uint32 m_BattleFieldId;
        uint32 m_ZoneId;
        uint32 m_MapId;

        uint32 m_timerinviteZonePlayerToQueue; // Called 1 minute before start the battle. Define in BattleField.cpp

        uint32 m_TimeAcceptInvite; // Use for send invited text_box in seconds. Define in BattleField.cpp

        // Start enter Queue player to battle when it is in progress
        bool canentermoreplayer;
        uint32 timerentermoreplayer; // 1 minute

        // value for timer worldstates in seconds. Get they from config in zone script
        uint32 m_timerNonBattle;
        uint32 m_timerBattle;

        uint32 m_defenderTeam;
        uint32 m_attackTeam;

        // minimum and maximum players by team, they set in config for specific zone
        uint32 m_mimPlayers; // Only use for preferably in the queue. IMPORTANT : The battle can be start with mium player or less in each team
        uint32 m_maxPlayers;

        // minimum level for entering zone, it set in config for specific zone
        uint32 m_minLevel;

        // Kick position,it set in zone script. If spell is 0 use the KickPosition
        WorldLocation KickPosition;
        uint32 m_spellKick;

         // store the players inside the area
        GuidSet m_zonePlayers;
        GuidSet m_queuePreferencePlayers[2];
        GuidSet m_queuePlayers[2];
        GuidSet m_invitedPlayers[2];
        GuidSet m_battlePlayers[2];
};

#endif
