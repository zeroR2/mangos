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

#include "Common.h"
#include "../ObjectGuid.h"
#include "../SharedDefines.h"
#include "BattleFieldMgr.h"

class Player;
class WorldPacket;
class Creature;
class GameObject;
class Unit;
class WorldObject;

class BattleField
{
    friend class BattleFieldMgr;

    public:
        BattleField() {}
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

        // Called on creature death
        virtual void HandleCreatureDeath(Creature* /*creature*/) {}

        // called when a player uses a gameobject related to outdoor pvp events
        virtual bool HandleGameObjectUse(Player* /*player*/, GameObject* /*go*/) { return false; }

        // called when a player triggers an areatrigger
        virtual bool HandleAreaTrigger(Player* /*player*/, uint32 /*triggerId*/) { return false; }

        // called when a player drops a flag
        virtual bool HandleDropFlag(Player* /*player*/, uint32 /*spellId*/) { return false; }

        // update - called by the OutdoorPvPMgr
        virtual void Update(uint32 /*diff*/) {}

        // handle npc/player kill
        void HandlePlayerKill(Player* killer, Unit* victim);

        // Damage GO
        virtual void EventPlayerDamageGO(Player* /*player*/, GameObject* /*target_obj*/, uint32 /*eventId*/, uint32 /*bySpellId*/) {}

         // send world state update to all players present
        void SendUpdateWorldState(uint32 field, uint32 value);

        // remove world states
        virtual void SendRemoveWorldStates(Player* /*player*/) {}

        // invite void
        void InvitePlayerToQueue(Player* player);
        void InvitePlayerToWar(Player* player);

        uint32 GetTimer() {return m_timer;}

        team GetDefender() {return m_defenderTeam;}
        team GetAttack() {return m_attackTeam;}

    protected:

        // Player related stuff
        virtual void HandlePlayerEnterZone(Player* /*player*/);
        virtual void HandlePlayerLeaveZone(Player* /*player*/);

        // handle npc/player kill
        virtual void HandlePlayerKillInsideArea(Player* /*killer*/, Unit* /*victim*/) {}

        // set banner visual
        void SetBannerVisual(const WorldObject* objRef, ObjectGuid goGuid, uint32 artKit, uint32 animId);
        void SetBannerVisual(GameObject* go, uint32 artKit, uint32 animId);

        // invite zone players to queue. it use before start battle
        void InviteZonePlayersToQueue();

        // value for timer worldstates in seconds.
        uint32 m_timer;

        team m_defenderTeam;
        team m_attackTeam;

         // store the players inside the area
        GuidSet m_zonePlayers;
        GuidSet m_queuePlayers;
        GuidSet m_warPlayers[2];
        GuidSet m_invitedPlayers;
};

#endif
