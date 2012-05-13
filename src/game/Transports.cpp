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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"

#include "Transports.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Path.h"

#include "WorldPacket.h"
#include "DBCStores.h"
#include "ProgressBar.h"
#include "ScriptMgr.h"

Transport::Transport() : GameObject()
{
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
}

Transport::~Transport()
{
    sLog.outError("Transport::~Transport() Transport %s %s destroyed", 
    GetObjectGuid().GetString().c_str(),
    GetGOInfo()->name
    );
}

bool Transport::Create(uint32 entry)
{
    GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(entry);
    if (!goinfo)
    {
        sLog.outErrorDb("Transport::Create Transport not created: entry in `gameobject_template` not found, entry: %u", entry);
        return false;
    }

    if (goinfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
    {
        sLog.outErrorDb("Transport ID:%u, will not be loaded, gameobject_template type wrong", entry);
        return false;
    }

    // in MO_TRANSPORT case entry is always equal to low guid
    Object::_Create(ObjectGuid(HIGHGUID_MO_TRANSPORT, entry));

    std::set<uint32> mapsUsed;
    if(!GenerateWaypoints(goinfo->moTransport.taxiPathId, mapsUsed))// skip transports with empty waypoints list
    {
        sLog.outErrorDb("Transport::Create Transport (path id %u) path size = 0. Transport ignored, check DBC files or transport GO data0 field.",goinfo->moTransport.taxiPathId);
        return false;
    }

    float x, y, z;
    uint32 mapid;
    mapid = m_WayPoints[0].mapid;
    x = m_WayPoints[0].x;
    y = m_WayPoints[0].y;
    z = m_WayPoints[0].z;

    Relocate(x,y,z);

    if (!IsPositionValid())
    {
        sLog.outError("Transport::Create Transport (GUID: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)", entry,x,y);
        return false;
    }

    m_goInfo = goinfo;

    SetObjectScale(goinfo->size);

    SetUInt32Value(GAMEOBJECT_FACTION, goinfo->faction);

    //SetUInt32Value(GAMEOBJECT_FLAGS, goinfo->flags);
    SetUInt32Value(GAMEOBJECT_FLAGS, (GO_FLAG_TRANSPORT | GO_FLAG_NODESPAWN));

    SetEntry(goinfo->id);
    SetDisplayId(goinfo->displayId);

    if (goinfo->moTransport.defaultState == 1)
        SetGoState(GO_STATE_ACTIVE);
    else
        SetGoState(GO_STATE_READY);

    SetGoType(GameobjectTypes(goinfo->type));
    SetGoArtKit(0);
    SetGoAnimProgress(GO_ANIMPROGRESS_DEFAULT);

    // low part always 0, dynamicHighValue is some kind of progression (not implemented)
    SetUInt16Value(GAMEOBJECT_DYNAMIC, 0, 0);
    SetUInt16Value(GAMEOBJECT_DYNAMIC, 1, 0);

    DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::Create %s (%s) created, %s, %u waypoints, current coords: map %u, %f, %f, %f", 
        GetObjectGuid().GetString().c_str(),
        GetName(),
        isActiveObject() ? "active" : "passive",
        m_WayPoints.size(),
        mapid,
        GetPositionX(),
        GetPositionY(),
        GetPositionZ());
    return true;
}

struct keyFrame
{
    explicit keyFrame(TaxiPathNodeEntry const& _node) : node(&_node),
        distSinceStop(-1.0f), distUntilStop(-1.0f), distFromPrev(-1.0f), tFrom(0.0f), tTo(0.0f)
    {
    }

    TaxiPathNodeEntry const* node;

    float distSinceStop;
    float distUntilStop;
    float distFromPrev;
    float tFrom, tTo;
};

