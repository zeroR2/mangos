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

#include "BattleField.h"
#include "../Player.h"
#include "../ObjectMgr.h"

BattleField::BattleField()
{
    ProgressBattle = false;
    m_defenderTeam = 0;
    m_attackTeam = 0;
    m_timerinviteZonePlayerToQueue = (m_timerNonBattle - 60) * 1000;
    m_TimeAcceptInvite = 20;
    finishNonBattle = false;
    finishBattle = false;
    m_group[0] = NULL;
    m_group[1] = NULL;
    canentermoreplayer = false;
    timerentermoreplayer = 60000;

}

void BattleField::HandlePlayerEnterZone(Player* player)
{
   if(player->getLevel() >= m_minLevel)
   {
    m_zonePlayers.insert(player->GetObjectGuid());

    if(InProgressBattle())
    {
      if(m_battlePlayers[GetTeamIndex(player->GetTeam())].size() + m_invitedPlayers[GetTeamIndex(player->GetTeam())].size() < m_maxPlayers)
         InvitePlayerToBattle(player);
      else
         InvitePlayerToQueue(player);
    }
    else
     InvitePlayerToQueue(player);
   }
   else
    KickFromBattleField(player,m_spellKick);
}

void BattleField::HandlePlayerLeaveZone(Player* player)
{
   if(player->getLevel() >= m_minLevel)
   {
       if(m_zonePlayers.count(player->GetObjectGuid()))
          m_zonePlayers.erase(player->GetObjectGuid());

       if(InProgressBattle())
       {
           if(m_battlePlayers[GetTeamIndex(player->GetTeam())].count(player->GetObjectGuid()))
           {
               m_battlePlayers[GetTeamIndex(player->GetTeam())].erase(player->GetObjectGuid());
               if(!m_group[GetTeamIndex(player->GetTeam())]->RemoveMember(player->GetObjectGuid(),0))
               {
                    m_group[GetTeamIndex(player->GetTeam())]->SetBattleFieldGroup(NULL);
                    sObjectMgr.RemoveGroup(m_group[GetTeamIndex(player->GetTeam())]);
                    m_group[GetTeamIndex(player->GetTeam())] = NULL;
               }
           }
       }
   }
}

void BattleField::Update(uint32 diff)
{
    if(!InProgressBattle())
    {
       if (m_timerinviteZonePlayerToQueue <= diff)
       {
          for (GuidSet::iterator itr = m_zonePlayers.begin(); itr != m_zonePlayers.end(); ++itr)
          {
              if(Player* pPlayer = sObjectMgr.GetPlayer(*itr))
                InvitePlayerToQueue(pPlayer);
          }
       }
       else
          m_timerinviteZonePlayerToQueue -= diff;

      if(finishNonBattle)
           StartTheBattle();
    }
    else
    {

       if (timerentermoreplayer <= diff)
       {
            canentermoreplayer = true;
       }
       else
          timerentermoreplayer -= diff;

      if(canentermoreplayer)
         InviteQueuePlayerToBattle();

      if(finishBattle)
           EndTheBattle();
    }
}

void BattleField::StartTheBattle()
{
    finishBattle = false;
    m_group[0] = NULL;
    m_group[1] = NULL;
    canentermoreplayer = false;
    timerentermoreplayer = 60000;

    ProgressBattle = true;

    Player* pPlayer = NULL;

   for (uint8 i = 0; i < 2; i++)
   {
     for (GuidSet::iterator itr = m_queuePreferencePlayers[i].begin(); itr != m_queuePreferencePlayers[i].end(); ++itr)
     {
       if(pPlayer = sObjectMgr.GetPlayer(*itr))
           InvitePlayerToBattle(pPlayer);
     }
   }
}

void BattleField::EndTheBattle()
{
    m_queuePreferencePlayers[0].clear();
    m_queuePreferencePlayers[1].clear();  
    m_queuePlayers[0].clear();
    m_queuePlayers[1].clear();

    m_timerinviteZonePlayerToQueue = (m_timerNonBattle - 60) * 1000;
    finishNonBattle = false;

    ProgressBattle = false;

    Player* pPlayer = NULL;

   for (uint8 i = 0; i < 2; i++)
   {
     for (GuidSet::iterator itr = m_battlePlayers[i].begin(); itr != m_battlePlayers[i].end(); ++itr)
     {
       if(pPlayer = sObjectMgr.GetPlayer(*itr))
       {
           if(!m_group[GetTeamIndex(pPlayer->GetTeam())]->RemoveMember(pPlayer->GetObjectGuid(),0))
           {
                m_group[GetTeamIndex(pPlayer->GetTeam())]->SetBattleFieldGroup(NULL);
                sObjectMgr.RemoveGroup(m_group[GetTeamIndex(pPlayer->GetTeam())]);
                m_group[GetTeamIndex(pPlayer->GetTeam())] = NULL;
           }

           if (BattleField* script = sBattleFieldMgr.GetScript(m_ZoneId))
               script->LeaveToBattle(pPlayer);

           pPlayer = NULL;
       }
     }
   }

}

void BattleField::SendUpdateWorldState(uint32 field, uint32 value)
{
    sWorldStateMgr.SetWorldStateValueFor(m_ZoneId, field, value);
}

