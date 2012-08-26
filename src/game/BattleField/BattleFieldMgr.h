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

#ifndef BATTLEFIELD_MGR_H
#define BATTLEFIELD_MGR_H

#include "Common.h"
#include "Policies/Singleton.h"
#include "Timer.h"

enum
{
    TIMER_BATTLEFIELD_MGR_UPDATE = MINUTE * IN_MILLISECONDS // 1 minute is enough for us but this might change with wintergrasp support
};

enum BattleFieldTypes
{
    BATTLEFIELD_ID_WG = 0,

    MAX_BATTLEFIELD_ID
};

enum battleFieldZones
{
    ZONE_ID_WINTERGRASP                = 4197,
};

class Player;
class BattleField;

class BattleFieldMgr
{
    public:
        BattleFieldMgr();
        ~BattleFieldMgr();

        // load all outdoor pvp scripts
        void InitBattleField();

        // called when a player enters an outdoor pvp area
        void HandlePlayerEnterZone(Player* player, uint32 zoneId);

        // called when player leaves an outdoor pvp area
        void HandlePlayerLeaveZone(Player* player, uint32 zoneId);

        // return assigned outdoor pvp script
        BattleField* GetScript(uint32 zoneId);

        void Update(uint32 diff);

    private:
        // contains all outdoor pvp scripts
        BattleField* m_scripts[MAX_BATTLEFIELD_ID];

        // update interval
        ShortIntervalTimer m_updateTimer;
};

#define sBattleFieldMgr MaNGOS::Singleton<BattleFieldMgr>::Instance()

#endif