bool Transport::GenerateWaypoints(uint32 pathid, std::set<uint32> &mapids)
{
    if (pathid >= sTaxiPathNodesByPath.size())
        return false;

    TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathid];

    std::vector<keyFrame> keyFrames;
    int mapChange = 0;
    mapids.clear();
    for (size_t i = 1; i < path.size() - 1; ++i)
    {
        if (mapChange == 0)
        {
            TaxiPathNodeEntry const& node_i = path[i];
            if (node_i.mapid == path[i+1].mapid)
            {
                keyFrame k(node_i);
                keyFrames.push_back(k);
                mapids.insert(k.node->mapid);
            }
            else
            {
                mapChange = 1;
            }
        }
        else
        {
            --mapChange;
        }
    }

    int lastStop = -1;
    int firstStop = -1;

    // first cell is arrived at by teleportation :S
    keyFrames[0].distFromPrev = 0;
    if (keyFrames[0].node->actionFlag == 2)
    {
        lastStop = 0;
    }

    // find the rest of the distances between key points
    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
        if ((keyFrames[i].node->actionFlag == 1) || (keyFrames[i].node->mapid != keyFrames[i-1].node->mapid))
        {
            keyFrames[i].distFromPrev = 0;
        }
        else
        {
            keyFrames[i].distFromPrev =
                sqrt(pow(keyFrames[i].node->x - keyFrames[i - 1].node->x, 2) +
                    pow(keyFrames[i].node->y - keyFrames[i - 1].node->y, 2) +
                    pow(keyFrames[i].node->z - keyFrames[i - 1].node->z, 2));
        }
        if (keyFrames[i].node->actionFlag == 2)
        {
            // remember first stop frame
            if(firstStop == -1)
                firstStop = i;
            lastStop = i;
        }
    }

    float tmpDist = 0;
    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        int j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].node->actionFlag == 2)
            tmpDist = 0;
        else
            tmpDist += keyFrames[j].distFromPrev;
        keyFrames[j].distSinceStop = tmpDist;
    }

    for (int i = int(keyFrames.size()) - 1; i >= 0; i--)
    {
        int j = (i + (firstStop+1)) % keyFrames.size();
        tmpDist += keyFrames[(j + 1) % keyFrames.size()].distFromPrev;
        keyFrames[j].distUntilStop = tmpDist;
        if (keyFrames[j].node->actionFlag == 2)
            tmpDist = 0;
    }

    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        if (keyFrames[i].distSinceStop < (30 * 30 * 0.5f))
            keyFrames[i].tFrom = sqrt(2 * keyFrames[i].distSinceStop);
        else
            keyFrames[i].tFrom = ((keyFrames[i].distSinceStop - (30 * 30 * 0.5f)) / 30) + 30;

        if (keyFrames[i].distUntilStop < (30 * 30 * 0.5f))
            keyFrames[i].tTo = sqrt(2 * keyFrames[i].distUntilStop);
        else
            keyFrames[i].tTo = ((keyFrames[i].distUntilStop - (30 * 30 * 0.5f)) / 30) + 30;

        keyFrames[i].tFrom *= 1000;
        keyFrames[i].tTo *= 1000;
    }

    //    for (int i = 0; i < keyFrames.size(); ++i) {
    //        sLog.outString("%f, %f, %f, %f, %f, %f, %f", keyFrames[i].x, keyFrames[i].y, keyFrames[i].distUntilStop, keyFrames[i].distSinceStop, keyFrames[i].distFromPrev, keyFrames[i].tFrom, keyFrames[i].tTo);
    //    }

    // Now we're completely set up; we can move along the length of each waypoint at 100 ms intervals
    // speed = max(30, t) (remember x = 0.5s^2, and when accelerating, a = 1 unit/s^2
    int t = 0;
    bool teleport = false;
    if (keyFrames[keyFrames.size() - 1].node->mapid != keyFrames[0].node->mapid)
        teleport = true;

    WayPoint pos(keyFrames[0].node->mapid, keyFrames[0].node->x, keyFrames[0].node->y, keyFrames[0].node->z, teleport,
        keyFrames[0].node->arrivalEventID, keyFrames[0].node->departureEventID);
    m_WayPoints[0] = pos;
    t += keyFrames[0].node->delay * 1000;

    uint32 cM = keyFrames[0].node->mapid;
    for (size_t i = 0; i < keyFrames.size() - 1; ++i)
    {
        float d = 0;
        float tFrom = keyFrames[i].tFrom;
        float tTo = keyFrames[i].tTo;

        // keep the generation of all these points; we use only a few now, but may need the others later
        if (((d < keyFrames[i + 1].distFromPrev) && (tTo > 0)))
        {
            while ((d < keyFrames[i + 1].distFromPrev) && (tTo > 0))
            {
                tFrom += 100;
                tTo -= 100;

                if (d > 0)
                {
                    float newX, newY, newZ;
                    newX = keyFrames[i].node->x + (keyFrames[i + 1].node->x - keyFrames[i].node->x) * d / keyFrames[i + 1].distFromPrev;
                    newY = keyFrames[i].node->y + (keyFrames[i + 1].node->y - keyFrames[i].node->y) * d / keyFrames[i + 1].distFromPrev;
                    newZ = keyFrames[i].node->z + (keyFrames[i + 1].node->z - keyFrames[i].node->z) * d / keyFrames[i + 1].distFromPrev;

                    bool teleport = false;
                    if (keyFrames[i].node->mapid != cM)
                    {
                        teleport = true;
                        cM = keyFrames[i].node->mapid;
                    }

                    //                    sLog.outString("T: %d, D: %f, x: %f, y: %f, z: %f", t, d, newX, newY, newZ);
                    WayPoint pos(keyFrames[i].node->mapid, newX, newY, newZ, teleport);
                    if (teleport)
                        m_WayPoints[t] = pos;
                }

                if (tFrom < tTo)                            // caught in tFrom dock's "gravitational pull"
                {
                    if (tFrom <= 30000)
                    {
                        d = 0.5f * (tFrom / 1000) * (tFrom / 1000);
                    }
                    else
                    {
                        d = 0.5f * 30 * 30 + 30 * ((tFrom - 30000) / 1000);
                    }
                    d = d - keyFrames[i].distSinceStop;
                }
                else
                {
                    if (tTo <= 30000)
                    {
                        d = 0.5f * (tTo / 1000) * (tTo / 1000);
                    }
                    else
                    {
                        d = 0.5f * 30 * 30 + 30 * ((tTo - 30000) / 1000);
                    }
                    d = keyFrames[i].distUntilStop - d;
                }
                t += 100;
            }
            t -= 100;
        }

        if (keyFrames[i + 1].tFrom > keyFrames[i + 1].tTo)
            t += 100 - ((long)keyFrames[i + 1].tTo % 100);
        else
            t += (long)keyFrames[i + 1].tTo % 100;

        bool teleport = false;
        if ((keyFrames[i + 1].node->actionFlag == 1) || (keyFrames[i + 1].node->mapid != keyFrames[i].node->mapid))
        {
            teleport = true;
            cM = keyFrames[i + 1].node->mapid;
        }

        WayPoint pos(keyFrames[i + 1].node->mapid, keyFrames[i + 1].node->x, keyFrames[i + 1].node->y, keyFrames[i + 1].node->z, teleport,
            keyFrames[i + 1].node->arrivalEventID, keyFrames[i + 1].node->departureEventID);

        //        sLog.outString("T: %d, x: %f, y: %f, z: %f, t:%d", t, pos.x, pos.y, pos.z, teleport);

        //if (teleport)
        m_WayPoints[t] = pos;

        t += keyFrames[i + 1].node->delay * 1000;
        //        sLog.outString("------");
    }

    uint32 timer = t;

    //    sLog.outDetail("    Generated %lu waypoints, total time %u.", (unsigned long)m_WayPoints.size(), timer);

    m_next = m_WayPoints.begin();                           // will used in MoveToNextWayPoint for init m_curr
    MoveToNextWayPoint();                                   // m_curr -> first point
    MoveToNextWayPoint();                                   // skip first point

    m_pathTime = timer;

    m_nextNodeTime = m_curr->first;

    return true;
}