void BattleField::SetBannerVisual(const WorldObject* objRef, ObjectGuid goGuid, uint32 artKit, uint32 animId)
{
    if (GameObject* go = objRef->GetMap()->GetGameObject(goGuid))
        SetBannerVisual(go, artKit, animId);
}

void BattleField::SetBannerVisual(GameObject* go, uint32 artKit, uint32 animId)
{
    go->SendGameObjectCustomAnim(go->GetObjectGuid(), animId);
    go->SetGoArtKit(artKit);
    go->Refresh();
}

// Kick player from zone by spell or position

void BattleField::KickFromBattleField(Player* player,uint32 spellid)
{
    if(m_zonePlayers.count(player->GetObjectGuid()))
      m_zonePlayers.erase(player->GetObjectGuid());

    if(spellid)
       player->CastSpell(player, spellid, true);
    else
       player->TeleportTo(KickPosition);
}

// Invite system

// Invite player to queue
void BattleField::InvitePlayerToQueue(Player* player)
{
    if(m_queuePlayers[GetTeamIndex(player->GetTeam())].count(player->GetObjectGuid()))
       return;

    if(m_queuePreferencePlayers[GetTeamIndex(player->GetTeam())].count(player->GetObjectGuid()))
       return;

      player->GetSession()->SendBattleFieldInvitePlayerToQueue(m_BattleFieldId);

}

// Acept invitation to queue
void BattleField::AcceptInvitePlayerToQueue(Player* player)
{
    if(m_queuePreferencePlayers[GetTeamIndex(player->GetTeam())].size() < m_mimPlayers && !InProgressBattle())
        m_queuePreferencePlayers[GetTeamIndex(player->GetTeam())].insert(player->GetObjectGuid());
    else
        m_queuePlayers[GetTeamIndex(player->GetTeam())].insert(player->GetObjectGuid());

   player->GetSession()->SendBattleFieldQueueInviteResponce(m_BattleFieldId,m_ZoneId);
}


void BattleField::InviteQueuePlayerToBattle()
{

   Player* pPlayer = NULL;

   for (uint8 i = 0; i < 2; i++)
   {
     for (GuidSet::iterator itr = m_queuePlayers[i].begin(); itr != m_queuePlayers[i].end(); ++itr)
     {
       if(pPlayer = sObjectMgr.GetPlayer(*itr))
       {
           if(m_battlePlayers[GetTeamIndex(pPlayer->GetTeam())].size() + m_invitedPlayers[GetTeamIndex(pPlayer->GetTeam())].size() < m_maxPlayers)
              InvitePlayerToBattle(pPlayer);

           pPlayer = NULL;
       }
     }
   }
}

// Invite player to Battle
void BattleField::InvitePlayerToBattle(Player* player)
{
     // dheck player in queue who player in Arena or BG
     if(player->InArena() || player->InBattleGround())
        return;

     // delete from queue if it is necesary
     if(m_queuePreferencePlayers[GetTeamIndex(player->GetTeam())].count(player->GetObjectGuid()))
        m_queuePreferencePlayers[GetTeamIndex(player->GetTeam())].erase(player->GetObjectGuid());

     if(m_queuePlayers[GetTeamIndex(player->GetTeam())].count(player->GetObjectGuid()))
        m_queuePlayers[GetTeamIndex(player->GetTeam())].erase(player->GetObjectGuid());

     // save site although you don't want acept invitation. If cancel you lost site
     m_invitedPlayers[GetTeamIndex(player->GetTeam())].insert(player->GetObjectGuid());
     player->GetSession()->SendBattleFieldInvitePlayerToBattle(m_BattleFieldId,m_ZoneId,m_TimeAcceptInvite);

}

void BattleField::AcceptInvitePlayerToBattle(Player* player)
{
     if(m_invitedPlayers[GetTeamIndex(player->GetTeam())].count(player->GetObjectGuid()))
        m_invitedPlayers[GetTeamIndex(player->GetTeam())].erase(player->GetObjectGuid());

      m_battlePlayers[GetTeamIndex(player->GetTeam())].insert(player->GetObjectGuid());

     player->GetSession()->SendBattleFieldEntered(m_BattleFieldId);

      if(Group* group = player->GetGroup())
         group->RemoveMember(player->GetObjectGuid(),0);

      AddGroup(player);

      if (BattleField* script = sBattleFieldMgr.GetScript(m_ZoneId))
        script->EnterToBattle(player);

}

void BattleField::CancelInvitePlayerToBattle(Player* player)
{
     if(m_invitedPlayers[GetTeamIndex(player->GetTeam())].count(player->GetObjectGuid()))
        m_invitedPlayers[GetTeamIndex(player->GetTeam())].erase(player->GetObjectGuid());
}

// Group System

void BattleField::AddGroup(Player* player)
{
    if(m_group[GetTeamIndex(player->GetTeam())] != NULL)
    {
        if(Group* group = m_group[GetTeamIndex(player->GetTeam())])
          group->AddMember(player->GetObjectGuid(), player->GetName());
    }
    else
    {
        m_group[GetTeamIndex(player->GetTeam())] = new Group;
        m_group[GetTeamIndex(player->GetTeam())]->SetBattleFieldGroup(this);
        m_group[GetTeamIndex(player->GetTeam())]->Create(player->GetObjectGuid(), player->GetName());
    }
}
