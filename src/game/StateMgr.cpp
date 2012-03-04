/*
 * Copyright (C) 2009-2012 /dev/rsa for MaNGOS <http://getmangos.com/>
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
/* StateMgr based on idea and part of code from SilverIce (http:://github.com/SilverIce
*/
#include "ConfusedMovementGenerator.h"
#include "TargetedMovementGenerator.h"
#include "IdleMovementGenerator.h"
#include "Timer.h"
#include "StateMgr.h"
#include "Player.h"
#include "Creature.h"

static const class staticActionInfo
{
    public:
    staticActionInfo()
    {
        actionInfo[UNIT_ACTION_IDLE](UNIT_ACTION_PRIORITY_IDLE);
        actionInfo[UNIT_ACTION_DOWAYPOINTS](UNIT_ACTION_PRIORITY_DOWAYPOINTS);
        actionInfo[UNIT_ACTION_CHASE](UNIT_ACTION_PRIORITY_CHASE);
        actionInfo[UNIT_ACTION_CONFUSED](UNIT_ACTION_PRIORITY_CONFUSED);
        actionInfo[UNIT_ACTION_FEARED]( UNIT_ACTION_PRIORITY_FEARED);
        actionInfo[UNIT_ACTION_STUN](UNIT_ACTION_PRIORITY_STUN);
        actionInfo[UNIT_ACTION_ROOT](UNIT_ACTION_PRIORITY_ROOT);
        actionInfo[UNIT_ACTION_EFFECT](UNIT_ACTION_PRIORITY_EFFECT,ACTION_TYPE_NONRESTOREABLE);
    }

    const StaticActionInfo& operator[](UnitActionId i) const { return actionInfo[i];}

    private:
    StaticActionInfo actionInfo[UNIT_ACTION_END];
} staticActionInfo;

// derived from IdleState_ to not write new GetMovementGeneratorType, Update
class StunnedState : public IdleMovementGenerator
{
public:

    const char* Name() const { return "<Stunned>"; }
    void Interrupt(Unit &u) {Finalize(u);}
    void Reset(Unit &u) {Initialize(u);}
    void Initialize(Unit &u)
    {
        Unit* const target = &u;
        target->addUnitState(UNIT_STAT_STUNNED);
        target->SetTargetGuid(ObjectGuid());

        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);

        // Creature specific
        if (target->GetTypeId() != TYPEID_PLAYER)
            target->StopMoving();
        else
        {
            ((Player*)target)->m_movementInfo.SetMovementFlags(MOVEFLAG_NONE);
            target->SetStandState(UNIT_STAND_STATE_STAND);// in 1.5 client
        }

        WorldPacket data(SMSG_FORCE_MOVE_ROOT, 8);
        data << target->GetPackGUID();
        data << uint32(0);
        target->SendMessageToSet(&data, true);
    }

    void Finalize(Unit &u)
    {
        Unit* const target = &u;

        target->clearUnitState(UNIT_STAT_STUNNED);
        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);

        WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 8+4);
        data << target->GetPackGUID();
        data << uint32(0);
        target->SendMessageToSet(&data, true);
    }

};

class RootState : public IdleMovementGenerator
{
public:

    const char* Name() const { return "<Rooted>"; }
    void Interrupt(Unit &u) {Finalize(u);}
    void Reset(Unit &u) {Initialize(u);}
    void Initialize(Unit &u)
    {
        Unit* const target = &u;
        target->addUnitState(UNIT_STAT_ROOT);
        target->SetTargetGuid(ObjectGuid());

        //Save last orientation
        if(target->getVictim())
            target->SetOrientation(target->GetAngle(target->getVictim()));

        if(target->GetTypeId() == TYPEID_PLAYER)
        {
            WorldPacket data(SMSG_FORCE_MOVE_ROOT, 10);
            data << target->GetPackGUID();
            data << (uint32)2;
            target->SendMessageToSet(&data, true);

            //Clear unit movement flags
            ((Player*)target)->m_movementInfo.SetMovementFlags(MOVEFLAG_NONE);
        }
        else
            target->StopMoving();
    }

    void Finalize(Unit &u)
    {
        Unit* const target = &u;
        target->clearUnitState(UNIT_STAT_ROOT);
        if(target->GetTypeId() == TYPEID_PLAYER)
        {
            WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 10);
            data << target->GetPackGUID();
            data << (uint32)2;
            target->SendMessageToSet(&data, true);
        }
    }
};

UnitActionPtr UnitStateMgr::CreateStandartState(UnitActionId stateId)
{
    UnitActionPtr state = UnitActionPtr(NULL);
    switch (stateId)
    {
        case UNIT_ACTION_CONFUSED:
            break;
        case UNIT_ACTION_STUN:
            state = UnitActionPtr(new StunnedState());
            break;
        case UNIT_ACTION_ROOT:
            state = UnitActionPtr(new RootState());
            break;
        case UNIT_ACTION_FEARED:
            break;
        case UNIT_ACTION_CHASE:
            break;
        default:
            break;
    }

    if (!state)
        state = UnitActionPtr(new IdleMovementGenerator());

    return state;
}

UnitStateMgr::UnitStateMgr(Unit* owner) : m_owner(owner)
{
    InitDefaults();
}

UnitStateMgr::~UnitStateMgr()
{
}

void UnitStateMgr::InitDefaults()
{
    m_oldAction = NULL;
    m_actions.clear();
    PushAction(UNIT_ACTION_IDLE,UNIT_ACTION_PRIORITY_NONE);
}

void UnitStateMgr::Update(uint32 diff)
{
    ActionInfo* state = CurrentState();

    if (!m_oldAction || m_oldAction != state)
    {
        state->Initialize(this);
        m_oldAction = state;
    }

    if (!state->Update(this, diff))
        DropAction(state->priority);
}

