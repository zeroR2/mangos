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

#include "EventProcessor.h"
#include "../game/World.h"

EventProcessor::EventProcessor() : m_aborting(false)
{
}

EventProcessor::~EventProcessor()
{
    KillAllEvents(true);
}

void EventProcessor::Update(uint32 update_diff, uint32 p_time)
{
    if (m_aborting)
        return;

    // update time
    uint32 m_time = WorldTimer::getMSTime();

    if (m_events.empty())
        return;

    for (EventList::iterator itr = m_events.begin(); !m_events.empty() && itr < m_events.end(); ++itr)
    {
        if (m_aborting)
            break;

        BasicEventPtr Event = *itr;
        if (Event->m_execTime > m_time && m_time - Event->m_execTime < DAY * IN_MILLISECONDS)
        {
            ++itr;
            continue;
        }

        if (!Event->to_Abort)
        {
            if (!Event->Execute(m_time, p_time))
            {
                // event execute time may be already changed in ::Execute() method
                if (Event->m_execTime <= m_time)
                    Event->ModExecTime(m_time + sWorld.getConfig(CONFIG_UINT32_INTERVAL_MAPUPDATE));
                continue;
            }
            else
            {
                Event->to_Abort = true;
            }
        }
        else
        {
            Event->Abort(m_time);
        }
        Event->Finish();
    }

    // reverce cleanup chain may some speedup process - deleting last records more fast then first.
    for (EventList::reverse_iterator itr = m_events.rbegin(); !m_events.empty() && itr != m_events.rend();)
    {
        BasicEventPtr Event = *itr;
        if (Event->IsFinished())
        {
            m_events.erase(Event);
            itr = m_events.rbegin();
        }
        else
            ++itr;
    }
}

void EventProcessor::KillAllEvents(bool force)
{
    if (m_aborting)
        return;

    // prevent event insertions
    m_aborting = true;

    // first, abort all existing events
    while (!m_events.empty())
    {
        BasicEventPtr Event = m_events.back();
        m_events.pop_back();
        if (Event)
        {
            Event->to_Abort = true;
            Event->Abort(WorldTimer::getMSTime());
        }
    }
}

void EventProcessor::AddEvent(BasicEvent* _event, uint32 e_time, bool set_addtime)
{
    if (m_aborting || !_event)
        return;

    BasicEventPtr Event = BasicEventPtr(_event);

    if (!Event)
    {
        sLog.outError("EventProcessor::AddEvent cannot add event to event processor by unknown reason.");
        return;
    }
    uint32 m_time = WorldTimer::getMSTime();

    if (set_addtime)
        Event->m_addTime = m_time;

    // correct e_time - not may be less, then current time, also may be too big
    if (e_time < m_time || WorldTimer::getMSTimeDiff(e_time,m_time) > DAY * IN_MILLISECONDS)
        e_time = m_time;

    Event->m_execTime = e_time;

    AddEvent(Event);
}

void EventProcessor::AddEvent(BasicEventPtr Event)
{
    m_events.push_back(Event);
}

uint32 EventProcessor::CalculateTime(uint32 t_offset)
{
    return WorldTimer::getMSTime() + t_offset;
}

// these can be used for time offset control
void BasicEvent::ModExecTime(uint32 e_time)
{
    if (e_time > WorldTimer::getMSTime() && WorldTimer::getMSTimeDiff(e_time,WorldTimer::getMSTime()) < DAY * IN_MILLISECONDS)
        m_execTime = e_time;
    else if (e_time == 0)
        m_execTime = WorldTimer::getMSTime() + sWorld.getConfig(CONFIG_UINT32_INTERVAL_MAPUPDATE);
    else  if (e_time < 30 * IN_MILLISECONDS)
        m_execTime += e_time;
    // else value incorrect - do nothing.
    else
        sLog.outError("BasicEvent::ModExecTime bad time diff! do nothing.");
}
