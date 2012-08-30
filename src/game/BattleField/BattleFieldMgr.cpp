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

#include "BattleFieldMgr.h"
#include "Policies/SingletonImp.h"
#include "BattleField.h"
#include "Log.h"

INSTANTIATE_SINGLETON_1(BattleFieldMgr);

BattleFieldMgr::BattleFieldMgr()
{
    m_updateTimer.SetInterval(TIMER_BATTLEFIELD_MGR_UPDATE);
    memset(&m_scripts, 0, sizeof(m_scripts));
}

BattleFieldMgr::~BattleFieldMgr()
{
    for (uint8 i = 0; i < MAX_BATTLEFIELD_ID; ++i)
        delete m_scripts[i];
}

/**
Function which loads all BattleField scripts
*/
void BattleFieldMgr::InitBattleField()
{
    uint8 counter = 0;

    sLog.outString();
    sLog.outString(">> Loaded %u BatteField zones", counter);
}


BattleField* BattleFieldMgr::GetScript(uint32 zoneId)
{
    switch (zoneId)
    {
        case ZONE_ID_WINTERGRASP:
            return m_scripts[BATTLEFIELD_ID_WG];
        default:
            return NULL;
    }
}

BattleField* BattleFieldMgr::GetScriptById(uint32 Id)
{
    switch (Id)
    {
        case BATTLEFIELD_ID_WG:
            return m_scripts[BATTLEFIELD_ID_WG];
        default:
            return NULL;
    }
}
/**
Function that handles the players which enters a specific zone

@param player to be handled in the event
@param zone id used for the current outdoor pvp script
*/
void BattleFieldMgr::HandlePlayerEnterZone(Player* player, uint32 zoneId)
{
    if (BattleField* script = GetScript(zoneId))
        script->HandlePlayerEnterZone(player);
}

/**
Function that handles the player who leaves a specific zone

@param player to be handled in the event
@param zone id used for the current outdoor pvp script
*/
void BattleFieldMgr::HandlePlayerLeaveZone(Player* player, uint32 zoneId)
{
    // teleport: called once from Player::CleanupsBeforeDelete, once from Player::UpdateZone
    if (BattleField* script = GetScript(zoneId))
        script->HandlePlayerLeaveZone(player);
}

void BattleFieldMgr::Update(uint32 diff)
{
    m_updateTimer.Update(diff);
    if (!m_updateTimer.Passed())
        return;

    for (uint8 i = 0; i < MAX_BATTLEFIELD_ID; ++i)
        if (m_scripts[i])
            m_scripts[i]->Update(m_updateTimer.GetCurrent());

    m_updateTimer.Reset();
}