void Transport::MoveToNextWayPoint()
{
    m_curr = m_next;

    ++m_next;
    if (m_next == m_WayPoints.end())
        m_next = m_WayPoints.begin();
}

void Transport::TeleportTransport(uint32 newMapid, float x, float y, float z)
{
    Map* oldMap = GetMap();

    if (!oldMap)
        return;

    //we need to create and save new Map object with 'newMapid' because if not done -> lead to invalid Map object reference...
    //player far teleport would try to create same instance, but we need it NOW for transport...
    //correct me if I'm wrong O.o
    Map* newMap = sMapMgr.CreateMap(newMapid, this);
    /** Assertion will fail in case it will try to create instance map */
    MANGOS_ASSERT(newMap);

    DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES,"Transport::TeleportTransport %s passenger count %u", GetName(),  m_passengers.size());

    GetMap()->Remove<GameObject>(this, false);
    Relocate(x, y, z);
    newMap->Add<GameObject>(this);

    for (ObjectGuidSet::const_iterator itr = m_passengers.begin(); itr != m_passengers.end(); ++itr)
    {
        Player* plr = oldMap->GetPlayer(*itr);

        if (!plr)
            continue;

        if (plr->isDead() && !plr->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        {
            plr->ResurrectPlayer(1.0);
        }

        plr->TeleportTo(newMapid, x, y, z, GetOrientation(), TELE_TO_NOT_LEAVE_TRANSPORT);
        DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES,"Transport::TeleportTransport Player %s teleported with transport %s.",
            plr->GetObjectGuid().GetString().c_str(), GetName());

        //WorldPacket data(SMSG_811, 4);
        //data << uint32(0);
        //plr->GetSession()->SendPacket(&data);
    }

}

