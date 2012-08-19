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

#include "OutdoorPvP.h"
#include "../Language.h"
#include "../World.h"
#include "../ObjectMgr.h"
#include "../Object.h"
#include "../GameObject.h"
#include "../Player.h"
#include "OutdoorPvPWG.h"

/**
   Function that adds a player to the players of the affected outdoor pvp zones

   @param   player to add
   @param   whether zone is main outdoor pvp zone or a affected zone
 */
void OutdoorPvP::HandlePlayerEnterZone(Player* player, bool isMainZone)
{
    // need move		 
	if(player->GetTeam() == ALLIANCE)
        m_sZonePlayersAlliance.insert(player->GetObjectGuid());
	else if(player->GetTeam() == HORDE)
        m_sZonePlayersHorde.insert(player->GetObjectGuid());

    m_zonePlayers[player->GetObjectGuid()] = isMainZone;
}

/**
   Function that removes a player from the players of the affected outdoor pvp zones

   @param   player to remove
   @param   whether zone is main outdoor pvp zone or a affected zone
 */
void OutdoorPvP::HandlePlayerLeaveZone(Player* player, bool isMainZone)
{
    if (m_zonePlayers.erase(player->GetObjectGuid()))
    {
        // remove the world state information from the player
        if (isMainZone && !player->GetSession()->PlayerLogout())
            SendRemoveWorldStates(player);

        sLog.outDebug("Player %s left an Outdoor PvP zone", player->GetName());
    }
	
	// need move
	if(player->GetTeam() == ALLIANCE)
        m_sZonePlayersAlliance.erase(player->GetObjectGuid());
	else if(player->GetTeam() == HORDE)
        m_sZonePlayersHorde.erase(player->GetObjectGuid());	
		
}

/**
   Function that updates the world state for all the players of the outdoor pvp zone

   @param   world state to update
   @param   new world state value
 */
void OutdoorPvP::SendUpdateWorldState(uint32 field, uint32 value)
{
    for (GuidZoneMap::iterator itr = m_zonePlayers.begin(); itr != m_zonePlayers.end(); ++itr)
    {
        // only send world state update to main zone
        if (!itr->second)
            continue;

        if (Player* player = sObjectMgr.GetPlayer(itr->first))
            player->SendUpdateWorldState(field, value);
    }
}

/**
   Function that handles player kills in the main outdoor pvp zones

   @param   player who killed another player
   @param   victim who was killed
 */
void OutdoorPvP::HandlePlayerKill(Player* killer, Unit* victim)
{
    if (Group* group = killer->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* groupMember = itr->getSource();

            if (!groupMember)
                continue;

            // skip if too far away
            if (!groupMember->IsAtGroupRewardDistance(victim))
                continue;

            // creature kills must be notified, even if not inside objective / not outdoor pvp active
            // player kills only count if active and inside objective
            if (groupMember->CanUseOutdoorCapturePoint())
                HandlePlayerKillInsideArea(groupMember, victim);
        }
    }
    else
    {
        // creature kills must be notified, even if not inside objective / not outdoor pvp active
        if (killer && killer->CanUseOutdoorCapturePoint())
            HandlePlayerKillInsideArea(killer, victim);
    }
}

// apply a team buff for the main and affected zones
void OutdoorPvP::BuffTeam(Team team, uint32 spellId, bool remove /*= false*/)
{
    for (GuidZoneMap::iterator itr = m_zonePlayers.begin(); itr != m_zonePlayers.end(); ++itr)
    {
        Player* player = sObjectMgr.GetPlayer(itr->first);
        if (player && player->GetTeam() == team)
        {
            if (remove)
                player->RemoveAurasDueToSpell(spellId);
            else
                player->CastSpell(player, spellId, true);
        }
    }
}

uint32 OutdoorPvP::GetBannerArtKit(Team team, uint32 artKitAlliance /*= CAPTURE_ARTKIT_ALLIANCE*/, uint32 artKitHorde /*= CAPTURE_ARTKIT_HORDE*/, uint32 artKitNeutral /*= CAPTURE_ARTKIT_NEUTRAL*/)
{
    switch (team)
    {
        case ALLIANCE:
            return artKitAlliance;
        case HORDE:
            return artKitHorde;
        default:
            return artKitNeutral;
    }
}

void OutdoorPvP::SetBannerVisual(const WorldObject* objRef, ObjectGuid goGuid, uint32 artKit, uint32 animId)
{
    if (GameObject* go = objRef->GetMap()->GetGameObject(goGuid))
        SetBannerVisual(go, artKit, animId);
}

void OutdoorPvP::SetBannerVisual(GameObject* go, uint32 artKit, uint32 animId)
{
    go->SendGameObjectCustomAnim(go->GetObjectGuid(), animId);
    go->SetGoArtKit(artKit);
    go->Refresh();
}