void UnitStateMgr::DropAction(UnitActionId actionId)
{
    DropAction(actionId, staticActionInfo[actionId].priority);
}

void UnitStateMgr::DropAction(UnitActionId actionId, UnitActionPriority priority)
{
    for (UnitActionStorage::iterator itr = m_actions.begin(); itr != m_actions.end();)
    {
        if (itr->second.Id == actionId)
        {
            UnitActionPriority _priority = itr->first;
            ++itr;
            if (_priority <= priority)
                DropAction(_priority);
        }
        else
            ++itr;
    }
}

void UnitStateMgr::DropAction(UnitActionPriority priority)
{
    if (priority <= UNIT_ACTION_PRIORITY_NONE)
        return;

    ActionInfo* oldInfo = CurrentState();
    MAPLOCK_WRITE(GetOwner(), MAP_LOCK_TYPE_DEFAULT);
    UnitActionStorage::iterator itr = m_actions.find(priority);
    if (itr != m_actions.end())
    {
        if (&itr->second == oldInfo)
        {
            oldInfo->Interrupt(this);
            oldInfo->Finalize(this);
        }
        m_actions.erase(itr);
    }
}

void UnitStateMgr::PushAction(UnitActionId actionId)
{
    UnitActionPtr state = CreateStandartState(actionId);
    PushAction(actionId, state, staticActionInfo[actionId].priority, staticActionInfo[actionId].restoreable); 
}

void UnitStateMgr::PushAction(UnitActionId actionId, UnitActionPriority priority)
{
    UnitActionPtr state = CreateStandartState(actionId);
    PushAction(actionId, state, priority, ACTION_TYPE_NONRESTOREABLE);
}

void UnitStateMgr::PushAction(UnitActionId actionId, UnitActionPtr state)
{
    PushAction(actionId, state, staticActionInfo[actionId].priority, staticActionInfo[actionId].restoreable); 
}

void UnitStateMgr::PushAction(UnitActionId actionId, UnitActionPtr state, UnitActionPriority priority, eActionType restoreable)
{
    ActionInfo* oldInfo = CurrentState();
    UnitActionPriority _priority = oldInfo ? oldInfo->priority : UNIT_ACTION_PRIORITY_IDLE;
    if (oldInfo && _priority < priority)
        oldInfo->Interrupt(this);

    DropAction(priority);
    DropAction(actionId, priority);

    m_actions.insert(UnitActionStorage::value_type(priority,ActionInfo(actionId, state, priority, restoreable)));

    ActionInfo* currentInfo = CurrentState();
    if (currentInfo && currentInfo != oldInfo)
        currentInfo->Initialize(this);
}

ActionInfo* UnitStateMgr::GetAction(UnitActionPriority priority)
{
    UnitActionStorage::iterator itr = m_actions.find(priority);
    if (itr != m_actions.end())
        return &itr->second;
    return NULL;
}

UnitActionPtr UnitStateMgr::CurrentAction()
{
    return CurrentState() ? CurrentState()->Action() : UnitActionPtr(NULL);
}

ActionInfo* UnitStateMgr::CurrentState()
{
    return m_actions.empty() ? NULL : &m_actions.rbegin()->second;
}

void UnitStateMgr::DropAllStates()
{
    for (int32 i = UNIT_ACTION_PRIORITY_IDLE + 1; i != UNIT_ACTION_PRIORITY_END; ++i)
        DropAction(UnitActionPriority(i));
}

std::string const UnitStateMgr::GetOwnerStr() 
{
    return GetOwner()->IsInWorld() ? GetOwner()->GetGuidStr() : "<Uninitialized>"; 
};

bool ActionInfo::operator < (const ActionInfo& val) const
{
    if (priority > val.priority)
        return true;
    return false;
};

void ActionInfo::Delete()
{
    delete this;
}

void ActionInfo::Initialize(UnitStateMgr* mgr)
{
    MAPLOCK_READ(mgr->GetOwner(), MAP_LOCK_TYPE_DEFAULT);
    if (!initialized && Action())
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "ActionInfo: %s initialize action %s", mgr->GetOwnerStr().c_str(), TypeName());
        Action()->Initialize(*mgr->GetOwner());
        initialized = true;
    }
    else
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "ActionInfo: %s reset action %s", mgr->GetOwnerStr().c_str(), TypeName());
        Action()->Reset(*mgr->GetOwner());
    }
}

void ActionInfo::Finalize(UnitStateMgr* mgr)
{
    if (!initialized)
        return;
    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "ActionInfo: %s finalize action %s", mgr->GetOwnerStr().c_str(), TypeName());
    MAPLOCK_READ(mgr->GetOwner(), MAP_LOCK_TYPE_DEFAULT);
    if (Action())
        Action()->Finalize(*mgr->GetOwner());
}

void ActionInfo::Interrupt(UnitStateMgr* mgr)
{
    if (!initialized)
        return;
    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "ActionInfo: %s interrupt action %s", mgr->GetOwnerStr().c_str(), TypeName());
    MAPLOCK_READ(mgr->GetOwner(), MAP_LOCK_TYPE_DEFAULT);
    if (Action())
        Action()->Interrupt(*mgr->GetOwner());
}

bool ActionInfo::Update(UnitStateMgr* mgr, uint32 diff)
{
    // DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "ActionInfo: %s update action %s", mgr->GetOwnerStr().c_str(), TypeName());
    if (Action())
        return Action()->Update(*mgr->GetOwner(), diff);
    else
        return false;
}

const char* ActionInfo::TypeName() const 
{
    return (action ? action->Name() : "<empty>");
}