bool Transport::AddPassenger(Unit* passenger)
{
    if (!passenger)
        return false;

    if (m_passengers.find(passenger->GetObjectGuid()) == m_passengers.end())
    {
        DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES,"Transport::AddPassenger Player %s boarded transport %s.", 
            passenger->GetObjectGuid().GetString().c_str(), 
            GetObjectGuid().GetString().c_str());
        m_passengers.insert(passenger->GetObjectGuid());

        passenger->SetTransport(this);
        passenger->m_movementInfo.SetTransportData(GetObjectGuid(), 0.0f, 0.0f, 0.0f, 0.0f, WorldTimer::getMSTime(), -1, NULL);
    }
    return true;
}

bool Transport::RemovePassenger(Unit* passenger)
{
    if (!passenger)
        return false;

    passenger->SetTransport(NULL);
    passenger->m_movementInfo.ClearTransportData();

    if (m_passengers.find(passenger->GetObjectGuid()) == m_passengers.end())
        return false;

    m_passengers.erase(passenger->GetObjectGuid());

    DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES,"Transport::RemovePassenger Player %s removed from transport %s.",
        passenger->GetObjectGuid().GetString().c_str(), 
        GetObjectGuid().GetString().c_str());

    return true;
}

void Transport::Update(uint32 update_diff, uint32 p_time)
{

    SetUpdated(true);

    if (GetGoState() == GO_STATE_ACTIVE)
        return;

    if (m_WayPoints.size() <= 1)
        return;

    m_timer = WorldTimer::getMSTime() % GetPeriod();
    while (((m_timer - m_curr->first) % m_pathTime) > ((m_next->first - m_curr->first) % m_pathTime))
    {

        DoEventIfAny(*m_curr,true);

        MoveToNextWayPoint();

        if (m_curr == m_WayPoints.begin())
            DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::Update BEGIN moves %s", GetName());
        else
            DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::Update %s moved from map %u %f %f %f to map %u %f %f %f", GetName(),
                GetMap()->GetId(),
                GetPositionX(),
                GetPositionY(),
                GetPositionZ(),
                m_curr->second.mapid,
                m_curr->second.x,
                m_curr->second.y,
                m_curr->second.z);

        DoEventIfAny(*m_curr,false);

        // first check help in case client-server transport coordinates de-synchronization
        if (m_curr->second.mapid != GetMapId() || m_curr->second.teleport)
        {
            TeleportTransport(m_curr->second.mapid, m_curr->second.x, m_curr->second.y, m_curr->second.z);
        }
        else
        {
            GetMap()->TransportRelocation(this, m_curr->second.x, m_curr->second.y, m_curr->second.z, GetOrientation());

            for (ObjectGuidSet::const_iterator itr = m_passengers.begin(); itr != m_passengers.end();)
            {
                Player* player = GetMap()->GetPlayer(*itr);
                ++itr;
                if (!player)
                    continue;

//                float xOffset = (player->GetTransOffsetX() * cos(GetOrientation()) - player->GetTransOffsetY() * sin(GetOrientation()));
//                float yOffset = (player->GetTransOffsetY() * cos(GetOrientation()) + player->GetTransOffsetX() * sin(GetOrientation()));
//                player->SetPosition(GetPositionX() + xOffset, GetPositionY() + yOffset, GetPositionZ() + player->GetTransOffsetZ(), GetOrientation() + player->GetTransOffsetO());

//                player->SetPosition(GetPositionX(),GetPositionY(),GetPositionZ(),GetOrientation());
            }
        }
        m_nextNodeTime = m_curr->first;
    }
}

