/*
 * Copyright (C) 2011-2012 /dev/rsa for MangosR2 <http://github.com/mangosR2>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Map.h"
#include "World.h"
#include "WorldStateMgr.h"
#include "ProgressBar.h"
#include "Player.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "SQLStorages.h"
#include "BattleGroundMgr.h"

INSTANTIATE_SINGLETON_1(WorldStateMgr);

void WorldStateMgr::Initialize()
{
    // for reload case - cleanup states first
    m_worldStateTemplates.clear();
    // Load some template types
    LoadTemplatesFromDBC();
    LoadTemplatesFromDB();
    LoadTemplatesFromObjectTemplateDB();
    // Load states data
    LoadFromDB();
    // Create all needed states (if not loaded)
    CreateWorldStatesIfNeed();
};

void WorldStateMgr::Update()
{
    {
    // Update part 1 - calculating (and mark for cleanup)

        ReadGuard guard(GetLock());
        for (WorldStateMap::iterator itr = m_worldState.begin(); itr != m_worldState.end(); ++itr)
        {
            WorldState* state = &itr->second;
            switch (state->GetType())
            {
                case WORLD_STATE_TYPE_BGWEEKEND:
                {
                    for (uint32 i = 1; i < sBattlemasterListStore.GetNumRows(); ++i)
                    {
                        BattlemasterListEntry const * bl = sBattlemasterListStore.LookupEntry(i);
                        if (bl && bl->HolidayWorldStateId == state->GetId())
                        {
                            if (BattleGroundMgr::IsBGWeekend(BattleGroundTypeId(bl->id)))
                                state->SetValue(1);
                            else
                                state->SetValue(0);
                        }
                    }
                }
                case WORLD_STATE_TYPE_CUSTOM:
                case WORLD_STATE_TYPE_WORLD:
                case WORLD_STATE_TYPE_EVENT:
                case WORLD_STATE_TYPE_MAP:
                case WORLD_STATE_TYPE_ZONE:
                case WORLD_STATE_TYPE_AREA:
                case WORLD_STATE_TYPE_BATTLEGROUND:
                case WORLD_STATE_TYPE_CAPTURE_POINT:
                case WORLD_STATE_TYPE_WORLD_UNCOMMON:
                default:
                    break;
            }
        }
    }

    // Saving data (and DB cleanup)
    SaveToDB();

    {
    // Update part 2 - remove states with WORLD_STATE_FLAG_DELETED flag
        WriteGuard guard(GetLock());
        for (WorldStateMap::iterator itr = m_worldState.begin(); itr != m_worldState.end();)
        {
            if (itr->second.HasFlag(WORLD_STATE_FLAG_DELETED))
                m_worldState.erase(itr);
            else
                ++itr;
        }
    }

};

void WorldStateMgr::LoadTemplatesFromDBC()
{
    uint32 count = 0;
    for (uint32 i = 1; i < sBattlemasterListStore.GetNumRows(); ++i)
    {
        BattlemasterListEntry const * bl = sBattlemasterListStore.LookupEntry(i);
        if (bl && bl->HolidayWorldStateId)
        {
            m_worldStateTemplates.insert(WorldStateTemplateMap::value_type(bl->HolidayWorldStateId, 
                WorldStateTemplate(bl->HolidayWorldStateId, WORLD_STATE_TYPE_BGWEEKEND, WORLD_STATE_TYPE_BGWEEKEND, (1 << WORLD_STATE_FLAG_INITIAL_STATE), BattleGroundMgr::IsBGWeekend(BattleGroundTypeId(bl->id)) ? 1 : 0, 0)));
            ++count;
        }
    }
    sLog.outString();
    sLog.outString( ">> Loaded static DBC templates for %u WorldStates", count);
};

void WorldStateMgr::LoadTemplatesFromDB()
{
    //                                                        0       1            2        3          4            5
    QueryResult* result = WorldDatabase.Query("SELECT `state_id`, `type`, `condition`, `flags`, `default`, `linked_id` FROM `worldstate_template`");
    if (!result)
    {
        sLog.outString(">> Table worldstate_template is empty:");
        sLog.outString();
        return;
    }

    uint32 count = 0;
    BarGoLink bar((int)result->GetRowCount());
    do
    {
        Field* fields = result->Fetch();

        bar.step();

        uint32   stateId        = fields[0].GetUInt32();
        uint32   type           = fields[1].GetUInt32();
        uint32   condition      = fields[2].GetUInt32();
        uint32   flags          = fields[3].GetUInt32();
        uint32   default_value  = fields[4].GetUInt32();
        uint32   linkedId       = fields[5].GetUInt32();

        // Store the state data
        m_worldStateTemplates.insert(WorldStateTemplateMap::value_type(stateId, WorldStateTemplate(stateId, type, condition, flags, default_value, linkedId)));
        ++count;
    }
    while (result->NextRow());

    sLog.outString();
    sLog.outString( ">> Loaded static templates for %u WorldStates", count);
    delete result;

};

void WorldStateMgr::LoadTemplatesFromObjectTemplateDB()
{
    //                                                     0  
    QueryResult* result = WorldDatabase.PQuery("SELECT `entry` FROM `gameobject_template` WHERE `type` = %u",(uint32)GAMEOBJECT_TYPE_CAPTURE_POINT);
    if (!result)
    {
        sLog.outString(">> No templates for object type 29 found in DB!");
        sLog.outString();
        return;
    }
    uint32 count = 0;
    BarGoLink bar((int)result->GetRowCount());
    do
    {
        Field* fields = result->Fetch();

        bar.step();

        uint32   goEntry        = fields[0].GetUInt32();

        GameObjectInfo const* goInfo = sGOStorage.LookupEntry<GameObjectInfo>(goEntry);
        if (!goInfo)
            continue;

        // setup state 1
        if (!goInfo->capturePoint.worldState1)
            continue;

        m_worldStateTemplates.insert(WorldStateTemplateMap::value_type(goInfo->capturePoint.worldState1,
            WorldStateTemplate(goInfo->capturePoint.worldState1, WORLD_STATE_TYPE_CAPTURE_POINT, goEntry, (1 << WORLD_STATE_FLAG_ACTIVE), 0, 0)));
        ++count;

        // setup state 2
        if (!goInfo->capturePoint.worldState2)
            continue;

        m_worldStateTemplates.insert(WorldStateTemplateMap::value_type(goInfo->capturePoint.worldState2,
            WorldStateTemplate(goInfo->capturePoint.worldState2, WORLD_STATE_TYPE_CAPTURE_POINT, goEntry, (1 << WORLD_STATE_FLAG_ACTIVE), 0, goInfo->capturePoint.worldState1)));
        ++count;

        // setup state 3
        if (!goInfo->capturePoint.worldState3)
            continue;

        m_worldStateTemplates.insert(WorldStateTemplateMap::value_type(goInfo->capturePoint.worldState3,
            WorldStateTemplate(goInfo->capturePoint.worldState2, WORLD_STATE_TYPE_CAPTURE_POINT, goEntry, (1 << WORLD_STATE_FLAG_ACTIVE), goInfo->capturePoint.neutralPercent, goInfo->capturePoint.worldState1)));
        ++count;
    }
    while (result->NextRow());

    sLog.outString();
    sLog.outString( ">> Loaded static templates for %u GAMEOBJECT_TYPE_CAPTURE_POINT linked WorldStates", count);
    delete result;

};

void WorldStateMgr::LoadFromDB()
{
    // cannot be reloaded!
    m_worldState.clear();
    //                                                            0           1       2            3        4        5            6
    QueryResult* result = CharacterDatabase.Query("SELECT `state_id`, `instance`, `type`, `condition`, `flags`, `value`, `renewtime` FROM `worldstate_data`");

    if (!result)
    {
        sLog.outString(">> Table worldstate_data is empty:");
        sLog.outString();
        return;
    }

    BarGoLink bar((int)result->GetRowCount());
    do
    {
        Field* fields = result->Fetch();

        bar.step();

        uint32   stateId        = fields[0].GetUInt32();
        uint32   instanceId     = fields[1].GetUInt32();
        uint32   type           = fields[2].GetUInt32();
        uint32   condition      = fields[3].GetUInt32();
        uint32   flags          = fields[4].GetUInt32();
        uint32   _value         = fields[5].GetUInt32();
        time_t   renewtime      = time_t(fields[6].GetUInt64());

        // Store the state data
        WorldStateTemplate const* tmpl = FindTemplate(stateId);
        if (tmpl)
        {
            m_worldState.insert(WorldStateMap::value_type(stateId, WorldState(tmpl, instanceId, flags, _value, renewtime)));
        }
        else if (type == WORLD_STATE_TYPE_CUSTOM)
        {
            DEBUG_FILTER_LOG(LOG_FILTER_DB_STRICTED_CHECK,"WorldStateMgr::LoadFromDB loaded custom state %u (%u %u %u %u %u)",
                stateId, instanceId, type, condition, flags, _value, renewtime);
            m_worldState.insert(WorldStateMap::value_type(stateId, WorldState(stateId, instanceId, flags, _value, renewtime)));
        }
        else
        {
            sLog.outError("WorldStateMgr::LoadFromDB unknown state %u (%u %u %u %u %u)",
                stateId, instanceId, type, condition, flags, _value, renewtime);
        }
    }
    while (result->NextRow());

    sLog.outString();
    sLog.outString( ">> Loaded data for %u WorldStates", m_worldState.size());
    delete result;

}

void WorldStateMgr::CreateWorldStatesIfNeed()
{
    for (WorldStateTemplateMap::const_iterator itr = m_worldStateTemplates.begin(); itr != m_worldStateTemplates.end(); ++itr)
    {
        if (itr->second.m_flags & (1 << WORLD_STATE_FLAG_INITIAL_STATE) &&
            (itr->second.m_stateType == WORLD_STATE_TYPE_WORLD ||
            itr->second.m_stateType == WORLD_STATE_TYPE_BGWEEKEND))
        {
            if (GetWorldState(itr->second.m_stateId, 0))
                continue;

            CreateWorldState(&itr->second, 0);
        }
    }
}

void WorldStateMgr::CreateLinkedWorldStatesIfNeed(WorldObject* object)
{
    if (!object)
        return;

    ObjectGuid guid = object->GetObjectGuid();
    uint32 instanceId = object->GetMap() ? object->GetMap()->GetInstanceId() :0;

    switch (guid.GetHigh())
    {
        case HIGHGUID_GAMEOBJECT:
        {
            GameObjectInfo const* goInfo = sGOStorage.LookupEntry<GameObjectInfo>(guid.GetEntry());
            if (!goInfo || goInfo->type != GAMEOBJECT_TYPE_CAPTURE_POINT)
            {
                sLog.outError("WorldStateMgr::CreateLinkedWorldStatesIfNeed try create linked WorldStates for %s, but currently this object type not supported!", guid.GetString().c_str());
                break;
            }

            // state 1
            if (goInfo->capturePoint.worldState1)
            {
                WorldState const* _state = NULL;
                if (_state  = GetWorldState(goInfo->capturePoint.worldState1, instanceId, WORLD_STATE_TYPE_CAPTURE_POINT))
                {
                    if (_state->GetValue() != 0)
                        DEBUG_LOG("WorldStateMgr::CreateLinkedWorldStatesIfNeed Warning - at load WorldState %u for %s current value %u not equal default %u!", 
                            goInfo->capturePoint.worldState1,
                            guid.GetString().c_str(),
                            _state->GetValue(),
                            0
                        );
                }
                else
                    _state = CreateWorldState(goInfo->capturePoint.worldState1, instanceId, 0);

                const_cast<WorldState*>(_state)->SetLinkedGuid(guid);
            }

            // state 2
            if (goInfo->capturePoint.worldState2)
            {
                WorldState const* _state = NULL;
                if (_state  = GetWorldState(goInfo->capturePoint.worldState2, instanceId, WORLD_STATE_TYPE_CAPTURE_POINT))
                {
                    if (_state->GetValue() != 0)
                        DEBUG_LOG("WorldStateMgr::CreateLinkedWorldStatesIfNeed Warning - at load WorldState %u for %s current value %u not equal default %u!", 
                            goInfo->capturePoint.worldState2,
                            guid.GetString().c_str(),
                            _state->GetValue(),
                            0
                        );
                }
                else
                     _state = CreateWorldState(goInfo->capturePoint.worldState2, instanceId, 0);

                const_cast<WorldState*>(_state)->SetLinkedGuid(guid);
            }

            // state 3
            if (goInfo->capturePoint.worldState3)
            {
                WorldState const* _state = NULL;
                if (_state  = GetWorldState(goInfo->capturePoint.worldState3, instanceId, WORLD_STATE_TYPE_CAPTURE_POINT))
                {
                    if (_state->GetValue() != 0)
                        DEBUG_LOG("WorldStateMgr::CreateLinkedWorldStatesIfNeed Warning - at load WorldState %u for %s current value %u not equal default %u!", 
                            goInfo->capturePoint.worldState3,
                            guid.GetString().c_str(),
                            _state->GetValue(),
                            0
                        );
                }
                else
                    _state = CreateWorldState(goInfo->capturePoint.worldState3, instanceId, goInfo->capturePoint.neutralPercent);

                const_cast<WorldState*>(_state)->SetLinkedGuid(guid);
            }

            break;
        }
        case HIGHGUID_UNIT:
        case HIGHGUID_VEHICLE:
        case HIGHGUID_PET:
        case HIGHGUID_PLAYER:
        default:
            sLog.outError("WorldStateMgr::CreateLinkedWorldStatesIfNeed try create linked WorldStates for %s, but currently this object type not supported!", guid.GetString().c_str());
        break;
    }
}

void WorldStateMgr::SaveToDB()
{
    CharacterDatabase.BeginTransaction();
    for (WorldStateMap::iterator iter = m_worldState.begin(); iter != m_worldState.end(); ++iter)
    {
        if (!iter->second.HasFlag(WORLD_STATE_FLAG_SAVED))
            Save(&iter->second);
    }
    CharacterDatabase.CommitTransaction();
}

void WorldStateMgr::Save(WorldState const* state)
{
    ReadGuard guard(GetLock());
    static SqlStatementID wsDel;

    SqlStatement stmt = CharacterDatabase.CreateStatement(wsDel, "DELETE FROM `worldstate_data` WHERE `state_id` = ? AND `instance` = ?");
    stmt.PExecute(state->GetId(), state->GetInstance());

    if (!state->HasFlag(WORLD_STATE_FLAG_DELETED))
    {
        static SqlStatementID wsSave;
        SqlStatement stmt1 = CharacterDatabase.CreateStatement(wsSave, "INSERT INTO `worldstate_data` (`state_id`, `instance`, `type`, `condition`, `flags`, `value`, `renewtime`) VALUES (?,?,?,?,?,?,?)");

        const_cast<WorldState*>(state)->AddFlag(WORLD_STATE_FLAG_SAVED);

        stmt1.addUInt32(state->GetId());
        stmt1.addUInt32(state->GetInstance());
        stmt1.addUInt32(state->GetType());
        stmt1.addUInt32(state->GetCondition());
        stmt1.addUInt32(state->GetFlags());
        stmt1.addUInt32(state->GetValue());
        stmt1.addUInt64(state->GetRenewTime());
        stmt1.Execute();
    }
}

void WorldStateMgr::SaveToTemplate(WorldStateType type, uint32 stateId, uint32 value, uint32 data)
{

    // Store the state data
    uint32 flags = (1 << WORLD_STATE_FLAG_ACTIVE);
    m_worldStateTemplates.insert(WorldStateTemplateMap::value_type(stateId, WorldStateTemplate(stateId, type, data, flags, value, 0)));

    // Store the state data
    static SqlStatementID wstSave;
    SqlStatement stmt = WorldDatabase.CreateStatement(wstSave, "INSERT IGNORE INTO `worldstate_template` (`state_id`, `type`, `condition`, `flags`, `default`, `comment`) VALUES (?,?,?,?,?,?)");
    stmt.addUInt32(stateId);
    stmt.addUInt32(type);
    stmt.addUInt32(data);
    stmt.addUInt32(flags);
    stmt.addUInt32(value);
    stmt.addString("added by tempfix");
    stmt.Execute();

}

void WorldStateMgr::DeleteWorldState(WorldState* state)
{
    static SqlStatementID wsDel;

    SqlStatement stmt = CharacterDatabase.CreateStatement(wsDel, "DELETE FROM `worldstate_data` WHERE `state_id` = ? AND `instance` = ?");
    stmt.PExecute(state->GetId(), state->GetInstance());

    WriteGuard guard(GetLock());
    for (WorldStateMap::iterator iter = m_worldState.begin(); iter != m_worldState.end();)
    {
        if (&iter->second == state)
            m_worldState.erase(iter);
        else
            ++iter;
    }
}

WorldStateTemplate const* WorldStateMgr::FindTemplate(uint32 stateId, uint32 type, uint32 condition)
{
    ReadGuard guard(GetLock());
    WorldStateTemplateBounds bounds = m_worldStateTemplates.equal_range(stateId);

    if (bounds.first == bounds.second)
        return NULL;

    if (stateId != 0)
        return &bounds.first->second;

    // for custom States (Id == 0)
    for (WorldStateTemplateMap::const_iterator iter = bounds.first; iter != bounds.second; ++iter)
    {
        if (iter->second.m_stateId == stateId &&
            iter->second.m_stateType == type &&
            iter->second.m_condition == condition)
            return &iter->second;
    }
    return NULL;
};

void WorldStateMgr::MapUpdate(Map* map)
{
    if (!map)
        return;

    for (WorldStateMap::iterator itr = m_worldState.begin(); itr != m_worldState.end(); ++itr)
    {
        WorldState* state = &itr->second;

        if (!state || !IsFitToCondition(map, state))
            continue;

        switch (state->GetType())
        {
            case WORLD_STATE_TYPE_CAPTURE_POINT:
            {
                if (state->GetTemplate() && state->GetTemplate()->m_linkedId)
                    continue;

                GameObject* go = map->GetGameObject(state->GetLinkedGuid());

                if (!go || go->GetGOInfo()->type != GAMEOBJECT_TYPE_CAPTURE_POINT)
                    continue;

                // search for players in radius
                std::list<Player*> pointPlayers;
                MaNGOS::AnyPlayerInObjectRangeCheck u_check(go, go->GetGOInfo()->capturePoint.radius);
                MaNGOS::PlayerListSearcher<MaNGOS::AnyPlayerInObjectRangeCheck> checker(pointPlayers, u_check);
                Cell::VisitWorldObjects(go, checker, go->GetGOInfo()->capturePoint.radius);

                GuidSet currentGuids;
                GuidSet differenceGuids;

                for (std::list<Player*>::iterator itr = pointPlayers.begin(); itr != pointPlayers.end(); ++itr)
                {
                    if ((*itr) && (*itr)->IsInWorld() && (*itr)->IsWorldPvPActive()) 
                    {
                        if (!state->HasClient(*itr))
                        {
                            state->AddClient(*itr);
                            // send WS activate
                        }
                        currentGuids.insert((*itr)->GetObjectGuid());
                    }
                }
                std::set_difference(state->GetClients().begin(),state->GetClients().end(), currentGuids.begin(),currentGuids.end(),std::inserter(differenceGuids,differenceGuids.end()));
                // work with send lists here
                for (GuidSet::iterator itr = differenceGuids.begin(); itr != differenceGuids.end(); ++itr)
                {
                    if (currentGuids.find(*itr) == currentGuids.end())
                    {
                        state->RemoveClient(*itr);
                        // send WS deactivate
                    }
                }
                break;
            }
            case WORLD_STATE_TYPE_MAP:
            case WORLD_STATE_TYPE_BATTLEGROUND:
            {
                for (GuidSet::iterator itr = state->GetClients().begin(); itr != state->GetClients().end();)
                {
                    if (!map->GetPlayer(*itr))
                    {
                        state->RemoveClient(*itr);
                    }
                    else
                        ++itr;
                }
                break;
            }
            case WORLD_STATE_TYPE_ZONE:
            case WORLD_STATE_TYPE_AREA:
            {
                for (GuidSet::iterator itr = state->GetClients().begin(); itr != state->GetClients().end();)
                {
                    Player* player = map->GetPlayer(*itr);
                    if (!player || !player->IsInWorld())
                    {
                        state->RemoveClient(*itr);
                    }
                    else
                    {
                        uint32 zone, area;
                        player->GetZoneAndAreaId(zone, area);
                        if (state->GetType() == WORLD_STATE_TYPE_ZONE && state->GetCondition() != zone)
                        {
                            // send state clean here
                            state->RemoveClient(*itr);
                        }
                        else if (state->GetType() == WORLD_STATE_TYPE_AREA && state->GetCondition() != area)
                        {
                            // send state clean here
                            state->RemoveClient(*itr);
                        }
                        else
                            ++itr;
                    }
                }
                break;
            }
            case WORLD_STATE_TYPE_EVENT:
            case WORLD_STATE_TYPE_BGWEEKEND:
            case WORLD_STATE_TYPE_CUSTOM:
            case WORLD_STATE_TYPE_WORLD:
            case WORLD_STATE_TYPE_WORLD_UNCOMMON:
            default:
                break;
        }
    }
}

WorldStateSet WorldStateMgr::GetWorldStatesFor(Player* player, uint32 flags)
{
    WorldStateSet statesSet;
    statesSet.clear();

    bool bFull = player ? !player->IsInWorld() : true;

    ReadGuard guard(GetLock());
    for (WorldStateMap::iterator itr = m_worldState.begin(); itr != m_worldState.end(); ++itr)
    {
        if (itr->second.GetFlags() & flags)
            if (bFull || IsFitToCondition(player, &itr->second))
                statesSet.insert(&itr->second);
    }
    return statesSet;
};

WorldStateSet WorldStateMgr::GetUpdatedWorldStatesFor(Player* player, time_t updateTime)
{
    WorldStateSet statesSet;
    statesSet.clear();

    ReadGuard guard(GetLock());
    for (WorldStateMap::iterator itr = m_worldState.begin(); itr != m_worldState.end(); ++itr)
    {
            if (itr->second.HasFlag(WORLD_STATE_FLAG_ACTIVE) && 
                itr->second.GetRenewTime() >= updateTime &&
                itr->second.GetRenewTime() != time(NULL) &&
                IsFitToCondition(player, &itr->second))
                {
                    statesSet.insert(&itr->second);

                    // Always send Linked worldstate with own chains
                    if (itr->second.GetTemplate() && itr->second.GetTemplate()->m_linkedId)
                        if (WorldState* state = const_cast<WorldState*>(GetWorldState(itr->second.GetTemplate()->m_linkedId, player->GetInstanceId())))
                            statesSet.insert(state);
                }
    }
    return statesSet;
};

bool WorldStateMgr::IsFitToCondition(Player* player, WorldState const* state)
{
    if (!player || !state)
        return false;

    switch (state->GetType())
    {
        case WORLD_STATE_TYPE_WORLD:
        case WORLD_STATE_TYPE_EVENT:
            return true;

        case WORLD_STATE_TYPE_BGWEEKEND:
        {
            if (player->IsPvP())
                return true;
            break;
        }
        case WORLD_STATE_TYPE_MAP:
        case WORLD_STATE_TYPE_BATTLEGROUND:
        {
            if (player->GetMapId() == state->GetCondition() && player->GetInstanceId() == state->GetInstance())
                return true;
//            else if (player->GetMapId() != state->GetCondition() && state->HasFlag(WORLD_STATE_FLAG_INITIAL_STATE) && state->GetInstance() == 0)
//                return true;
            break;
        }
        case WORLD_STATE_TYPE_ZONE:
        {
            if (player->GetZoneId() == state->GetCondition() && player->GetInstanceId() == state->GetInstance())
                return true;
            break;
        }
        case WORLD_STATE_TYPE_AREA:
        {
            if (player->GetAreaId() == state->GetCondition() && player->GetInstanceId() == state->GetInstance())
                return true;
            break;
        }
        case WORLD_STATE_TYPE_CAPTURE_POINT:
        {
            // temporary
            return true;
            break;
        }
        case WORLD_STATE_TYPE_CUSTOM:
        {
            if (!state->GetCondition())
                return true;
            else if (
            (player->GetMapId() == state->GetCondition() ||
            player->GetAreaId() == state->GetCondition() || 
            player->GetZoneId() == state->GetCondition()) &&
            player->GetInstanceId() == state->GetInstance())
                return true;
            break;
        }
        default:
            break;
    }
    return false;
};

bool WorldStateMgr::IsFitToCondition(Map* map, WorldState const* state)
{
    if (!map || !state)
        return false;
    return IsFitToCondition(map->GetId(), map->GetInstanceId(), 0, 0, state);
}

bool WorldStateMgr::IsFitToCondition(uint32 mapId, uint32 instanceId, uint32 zoneId, uint32 areaId, WorldState const* state)
{
    if (!state)
        return false;

    switch (state->GetType())
    {
        case WORLD_STATE_TYPE_WORLD:
        case WORLD_STATE_TYPE_EVENT:
        case WORLD_STATE_TYPE_BGWEEKEND:
            return true;
        case WORLD_STATE_TYPE_MAP:
        case WORLD_STATE_TYPE_BATTLEGROUND:
        {
            if (mapId == state->GetCondition() && instanceId == state->GetInstance())
                return true;
            break;
        }
        case WORLD_STATE_TYPE_ZONE:
        {
            if (zoneId == 0)
                return false;

            if (zoneId == state->GetCondition() && instanceId == state->GetInstance())
                return true;
            break;
        }
        case WORLD_STATE_TYPE_AREA:
        {
            if (areaId == 0)
                return false;

            if (areaId == state->GetCondition() && instanceId == state->GetInstance())
                return true;
            break;
        }
        case WORLD_STATE_TYPE_CAPTURE_POINT:
        {
            // temporary
            return true;
            break;
        }
        case WORLD_STATE_TYPE_CUSTOM:
        {
            if (!state->GetCondition())
                return true;
            else if (mapId == state->GetCondition() &&
                instanceId == state->GetInstance())
                return true;
            break;
        }
        default:
            break;
    }
    return false;
};

uint32 WorldStateMgr::GetWorldStateValue(uint32 stateId)
{
    ReadGuard guard(GetLock());
    WorldStateBounds bounds = m_worldState.equal_range(stateId);

    if (bounds.first == bounds.second)
        return UINT32_MAX;

    return bounds.first->second.GetValue();
};

uint32 WorldStateMgr::GetWorldStateValueFor(Player* player, uint32 stateId)
{
    if (!player)
        return UINT32_MAX;

    ReadGuard guard(GetLock());
    WorldStateBounds bounds = m_worldState.equal_range(stateId);
    if (bounds.first == bounds.second)
        return UINT32_MAX;

    for (WorldStateMap::const_iterator iter = bounds.first; iter != bounds.second; ++iter)
    {
        if (IsFitToCondition(player, &iter->second))
            return iter->second.GetValue();
    }
    return UINT32_MAX;
};

void WorldStateMgr::SetWorldStateValueFor(Player* player, uint32 stateId, uint32 value)
{
    if (!player)
        return;

    WorldStateBounds bounds = m_worldState.equal_range(stateId);
    if (bounds.first != bounds.second)
    {
        for (WorldStateMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
        {
            if (IsFitToCondition(player, &itr->second))
            {
                if ((&itr->second)->GetValue() != value)
                    const_cast<WorldState*>(&itr->second)->SetValue(value);
                return;
            }
        }
    }
    else
        CreateWorldState(stateId, player->GetInstanceId(), value);
};

void WorldStateMgr::SetWorldStateValueFor(Map* map, uint32 stateId, uint32 value)
{
    if (!map)
        return;

    WorldStateBounds bounds = m_worldState.equal_range(stateId);
    if (bounds.first != bounds.second)
    {
        for (WorldStateMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
        {
            if (IsFitToCondition(map, &itr->second))
            {
                if ((&itr->second)->GetValue() != value)
                    const_cast<WorldState*>(&itr->second)->SetValue(value);
                return;
            }
        }
    }
    else
        CreateWorldState(stateId, map->GetInstanceId(), value);
};

WorldState const* WorldStateMgr::CreateWorldState(uint32 stateId, uint32 instanceId, uint32 value)
{
    // Don't create special states as custom!
    if (stateId == 0)
        return NULL;
    WorldStateTemplate const* tmpl = FindTemplate(stateId);
    return CreateWorldState(tmpl, instanceId, value);
};

WorldState const* WorldStateMgr::CreateWorldState(WorldStateTemplate const* tmpl, uint32 instanceId, uint32 value)
{
    if (!tmpl)
        return NULL;

    if (tmpl->IsGlobal() && instanceId > 0)
    {
        sLog.outError("WorldStateMgr::CreateWorldState tru create GLOBAL state %u  with instance Id %u.",tmpl->m_stateId, instanceId);
        return NULL;
    }

    if (WorldState const* _state  = GetWorldState(tmpl->m_stateId, instanceId, tmpl->m_stateType))
    {
        DEBUG_LOG("WorldStateMgr::CreateWorldState tru create  state %u  instance %u type %u (value %u) but state exists (value %u).",
            tmpl->m_stateId, instanceId, tmpl->m_stateType, value, _state->GetValue());
        return _state;
    }

    // Store the state data
    {
        WriteGuard guard(GetLock());
        m_worldState.insert(WorldStateMap::value_type(tmpl->m_stateId, WorldState(tmpl, instanceId)));
    }
    WorldState* _state  = const_cast<WorldState*>(GetWorldState(tmpl->m_stateId, instanceId));

    if (value != UINT32_MAX)
        _state->SetValue(value);
    else
        _state->RemoveFlag(WORLD_STATE_FLAG_SAVED);

    if (!tmpl->HasFlag(WORLD_STATE_FLAG_PASSIVE_AT_CREATE))
        _state->AddFlag(WORLD_STATE_FLAG_ACTIVE);

    DEBUG_LOG("WorldStateMgr::CreateWorldState state %u instance %u created, type %u (%u) flags %u (%u) value %u (%u, %u)",
        _state->GetId(), _state->GetInstance(),
        _state->GetType(), tmpl->m_stateType,
        _state->GetFlags(), tmpl->m_flags,
        _state->GetValue(), value, tmpl->m_defaultValue
        );

    return _state;
}

WorldState const* WorldStateMgr::GetWorldState(uint32 stateId, uint32 instanceId, uint32 type)
{
    ReadGuard guard(GetLock());
    WorldStateBounds bounds = m_worldState.equal_range(stateId);
    if (bounds.first == bounds.second)
        return NULL;

    for (WorldStateMap::const_iterator iter = bounds.first; iter != bounds.second; ++iter)
    {
        if (iter->second.GetInstance() == instanceId &&
            ((type == WORLD_STATE_TYPE_MAX) || (type == iter->second.GetType())))
            return &iter->second;
    }
    return NULL;
};

void WorldStateMgr::SendWorldState(Player* player, WorldState const* state)
{
    if (!player || !player->IsInWorld() || !state)
        return;

    if (state->GetId() == 0 || state->HasFlag(WORLD_STATE_FLAG_CUSTOM_FORMAT))
    {
        sLog.outError("WorldStateMgr::SendWorldState try send custom world state %u by standart way!");
        return;
    }

    WorldPacket data(SMSG_UPDATE_WORLD_STATE, 4+4);
    data << uint32(state->GetId());
    data << uint32(state->GetValue());

    player->GetSession()->SendPacket(&data);
}

void WorldStateMgr::CreateInstanceState(Map* map)
{
    if (!map)
        return;

    WorldStateSet statesSet = GetInstanceStates(map, (1 << WORLD_STATE_FLAG_INITIAL_STATE));

    if (!statesSet.empty())
        return;

    for (WorldStateTemplateMap::const_iterator itr = m_worldStateTemplates.begin(); itr != m_worldStateTemplates.end(); ++itr)
    {
        if (itr->second.m_flags & (1 << WORLD_STATE_FLAG_INITIAL_STATE) &&
            itr->second.m_stateType == WORLD_STATE_TYPE_MAP &&
            itr->second.m_condition == map->GetId())
        {
            if (GetWorldState(itr->second.m_stateId, map->GetInstanceId()))
                continue;

            CreateWorldState(&itr->second, map->GetInstanceId());
        }
    }
}

void WorldStateMgr::CreateZoneAreaStateIfNeed(Player* player, uint32 zone, uint32 area)
{
    if (!player)
        return;

    // temporary - not create zone/area states for instances!
    if (player->GetInstanceId() != 0)
        return;

    for (WorldStateTemplateMap::const_iterator itr = m_worldStateTemplates.begin(); itr != m_worldStateTemplates.end(); ++itr)
    {
        if (itr->second.HasFlag(WORLD_STATE_FLAG_ACTIVE))
        {
            if (itr->second.m_stateType == WORLD_STATE_TYPE_ZONE && itr->second.m_condition == zone)
            {
                if (GetWorldState(itr->second.m_stateId, player->GetInstanceId()))
                    continue;
                CreateWorldState(&itr->second, player->GetInstanceId());
            }
            else if (itr->second.m_stateType == WORLD_STATE_TYPE_AREA && itr->second.m_condition == area)
            {
                if (GetWorldState(itr->second.m_stateId, player->GetInstanceId()))
                    continue;
                CreateWorldState(&itr->second, player->GetInstanceId());
            }
        }
    }
}

void WorldStateMgr::DeleteInstanceState(uint32 mapId, uint32 instanceId)
{
    // Not delete states for 0 instance by standart way! only cleanup in ::Update
    if (instanceId == 0)
        return;

    MapEntry const* targetMapEntry = sMapStore.LookupEntry(mapId);

    if (!targetMapEntry || targetMapEntry->IsContinent() || !targetMapEntry->Instanceable())
    {
        sLog.outError("WorldStateMgr::DeleteInstanceState map %u not exists or not instanceable!", mapId);
        return;
    }

    WorldStateSet statesSet = GetInstanceStates(mapId,instanceId);
    if (!statesSet.empty())
        return;

    for (WorldStateSet::const_iterator itr = statesSet.begin(); itr != statesSet.end(); ++itr)
    {
        DeleteWorldState(*itr);
    }
}

WorldStateSet WorldStateMgr::GetInstanceStates(Map* map, uint32 flags, bool full)
{
    WorldStateSet statesSet;
    statesSet.clear();

    if (!map)
        return statesSet;

    return GetInstanceStates(map->GetId(), map->GetInstanceId(), flags, full);
}

WorldStateSet WorldStateMgr::GetInstanceStates(uint32 mapId, uint32 instanceId, uint32 flags, bool full)
{
    WorldStateSet statesSet;
    statesSet.clear();

    ReadGuard guard(GetLock());
    for (WorldStateMap::iterator itr = m_worldState.begin(); itr != m_worldState.end(); ++itr)
    {
        if (!flags || itr->second.GetFlags() & flags)
        {
            if (itr->second.GetType() == WORLD_STATE_TYPE_MAP &&
                itr->second.GetCondition() == mapId &&
                itr->second.GetInstance() == instanceId)
                statesSet.insert(&itr->second);
            else if (full)
            {
                Map* map = sMapMgr.FindMap(mapId, instanceId);
                if (IsFitToCondition(map, &itr->second))
                    statesSet.insert(&itr->second);
            }
        }
    }
    return statesSet;
}

WorldStateSet WorldStateMgr::GetInitWorldStates(uint32 mapId, uint32 instanceId, uint32 zoneId, uint32 areaId)
{
    WorldStateSet statesSet;
    statesSet.clear();

    ReadGuard guard(GetLock());
    for (WorldStateMap::iterator itr = m_worldState.begin(); itr != m_worldState.end(); ++itr)
    {
        if ((itr->second.HasFlag(WORLD_STATE_FLAG_INITIAL_STATE) ||
            itr->second.HasFlag(WORLD_STATE_FLAG_ACTIVE)) &&
            IsFitToCondition(mapId, instanceId, zoneId, areaId, &itr->second))
            statesSet.insert(&itr->second);
    }
    return statesSet;
};

void WorldStateMgr::FillInitialWorldState(uint32 stateId, uint32 value, WorldStateType type, uint32 data)
{
    // This method usable only temporary! Need move all states, for which his used, in DB!
    switch (type)
    {
        case WORLD_STATE_TYPE_BGWEEKEND:
        case WORLD_STATE_TYPE_CUSTOM:
        case WORLD_STATE_TYPE_WORLD:
        case WORLD_STATE_TYPE_EVENT:
        case WORLD_STATE_TYPE_MAP:
        case WORLD_STATE_TYPE_ZONE:
        case WORLD_STATE_TYPE_AREA:
        case WORLD_STATE_TYPE_BATTLEGROUND:
        case WORLD_STATE_TYPE_CAPTURE_POINT:
        case WORLD_STATE_TYPE_WORLD_UNCOMMON:
        default:
            break;
    }

    WorldStateBounds bounds = m_worldState.equal_range(stateId);
    if (bounds.first != bounds.second)
    {
        for (WorldStateMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
        {
            if ((&itr->second)->GetValue() != value)
            {
                //const_cast<WorldState*>(&itr->second)->SetValue(value);
                DEBUG_LOG("WorldStateMgr::FillInitialWorldState try fill state (type %u) %u value %u, but state (tmpl type %u) already has value %u",
                    type,
                    stateId,
                    value,
                    (&itr->second)->GetType(),
                    (&itr->second)->GetValue()
                    );
            }
            return;
        }
    }

    // Attention! dangerous part below!
    SaveToTemplate(type, stateId, value, data);

    WorldState const* _state = CreateWorldState(stateId, 0, value);

    DETAIL_LOG("WorldStateMgr::FillInitialWorldState filled state (type %u) %u flags %u value %u",
        _state->GetType(),
        _state->GetId(),
        _state->GetFlags(),
        _state->GetValue()
        );
}

void WorldStateMgr::RemoveWorldStateFor(Player* player, uint32 stateId)
{
    if (!player || !player->IsInWorld())
        return;

    WorldState const* state = GetWorldState(stateId,player->GetInstanceId());

    if (state)
        const_cast<WorldState*>(state)->RemoveClient(player);

    player->_SendUpdateWorldState(stateId, 0);
}

void WorldStateMgr::RemovePendingWorldStateFor(Player* player, uint32 mapId, uint32 instanceId, uint32 zoneId, uint32 areaId)
{
}

void WorldStateMgr::SendPendingWorldStateFor(Player* player, uint32 mapId, uint32 instanceId, uint32 zoneId, uint32 areaId)
{
}

bool WorldState::IsExpired() const
{
    return time(NULL) > time_t(m_renewTime + sWorld.getConfig(CONFIG_UINT32_WORLD_STATE_EXPIRETIME));
};

void WorldState::AddClient(Player* player)
{
    if (player)
        AddClient(player->GetObjectGuid());
};

bool WorldState::HasClient(Player* player) const
{
    return player ? HasClient(player->GetObjectGuid()) : false;
};

void WorldState::RemoveClient(Player* player)
{
    if (player)
        RemoveClient(player->GetObjectGuid());
};