void OutdoorPvP::RespawnGO(const WorldObject* objRef, ObjectGuid goGuid, bool respawn)
{
    if (GameObject* go = objRef->GetMap()->GetGameObject(goGuid))
    {
        go->SetRespawnTime(7 * DAY);

        if (respawn)
            go->Refresh();
        else if (go->isSpawned())
            go->SetLootState(GO_JUST_DEACTIVATED);
    }
}

void OutdoorPvP::FillInitialWorldState(uint32 zoneId, uint32 stateId, uint32& value)
{
    uint32 stateValue = sWorldStateMgr.GetWorldStateValueFor(UINT32_MAX, UINT32_MAX, zoneId, UINT32_MAX, stateId);

    if (stateValue != UINT32_MAX)
    {
        value = stateValue;
    }
    else
        sWorldStateMgr.FillInitialWorldState(stateId, value, WORLD_STATE_TYPE_ZONE, zoneId);
}

Player* WorldPvP::GetPlayersAlliance()
{
    Player* pPlayers = NULL;

    for (GuidSet::iterator itr = m_sZonePlayersAlliance.begin(); itr != m_sZonePlayersAlliance.end(); ++itr)
    {
        Player* pPlayer = sObjectMgr.GetPlayer(*itr);
		pPlayers = pPlayer;
    }
    
	return pPlayers;
}

Player* WorldPvP::GetPlayersHorde()
{
    Player* pPlayers = NULL;

    for (GuidSet::iterator itr = m_sZonePlayersHorde.begin(); itr != m_sZonePlayersHorde.end(); ++itr)
    {
        Player* pPlayer = sObjectMgr.GetPlayer(*itr);
		pPlayers = pPlayer;
    }

	return pPlayers;
}

uint32 WorldPvP::CountPlayersAlliance()
{
  uint32 count = 0;

    for (GuidSet::iterator itr = m_sZonePlayersAlliance.begin(); itr != m_sZonePlayersAlliance.end(); ++itr)
    {
      count = count + 1;
    }

  return count;
}

uint32 WorldPvP::CountPlayersHorde()
{
  uint32 count = 0;

    for (GuidSet::iterator itr = m_sZonePlayersHorde.begin(); itr != m_sZonePlayersHorde.end(); ++itr)
    {
       count = count + 1;
    }

  return count;
}

void WorldPvP::CompleteQuest(Player *player, uint32 entry)
{
    Quest const* pQuest = sObjectMgr.GetQuestTemplate(entry);

    // If player doesn't have the quest
    if (!pQuest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
        return;

    // Add quest items for quests that require items
    for(uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
    {
        uint32 id = pQuest->ReqItemId[x];
        uint32 count = pQuest->ReqItemCount[x];
        if (!id || !count)
            continue;

        uint32 curItemCount = player->GetItemCount(id,true);

        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, id, count - curItemCount );
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = player->StoreNewItem( dest, id, true);
            player->SendNewItem(item,count-curItemCount, true, false);
        }
    }

    // All creature/GO slain/casted (not required, but otherwise it will display "Creature slain 0/10")
    for(uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        int32 creature = pQuest->ReqCreatureOrGOId[i];
        uint32 creaturecount = pQuest->ReqCreatureOrGOCount[i];

        if (uint32 spell_id = pQuest->ReqSpell[i])
        {
            for(uint16 z = 0; z < creaturecount; ++z)
                player->CastedCreatureOrGO(creature, ObjectGuid(), spell_id);
        }
        else if (creature > 0)
        {
            if (CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creature))
                for(uint16 z = 0; z < creaturecount; ++z)
                    player->KilledMonster(cInfo, ObjectGuid());
        }
        else if (creature < 0)
        {
            for(uint16 z = 0; z < creaturecount; ++z)
                player->CastedCreatureOrGO(-creature, ObjectGuid(), 0);
        }
    }

    // If the quest requires reputation to complete
    if(uint32 repFaction = pQuest->GetRepObjectiveFaction())
    {
        uint32 repValue = pQuest->GetRepObjectiveValue();
        uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
        if (curRep < repValue)
            if (FactionEntry const *factionEntry = sFactionStore.LookupEntry(repFaction))
                player->GetReputationMgr().SetReputation(factionEntry,repValue);
    }

    // If the quest requires money
    int32 ReqOrRewMoney = pQuest->GetRewOrReqMoney();
    if (ReqOrRewMoney < 0)
        player->ModifyMoney(-ReqOrRewMoney);
}

WorldSafeLocsEntry const* WorldPvP::GetClosestGraveYardWG(Player* player)
{
   WorldSafeLocsEntry const* good_entry = NULL;

   if(player->GetMapId() == 571 && player->GetZoneId() == 4197)
   {
      WorldPvP* pWG = sWorldPvPMgr.GetWorldPvPToZoneId(ZONE_ID_WINTERGRASP);
	  WorldPvPWG* WG = ((WorldPvPWG*)pWG);

      good_entry = WG->GetClosestGraveYardWG(player);
   }
    
   return good_entry;
}