void Transport::DoEventIfAny(WayPointMap::value_type const& node, bool departure)
{
    if (uint32 eventid = departure ? node.second.departureEventID : node.second.arrivalEventID)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Taxi %s event %u of node %u of %s \"%s\") path", departure ? "departure" : "arrival", eventid, node.first, GetGuidStr().c_str(), GetName());

        if (!sScriptMgr.OnProcessEvent(eventid, this, this, departure))
            GetMap()->ScriptsStart(sEventScripts, eventid, this, this);
    }
}

void Transport::BuildStartMovePacket()
{
    SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
    SetGoState(GO_STATE_ACTIVE);
    //UpdateForMap(true);
}

void Transport::BuildStopMovePacket()
{
    RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
    SetGoState(GO_STATE_READY);
    //UpdateForMap(true);
}

Transport* Transport::Load(Map* map, uint32 entry, const std::string& name, uint32 period)
{
    MANGOS_ASSERT(map);
    Transport* transport = new Transport();

    if (!transport->Create(entry))
    {
        delete transport;
        return NULL;
    }

    transport->SetPhaseMask(PHASEMASK_ANYWHERE,false);
    transport->SetMap(map);
    transport->SetPeriod(period);
    transport->SetName(name);
    return transport;
}

uint32 Transport::GetPossibleMapByEntry(uint32 entry, bool start)
{
    GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(entry);
    if (!goinfo || goinfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        return UINT32_MAX;

    if (goinfo->moTransport.taxiPathId >= sTaxiPathNodesByPath.size())
        return UINT32_MAX;

    TaxiPathNodeList const& path = sTaxiPathNodesByPath[goinfo->moTransport.taxiPathId];
    return start ? path[0].mapid : path[path.size() - 1].mapid;
}

void Transport::UpdateForMap(bool arrival)
{
    Map::PlayerList const& pl = GetMap()->GetPlayers();

    if(pl.isEmpty())
        return;

    if (arrival)
    {
        for(Map::PlayerList::const_iterator itr = pl.begin(); itr != pl.end(); ++itr)
        {
            if (this != itr->getSource()->GetTransport())
            {
                UpdateData transData;
                BuildCreateUpdateBlockForPlayer(&transData, itr->getSource());
                WorldPacket packet;
                transData.BuildPacket(&packet);
                itr->getSource()->SendDirectMessage(&packet);
            }
        }
    }
    else
    {
        UpdateData transData;
        BuildOutOfRangeUpdateBlock(&transData);
        WorldPacket out_packet;
        transData.BuildPacket(&out_packet);

        for(Map::PlayerList::const_iterator itr = pl.begin(); itr != pl.end(); ++itr)
            if(this != itr->getSource()->GetTransport())
                itr->getSource()->SendDirectMessage(&out_packet);
    }
}
