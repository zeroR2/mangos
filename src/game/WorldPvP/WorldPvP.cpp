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

#include "WorldPvP.h"
#include "WorldPvPWG.h"

/**
   Function that a players to a players set

   @param   player to be added to set
 */
void WorldPvP::HandlePlayerEnterZone(Player* pPlayer)
{
    if (!pPlayer)
        return;

	if(pPlayer->GetTeam() == ALLIANCE)
        m_sZonePlayersAlliance.insert(pPlayer->GetObjectGuid());
	else if(pPlayer->GetTeam() == HORDE)
        m_sZonePlayersHorde.insert(pPlayer->GetObjectGuid());
		
    m_sZonePlayers.insert(pPlayer->GetObjectGuid());
}

/**
   Function that a players to a players set

   @param   player to be removed
 */
void WorldPvP::HandlePlayerLeaveZone(Player* pPlayer)
{
    if (!pPlayer)
        return;

    m_sZonePlayers.erase(pPlayer->GetObjectGuid());

	if(pPlayer->GetTeam() == ALLIANCE)
        m_sZonePlayersAlliance.erase(pPlayer->GetObjectGuid());
	else if(pPlayer->GetTeam() == HORDE)
        m_sZonePlayersHorde.erase(pPlayer->GetObjectGuid());
	
    sLog.outDebug("Player %s left an outdoorpvp zone", pPlayer->GetName());
}

/**
   Function that updates world state for all the players in an outdoor pvp area

   @param   world state it to update
   @param   value which should update the world state
 */
void WorldPvP::SendUpdateWorldState(uint32 uiField, uint32 uiValue)
{
    for (GuidSet::iterator itr = m_sZonePlayers.begin(); itr != m_sZonePlayers.end(); ++itr)
    {
        Player* pPlayer = sObjectMgr.GetPlayer(*itr);
        if (pPlayer)
            pPlayer->SendUpdateWorldState(uiField, uiValue);
    }
}

/**
   Function that handles the player kill in outdoor pvp

   @param   player which kills another player
   @param   player or unit (pet) which is victim
 */
void WorldPvP::HandlePlayerKill(Player* pKiller, Unit* pVictim)
{
    if (Group* pGroup = pKiller->GetGroup())
    {
        for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* pGroupGuy = itr->getSource();

            if (!pGroupGuy)
                continue;

            // skip if too far away
            if (!pGroupGuy->IsAtGroupRewardDistance(pVictim))
                continue;

            // creature kills must be notified, even if not inside objective / not outdoor pvp active
            // player kills only count if active and inside objective
            if (pGroupGuy->IsWorldPvPActive())
                HandlePlayerKillInsideArea(pGroupGuy, pVictim);
        }
    }
    else
    {
        // creature kills must be notified, even if not inside objective / not outdoor pvp active
        if (pKiller && (pKiller->IsWorldPvPActive()))
            HandlePlayerKillInsideArea(pKiller, pVictim);
    }
}

// register this zone as an outdoor pvp script
void WorldPvP::RegisterZone(uint32 uiZoneId)
{
    sWorldPvPMgr.AddZone(uiZoneId, this);
    FillInitialWorldStates(uiZoneId);
}

// return if has player inside the zone
bool WorldPvP::HasPlayer(Player* pPlayer) const
{
    if (!pPlayer)
        return false;

    return m_sZonePlayers.find(pPlayer->GetObjectGuid()) != m_sZonePlayers.end();
}

// lock a capture point
void WorldPvP::LockCapturePoint(uint32 pointEntry, bool isLocked)
{
     sWorldPvPMgr.SetCapturePointLockState(pointEntry, isLocked);
}

// reset a capture point slider
void WorldPvP::ResetCapturePoint(uint32 pointEntry, float fValue)
{
    sWorldPvPMgr.SetCapturePointSlider(pointEntry, fValue);
}

// apply a team buff for the specific zone
void WorldPvP::DoProcessTeamBuff(Team uiTeam, uint32 uiSpellId, bool bRemove)
{
    for (GuidSet::iterator itr = m_sZonePlayers.begin(); itr != m_sZonePlayers.end(); ++itr)
    {
        if (!(*itr))
            continue;

        Player* pPlayer = sObjectMgr.GetPlayer(*itr);

        if (!pPlayer)
            continue;

        if (pPlayer->GetTeam() == uiTeam)
        {
            if (!bRemove)
                pPlayer->CastSpell(pPlayer, uiSpellId, true);
            else
            {
                if (pPlayer->HasAura(uiSpellId))
                    pPlayer->RemoveAurasDueToSpell(uiSpellId);
            }
        }
    }
}

/// Get the first found Player* (with requested properties) in the zone. Can return NULL.
Player* WorldPvP::GetPlayerInZone(bool bOnlyAlive /*=false*/, bool bCanBeGamemaster /*=true*/)
{
    for (GuidSet::iterator itr = m_sZonePlayers.begin(); itr != m_sZonePlayers.end(); ++itr)
    {
        if (!(*itr))
            continue;

        Player* pPlayer = sObjectMgr.GetPlayer(*itr);

        if (!pPlayer)
            continue;

        if ((!bOnlyAlive || pPlayer->isAlive()) && (bCanBeGamemaster || !pPlayer->isGameMaster()))
            return pPlayer;
    }

    return NULL;
}

void WorldPvP::FillInitialWorldState(uint32 zoneId, uint32 stateId, uint32& value)
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