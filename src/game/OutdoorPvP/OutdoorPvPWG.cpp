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

/*
INFO :
- Author : Softbest
- Not work : Text, configs, spells and some small details
- Status : 90%
*/

#include "OutdoorPvP.h"
#include "OutdoorPvPWG.h"

OutdoorPvPWG::OutdoorPvPWG() : OutdoorPvP()
{
    InitBattlefield();
}

bool OutdoorPvPWG::InitBattlefield()
{
    m_bIsBattleStarted = false;
    get_map = false;

    activate = true; //get from config in the future

    if(activate)
    {
        SendUpdateWorldState(WS_BATTLE_ACTIVE_POS,1);
        SendUpdateWorldState(WS_BATTLE_ACTIVE_NEG,0);
    }
    else
    {
        SendUpdateWorldState(WS_BATTLE_ACTIVE_POS,0);
        SendUpdateWorldState(WS_BATTLE_ACTIVE_NEG,1);
    }

    defender = ALLIANCE; //get from config in the future
    if(defender == ALLIANCE)
      attacker = HORDE;
    else if(defender == HORDE)
      attacker = ALLIANCE;

    Install();

    return true;
}

void OutdoorPvPWG::Install()
{

       GraveyardIdsAlliance[0] = GraveyardIdsAlliance[1] = GraveyardIdsAlliance[2] = GraveyardIdsAlliance[3] = GraveyardIdsAlliance[4] = GraveyardIdsAlliance[5] = 0;
       GraveyardIdsHorde[0] = GraveyardIdsHorde[1] = GraveyardIdsHorde[2] = GraveyardIdsHorde[3] = GraveyardIdsHorde[4] = GraveyardIdsHorde[5] = 0;
       gyAllAlliance  = 0;
       gyAllHorde = 0;
       // GraveYard
       // Alliance
       if(WorldPvPGraveYardWG* gy = new WorldPvPGraveYardWG(this))
       {
         gy->Init(1332,ALLIANCE);
         gyAlliance = gy;
         gyAllAlliance = gyAllAlliance + 1;
         GraveyardIdsAlliance[gyAllAlliance - 1] = gyAlliance->GetId();
       }
       // Horde
       if(WorldPvPGraveYardWG* gy = new WorldPvPGraveYardWG(this))
       {
         gy->Init(1331,HORDE);
         gyHorde = gy;
         gyAllHorde = gyAllHorde + 1;
         GraveyardIdsHorde[gyAllHorde - 1] = gyHorde->GetId();
       }

       //Keep and towers
       for (uint8 i = 0; i < WG_MAX_OBJ; i++)
       {
          WorldPvPWGGameObjectBuilding *b = new WorldPvPWGGameObjectBuilding(this);
          b->Init(i, WGGameObjectBuillding[i].type, WGGameObjectBuillding[i].WorldState, WGGameObjectBuillding[i].textid);
          BuildingsInZone.insert(b);
          Building[i] = b;
       }

       // Keep GraveYard
           if(WorldPvPGraveYardWG* gy = new WorldPvPGraveYardWG(this))
        {
          gy->Init(1285,GetDefender());
          gyBuilding = gy;
          switch(GetDefender())
          {
            case ALLIANCE:
              gyAllAlliance  = gyAllAlliance + 1;
              GraveyardIdsAlliance[gyAllAlliance - 1] = gy->GetId();
              break;
            case HORDE:
              gyAllHorde = gyAllHorde + 1;
              GraveyardIdsHorde[gyAllHorde - 1] = gy->GetId();
              break;
          }
        }

       // WorkShops
       for (uint8 i = 0; i < WG_MAX_WORKSHOP; i++)
       {
          WorldPvPWGWorkShopData *ws = new WorldPvPWGWorkShopData(this);
          ws->Init(i,WGWorkShopDataBase[i].worldstate, WGWorkShopDataBase[i].type, WGWorkShopDataBase[i].textid);
          WorkShopList.push_back(ws);
          if(WGWorkShopDataBase[i].GraveYardId != 0)
          {
            WorldPvPGraveYardWG* gy = new WorldPvPGraveYardWG(this);
            gy->Init(WGWorkShopDataBase[i].GraveYardId,GetAttacker());
            ws->SetGraveYard(gy);
            if(ws->GetType() < WORLD_PVP_WG_WORKSHOP_KEEP_WEST)
            {
              switch(GetAttacker())
              {
                case ALLIANCE:
                     gyAllAlliance = gyAllAlliance + 1;
                     GraveyardIdsAlliance[gyAllAlliance - 1] = gy->GetId();
                    break;
                case HORDE:
                     gyAllHorde = gyAllHorde + 1;
                     GraveyardIdsHorde[gyAllHorde - 1] = gy->GetId();
                    break;
              }
            }
            switch(ws->GetType())
            {
              case WORLD_PVP_WG_WORKSHOP_NE:
                   gyNe = gy;
                   break;
              case WORLD_PVP_WG_WORKSHOP_NW:
                   gyNw = gy;
                   break;
              case WORLD_PVP_WG_WORKSHOP_SE:
                   gySe = gy;
                   break;
              case WORLD_PVP_WG_WORKSHOP_SW:
                   gySw = gy;
                   break;
            }
          }
            switch(ws->GetType())
            {
              case WORLD_PVP_WG_WORKSHOP_NE:
                   wsNe = ws;
                   break;
              case WORLD_PVP_WG_WORKSHOP_NW:
                   wsNw = ws;
                   break;
              case WORLD_PVP_WG_WORKSHOP_SE:
                   wsSe = ws;
                   break;
              case WORLD_PVP_WG_WORKSHOP_SW:
                   wsSw = ws;
                   break;
              case WORLD_PVP_WG_WORKSHOP_KEEP_WEST:
                  wsKw = ws;
                  break;
              case WORLD_PVP_WG_WORKSHOP_KEEP_EAST:
                  wsKe = ws;
                  break;
            }
       }

       CanUseRelic = false;
       DamageTowerAtt = 0;
       DamageTowerDef = 0;
       DestroyTowerAtt = 0;
       DestroyTowerDef = 0;
       m_tenacityStack = 0;
       m_bIsBattleStarted = true;
       TimeBattle = 500;
       m_Timer = TimeBattle;
       UpdateTimer = 1000;
       UpdateCounterVehicle(true);

}

void OutdoorPvPWG::UpdateTeleport(GameObject* pGo)
{
    if(GetDefender() == ALLIANCE)
       pGo->SetUInt32Value(GAMEOBJECT_FACTION, GO_FACTION_A);
    else if(GetDefender() == HORDE)
       pGo->SetUInt32Value(GAMEOBJECT_FACTION, GO_FACTION_H);
}

void OutdoorPvPWG::UpdateKeepTurret(Creature* pTurret)
{
     if(GetDefender() == ALLIANCE)
        pTurret->setFaction(NPC_FACTION_A);
    else if(GetDefender() == HORDE)
        pTurret->setFaction(NPC_FACTION_H);
}

void OutdoorPvPWG::PrepareKeepNpc(Creature* pCreature,uint32 team)
{
       if(GetDefender() == team)
       {
          if(GetDefender() == ALLIANCE)
              KeepCreatureA.push_back(pCreature->GetObjectGuid());
          else if(GetDefender() == HORDE)
              KeepCreatureH.push_back(pCreature->GetObjectGuid());
       }
       else if(GetDefender() != team)
       {
           pCreature->setFaction(35);
           pCreature->SetVisibility(VISIBILITY_OFF);

          if(GetDefender() == ALLIANCE)
              KeepCreatureH.push_back(pCreature->GetObjectGuid());
          else if(GetDefender() == HORDE)
              KeepCreatureA.push_back(pCreature->GetObjectGuid());
       }
}

void OutdoorPvPWG::PrepareKeepGo(GameObject* pGo,uint32 team)
{
       if(GetDefender() == team)
       {
          if(GetDefender() == ALLIANCE)
              m_KeepGameObjectA.push_back(pGo->GetObjectGuid());
          else if(GetDefender() == HORDE)
              m_KeepGameObjectH.push_back(pGo->GetObjectGuid());
       }
       else if(GetDefender() != team)
       {
          pGo->SetPhaseMask(100,true);

          if(GetDefender() == ALLIANCE)
              m_KeepGameObjectH.push_back(pGo->GetObjectGuid());
          else if(GetDefender() == HORDE)
              m_KeepGameObjectA.push_back(pGo->GetObjectGuid());
       }
}

void OutdoorPvPWG::HandlePlayerEnterZone(Player* pPlayer, bool isMainZone)
{
    if (pPlayer->getLevel() < 75)
    {
       pPlayer->CastSpell(pPlayer, SPELL_TELEPORT_DALARAN, true);
       return;
    }

    pPlayer->RemoveAurasDueToSpell(58045);
    pPlayer->RemoveAurasDueToSpell(SPELL_RECRUIT);
    pPlayer->RemoveAurasDueToSpell(SPELL_CORPORAL);
    pPlayer->RemoveAurasDueToSpell(SPELL_LIEUTENANT);
    pPlayer->RemoveAurasDueToSpell(SPELL_TOWER_CONTROL);
    pPlayer->RemoveAurasDueToSpell(SPELL_SPIRITUAL_IMMUNITY);
    pPlayer->RemoveAurasDueToSpell(SPELL_TENACITY);
    pPlayer->RemoveAurasDueToSpell(58730);

    if (pPlayer->GetTeam() == GetDefender())
        pPlayer->CastSpell(pPlayer,58045, true);

    pPlayer->CastSpell(pPlayer, SPELL_RECRUIT, true);
    pPlayer->CastSpell(pPlayer, SPELL_TENACITY, true);

    if(pPlayer->GetTeam() == GetAttacker())
    {
       pPlayer->CastSpell(pPlayer, SPELL_TOWER_CONTROL, true);
       if(SpellAuraHolderPtr pHolder = pPlayer->GetSpellAuraHolder(SPELL_TOWER_CONTROL))
       {
          pHolder->SetStackAmount(3 - DestroyTowerAtt);
       }
    }
    else if (!DestroyTowerAtt == 0)
    {
       pPlayer->CastSpell(pPlayer, SPELL_TOWER_CONTROL, true);
       if(SpellAuraHolderPtr pHolder = pPlayer->GetSpellAuraHolder(SPELL_TOWER_CONTROL))
       {
          pHolder->SetStackAmount(DestroyTowerAtt);
       }
    }

	m_sZonePlayers.insert(pPlayer->GetObjectGuid());

    if(pPlayer->GetTeam() == ALLIANCE)
        m_sZonePlayersAlliance.insert(pPlayer->GetObjectGuid());
	else if(pPlayer->GetTeam() == HORDE)
        m_sZonePlayersHorde.insert(pPlayer->GetObjectGuid());

    OutdoorPvP::HandlePlayerEnterZone(pPlayer,isMainZone);
    UpdateTenacityStack();
}

void OutdoorPvPWG::HandlePlayerLeaveZone(Player* pPlayer, bool isMainZone)
{
    pPlayer->RemoveAurasDueToSpell(58045);
    pPlayer->RemoveAurasDueToSpell(SPELL_RECRUIT);
    pPlayer->RemoveAurasDueToSpell(SPELL_CORPORAL);
    pPlayer->RemoveAurasDueToSpell(SPELL_LIEUTENANT);
    pPlayer->RemoveAurasDueToSpell(SPELL_TOWER_CONTROL);
    pPlayer->RemoveAurasDueToSpell(SPELL_SPIRITUAL_IMMUNITY);
    pPlayer->RemoveAurasDueToSpell(SPELL_TENACITY);
    pPlayer->RemoveAurasDueToSpell(58730);
    pPlayer->RemoveAurasDueToSpell(SPELL_TOWER_CONTROL);

	m_sZonePlayers.erase(pPlayer->GetObjectGuid());

	if(pPlayer->GetTeam() == ALLIANCE)
        m_sZonePlayersAlliance.erase(pPlayer->GetObjectGuid());
	else if(pPlayer->GetTeam() == HORDE)
        m_sZonePlayersHorde.erase(pPlayer->GetObjectGuid());

    OutdoorPvP::HandlePlayerLeaveZone(pPlayer,isMainZone);
    UpdateTenacityStack();
}

void OutdoorPvPWG::Update(uint32 uiDiff)
{
    if (m_bIsBattleStarted)
    {
        if (UpdateTimer < uiDiff)
        {
            m_Timer = m_Timer - 1;
            SendUpdateWorldState(WS_CLOCK_1,m_Timer + time(NULL));
            if(m_Timer > 0)
               UpdateTimer = 1000;
        }
        else
            UpdateTimer -= uiDiff;

        if(m_Timer == 0)
          NewRound(false);

        for (GameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
        {
           if((*itr)->map == false && get_map)
           {
              (*itr)->Mmap = m_Map;
              (*itr)->map = true;
           }
        }

        //Update World states
        UpdateVehicleCountWG();
        UpdateWSBuilding();
        UpdateWSWorkShop();

        if(activate)
        {
           SendUpdateWorldState(WS_BATTLE_ACTIVE_POS,1);
           SendUpdateWorldState(WS_BATTLE_ACTIVE_NEG,0);
        }
        else
        {
           SendUpdateWorldState(WS_BATTLE_ACTIVE_POS,0);
           SendUpdateWorldState(WS_BATTLE_ACTIVE_NEG,1);
        }

        // Vehicle Teleports
        if(GetDefender() == ALLIANCE)
        {
            if(get_map)
            {
                if(GameObject* gTeleport1 = m_Map->GetGameObject(VehcileTeleport1))
                {
                  for (std::list<ObjectGuid>::iterator itr = m_vehicleA.begin(); itr != m_vehicleA.end(); ++itr)
                  {
                      if(Creature* pCreature = m_Map->GetCreature((*itr)))
                      {
                          if(pCreature->GetDistance(gTeleport1) < 4.0f)
                               pCreature->NearTeleportTo(5240.45f,2703.42f,409.19f,3.19f);
                      }
                   }
                }

                if(GameObject* gTeleport2 = m_Map->GetGameObject(VehcileTeleport2))
                {
                  for (std::list<ObjectGuid>::iterator itr = m_vehicleA.begin(); itr != m_vehicleA.end(); ++itr)
                  {
                      if(Creature* pCreature = m_Map->GetCreature((*itr)))
                      {
                          if(pCreature->GetDistance(gTeleport2) < 4.0f)
                             pCreature->NearTeleportTo(5240.83f,2975.32f,409.187f,3.13f);
                      }
                   }
                }
            }
        }
        else if(GetDefender() == HORDE)
        {
            if(get_map)
            {
                if(GameObject* gTeleport1 = m_Map->GetGameObject(VehcileTeleport1))
                {
                  for (std::list<ObjectGuid>::iterator itr = m_vehicleH.begin(); itr != m_vehicleH.end(); ++itr)
                  {
                      if(Creature* pCreature = m_Map->GetCreature((*itr)))
                      {
                          if(pCreature->GetDistance(gTeleport1) < 4.0f)
                             pCreature->NearTeleportTo(5240.45f,2703.42f,409.19f,3.19f);
                      }
                   }
                }

                if(GameObject* gTeleport2 = m_Map->GetGameObject(VehcileTeleport2))
                {
                  for (std::list<ObjectGuid>::iterator itr = m_vehicleH.begin(); itr != m_vehicleH.end(); ++itr)
                  {
                      if(Creature* pCreature = m_Map->GetCreature((*itr)))
                      {
                          if(pCreature->GetDistance(gTeleport2) < 4.0f)
                             pCreature->NearTeleportTo(5240.83f,2975.32f,409.187f,3.13f);
                      }
                   }
                }
            }
        }
    }
}

void OutdoorPvPWG::NewRound(bool titan)
{
  if(titan)
  {
    if(GetDefender() == ALLIANCE)
    {
        defender = HORDE;
        attacker = ALLIANCE;
        SendUpdateWorldState(WS_DEFENDER_TEAM,1);
        SendUpdateWorldState(WS_ATTACKER_TEAM,0);
    }
    else if (GetDefender() == HORDE)
    {
        defender = ALLIANCE;
        attacker = HORDE;
        SendUpdateWorldState(WS_DEFENDER_TEAM,0);
        SendUpdateWorldState(WS_ATTACKER_TEAM,1);
    }

    for (GameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
    {
        (*itr)->ChangeTeam(GetDefender());
    }

    Building[27]->ChangeTeam(GetAttacker());
    Building[28]->ChangeTeam(GetAttacker());
    Building[29]->ChangeTeam(GetAttacker());

    wsKe->ChangeControl(GetDefender());
    wsKw->ChangeControl(GetDefender());


    if(GetDefender() == ALLIANCE)
    {
        for (std::list<ObjectGuid>::iterator itr = KeepCreatureH.begin(); itr != KeepCreatureH.end(); ++itr)
        {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
            {
               pCreature->setFaction(35);
               pCreature->SetVisibility(VISIBILITY_OFF);
            }
        }

        for (std::list<ObjectGuid>::iterator itr = KeepCreatureA.begin(); itr != KeepCreatureA.end(); ++itr)
        {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
            {
               pCreature->Respawn();
               pCreature->setFaction(NPC_FACTION_A);
               pCreature->SetVisibility(VISIBILITY_ON);
            }
        }

        for (std::list<ObjectGuid>::iterator itr = m_KeepGameObjectA.begin(); itr != m_KeepGameObjectA.end(); ++itr)
        {
            if(GameObject* pGo = m_Map->GetGameObject((*itr)))
               pGo->SetPhaseMask(1,true);
        }

        for (std::list<ObjectGuid>::iterator itr = m_KeepGameObjectH.begin(); itr != m_KeepGameObjectH.end(); ++itr)
        {
            if(GameObject* pGo = m_Map->GetGameObject((*itr)))
               pGo->SetPhaseMask(100,true);
        }

        gyBuilding->ChangeTeam(ALLIANCE);
        DeleteGraveYardWG(gyBuilding,HORDE);
        AddGraveYardWG(gyBuilding,ALLIANCE);

    }
    else if(GetDefender() == HORDE)
    {
        for (std::list<ObjectGuid>::iterator itr = KeepCreatureA.begin(); itr != KeepCreatureA.end(); ++itr)
        {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
            {
               pCreature->setFaction(35);
               pCreature->SetVisibility(VISIBILITY_OFF);
            }
        }

        for (std::list<ObjectGuid>::iterator itr = KeepCreatureH.begin(); itr != KeepCreatureH.end(); ++itr)
        {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
            {
               pCreature->Respawn();
               pCreature->setFaction(NPC_FACTION_H);
               pCreature->SetVisibility(VISIBILITY_ON);
            }
        }

        for (std::list<ObjectGuid>::iterator itr = m_KeepGameObjectH.begin(); itr != m_KeepGameObjectH.end(); ++itr)
        {
            if(GameObject* pGo = m_Map->GetGameObject((*itr)))
               pGo->SetPhaseMask(1,true);
        }

        for (std::list<ObjectGuid>::iterator itr = m_KeepGameObjectA.begin(); itr != m_KeepGameObjectA.end(); ++itr)
        {
            if(GameObject* pGo = m_Map->GetGameObject((*itr)))
               pGo->SetPhaseMask(100,true);
        }

        gyBuilding->ChangeTeam(HORDE);
        DeleteGraveYardWG(gyBuilding,ALLIANCE);
        AddGraveYardWG(gyBuilding,HORDE);
    }

    for (std::list<ObjectGuid>::iterator itr = TeleportGameObject.begin(); itr != TeleportGameObject.end(); ++itr)
    {
            if(GameObject* pGo = m_Map->GetGameObject((*itr)))
               UpdateTeleport(pGo);
    }

    for (std::list<ObjectGuid>::iterator itr = TurretCreature.begin(); itr != TurretCreature.end(); ++itr)
    {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
            {
               pCreature->Respawn();
               UpdateKeepTurret(pCreature);
            }
    }

    wsNe->ChangeControl(GetAttacker());
    wsNw->ChangeControl(GetAttacker());
    wsSe->ChangeControl(GetAttacker());
    wsSw->ChangeControl(GetAttacker());

    for (std::list<ObjectGuid>::iterator itr = m_vehicleA.begin(); itr != m_vehicleA.end(); ++itr)
    {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
               pCreature->ForcedDespawn();
    }

    for (std::list<ObjectGuid>::iterator itr = m_vehicleH.begin(); itr != m_vehicleH.end(); ++itr)
    {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
               pCreature->ForcedDespawn();
    }

    CanUseRelic = false;

    VehicleCountH = 0;
    VehicleCountA = 0;
    UpdateVehicleCountWG();

  }
  else if(!titan)
  {
    for (GameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
    {
        (*itr)->Rebuild();
    }

    if(GetDefender() == ALLIANCE)
    {
        for (std::list<ObjectGuid>::iterator itr = KeepCreatureA.begin(); itr != KeepCreatureA.end(); ++itr)
        {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
               pCreature->Respawn();
        }
    }
    else if(GetDefender() == HORDE)
    {
        for (std::list<ObjectGuid>::iterator itr = KeepCreatureH.begin(); itr != KeepCreatureH.end(); ++itr)
        {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
               pCreature->Respawn();
        }
    }

    for (std::list<ObjectGuid>::iterator itr = TurretCreature.begin(); itr != TurretCreature.end(); ++itr)
    {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
               pCreature->Respawn();
    }

    wsNe->ChangeControl(GetAttacker());
    wsNw->ChangeControl(GetAttacker());
    wsSe->ChangeControl(GetAttacker());
    wsSw->ChangeControl(GetAttacker());

    for (std::list<ObjectGuid>::iterator itr = m_vehicleA.begin(); itr != m_vehicleA.end(); ++itr)
    {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
               pCreature->ForcedDespawn();
    }

    for (std::list<ObjectGuid>::iterator itr = m_vehicleH.begin(); itr != m_vehicleH.end(); ++itr)
    {
            if(Creature* pCreature = m_Map->GetCreature((*itr)))
               pCreature->ForcedDespawn();
    }

    CanUseRelic = false;

    VehicleCountH = 0;
    VehicleCountA = 0;
    UpdateVehicleCountWG();

   }

    AddDataWhenWin();
    RecolatePlayers();

    m_Timer = TimeBattle;
}

void OutdoorPvPWG::HandleCreatureCreate(Creature* pCreature)
{
    if(!get_map)
    {
        m_Map = pCreature->GetMap();
        get_map = true;
    }

    switch (pCreature->GetEntry())
    {
       case NPC_GUARD_A:
        if(pCreature->GetGUIDLow() >= 530060  && pCreature->GetGUIDLow() < 530100)
        {
            if(pCreature->GetGUIDLow() < 530070)
                wsNe->AddCreature(pCreature,ALLIANCE);
            else if(pCreature->GetGUIDLow() >= 530070 && pCreature->GetGUIDLow() < 530080)
                wsNw->AddCreature(pCreature,ALLIANCE);
            else if(pCreature->GetGUIDLow() >= 530080 && pCreature->GetGUIDLow() < 530090)
                wsSe->AddCreature(pCreature,ALLIANCE);
            else if(pCreature->GetGUIDLow() >= 530090 && pCreature->GetGUIDLow() < 530100)
                wsSw->AddCreature(pCreature,ALLIANCE);
        }
        else if(pCreature->GetGUIDLow() >= 531000  && pCreature->GetGUIDLow() < 531200)
        {
            PrepareKeepNpc(pCreature,ALLIANCE);
        }
        else if(pCreature->GetGUIDLow() >= 531400)
        {
            if(pCreature->GetGUIDLow() >= 531400 && pCreature->GetGUIDLow() < 531410)
              Building[27]->AddCreature(pCreature,ALLIANCE);
            else if(pCreature->GetGUIDLow() >= 531410 && pCreature->GetGUIDLow() < 531420)
              Building[28]->AddCreature(pCreature,ALLIANCE);
            else if(pCreature->GetGUIDLow() >= 531420 && pCreature->GetGUIDLow() < 531430)
              Building[29]->AddCreature(pCreature,ALLIANCE);
        }
        break;
       case NPC_GUARD_H:
        if(pCreature->GetGUIDLow() >= 530160  && pCreature->GetGUIDLow() < 530200)
        {
            if(pCreature->GetGUIDLow() < 530170)
                wsNe->AddCreature(pCreature,HORDE);
            else if(pCreature->GetGUIDLow() >= 530170 && pCreature->GetGUIDLow() < 530180)
                wsNw->AddCreature(pCreature,HORDE);
            else if(pCreature->GetGUIDLow() >= 530180 && pCreature->GetGUIDLow() < 530190)
                wsSe->AddCreature(pCreature,HORDE);
            else if(pCreature->GetGUIDLow() >= 530190 && pCreature->GetGUIDLow() < 530200)
                wsSw->AddCreature(pCreature,HORDE);
        }
        else if(pCreature->GetGUIDLow() >= 531200  && pCreature->GetGUIDLow() < 531400)
        {
            PrepareKeepNpc(pCreature,HORDE);
        }
        else if(pCreature->GetGUIDLow() >= 531400)
        {
            if(pCreature->GetGUIDLow() >= 531500 && pCreature->GetGUIDLow() < 531510)
               Building[27]->AddCreature(pCreature,HORDE);
            else if(pCreature->GetGUIDLow() >= 531510 && pCreature->GetGUIDLow() < 531520)
               Building[28]->AddCreature(pCreature,HORDE);
            else if(pCreature->GetGUIDLow() >= 531520 && pCreature->GetGUIDLow() < 531530)
               Building[29]->AddCreature(pCreature,HORDE);
        }
        break;
       case NPC_BOWYER_RANDOLPH:
       case NPC_SORCERESS_KAYLANA:
       case NPC_MARSHAL_MAGRUDER:
       case NPC_COMMANDER_ZANNETH:
       case NPC_TACTICAL_OFFICER_AHBRAMIS:
       case NPC_SIEGE_MASTER_STOUTHANDLE:
       case NPC_ANCHORITE_TESSA:
       case NPC_SENIOR_DEMOLITIONIST_LEGOSO:
           PrepareKeepNpc(pCreature,ALLIANCE);
           break;
       case NPC_VIERON_BLAZEFEATHER:
       case NPC_HOODOO_MASTER_FU_JIN:
       case NPC_CHAMPION_ROS_SLAI:
       case NPC_COMMANDER_DARDOSH:
       case NPC_TACTICAL_OFFICER_KILRATH:
       case NPC_SIEGESMITH_STRONGHOOF:
       case NPC_PRIMALIST_MULFORT:
       case NPC_LIEUTENANT_MURP:
           PrepareKeepNpc(pCreature,HORDE);
           break;
       case NPC_TURRER:
            if(pCreature->GetGUIDLow() < 530000)
            {
               TurretCreature.push_back(pCreature->GetObjectGuid());
               UpdateKeepTurret(pCreature);
            }
            else
            {
                if(pCreature->GetGUIDLow() < 530010)
                  Building[27]->AddTurret(pCreature);
                else if(pCreature->GetGUIDLow() >= 530010 && pCreature->GetGUIDLow() < 530020)
                  Building[28]->AddTurret(pCreature);
                else if(pCreature->GetGUIDLow() >= 530020)
                  Building[29]->AddTurret(pCreature);
            }
           break;
       //Spirit guide
       case NPC_SPIRIT_GUIDE_A:
           switch (pCreature->GetGUIDLow())
           {
              case 530030:
                  gyNe->AddSpiritGuide(pCreature,ALLIANCE);
                  break;
              case 530032:
                  gyNw->AddSpiritGuide(pCreature,ALLIANCE);
                  break;
              case 530034:
                  gySe->AddSpiritGuide(pCreature,ALLIANCE);
                  break;
              case 530036:
                  gySw->AddSpiritGuide(pCreature,ALLIANCE);
                  break;
              case 530038:
                  gyBuilding->AddSpiritGuide(pCreature,ALLIANCE);
                  break;
           }
           break;
       case NPC_SPIRIT_GUIDE_H:
           switch (pCreature->GetGUIDLow())
           {
              case 530031:
                  gyNe->AddSpiritGuide(pCreature,HORDE);
                  break;
              case 530033:
                  gyNw->AddSpiritGuide(pCreature,HORDE);
                  break;
              case 530035:
                  gySe->AddSpiritGuide(pCreature,HORDE);
                  break;
              case 530037:
                  gySw->AddSpiritGuide(pCreature,HORDE);
                  break;
              case 530039:
                  gyBuilding->AddSpiritGuide(pCreature,HORDE);
                  break;
           }
           break;
       // Demolisher engineer
       case NPC_DEMOLISHER_ENGINEER_A:
           switch (pCreature->GetGUIDLow())
           {
              case 530040:
                  wsKw->AddCreature(pCreature,ALLIANCE);
                  break;
              case 530042:
                  wsKe->AddCreature(pCreature,ALLIANCE);
                  break;
              case 530044:
                  wsNe->AddCreature(pCreature,ALLIANCE);
                  break;
              case 530046:
                  wsNw->AddCreature(pCreature,ALLIANCE);
                  break;
              case 530048:
                  wsSe->AddCreature(pCreature,ALLIANCE);
                  break;
              case 530050:
                  wsSw->AddCreature(pCreature,ALLIANCE);
                  break;
           }
           break;
       case NPC_DEMOLISHER_ENGINEER_H:
           switch (pCreature->GetGUIDLow())
           {
              case 530041:
                  wsKw->AddCreature(pCreature,HORDE);
                  break;
              case 530043:
                  wsKe->AddCreature(pCreature,HORDE);
                  break;
              case 530045:
                  wsNe->AddCreature(pCreature,HORDE);
                  break;
              case 530047:
                  wsNw->AddCreature(pCreature,HORDE);
                  break;
              case 530049:
                  wsSe->AddCreature(pCreature,HORDE);
                  break;
              case 530051:
                  wsSw->AddCreature(pCreature,HORDE);
                  break;
           }
           break;
    }
}

void OutdoorPvPWG::HandleGameObjectCreate(GameObject* pGo)
{
    if(!get_map)
    {
        m_Map = pGo->GetMap();
        get_map = true;
    }

    switch (pGo->GetEntry())
    {
    //Wall and tower
       case 190219:
           gBandT[0] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190220:
           gBandT[1] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191795:
           gBandT[2] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191796:
           gBandT[3] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191799:
           gBandT[4] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191800:
           gBandT[5] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191801:
           gBandT[6] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191802:
           gBandT[7] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191803:
           gBandT[8] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191804:
           gBandT[9] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191806:
           gBandT[10] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191807:
           gBandT[11] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191808:
           gBandT[12] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191809:
           gBandT[13] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190369:
           gBandT[14] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190370:
           gBandT[15] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190371:
           gBandT[16] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190372:
           gBandT[17] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190374:
           gBandT[18] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190376:
           gBandT[19] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190221:
           gBandT[20] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190373:
           gBandT[21] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190377:
           gBandT[22] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190378:
           gBandT[23] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191797:
           gBandT[24] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191798:
           gBandT[25] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191805:
           gBandT[26] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190356:
           gBandT[27] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190357:
           gBandT[28] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190358:
           gBandT[29] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 190375:
           gBandT[30] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 191810:
           gBandT[31] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
    // Keep GameObject
       case 192269:
       case 192501:
       case 192416:
       case 192321:
       case 192335:
           if(pGo->GetGUIDLow() >= 531000 &&  pGo->GetGUIDLow() < 531200)
               PrepareKeepGo(pGo,ALLIANCE);
           else if(pGo->GetGUIDLow() >= 531400 && pGo->GetGUIDLow() < 531500)
           {
               if(pGo->GetGUIDLow() < 531410)
                   Building[27]->AddGameObject(pGo,ALLIANCE);
               else if(pGo->GetGUIDLow() >= 531410 && pGo->GetGUIDLow() < 531420)
                   Building[28]->AddGameObject(pGo,ALLIANCE);
               else if(pGo->GetGUIDLow() >= 531420 && pGo->GetGUIDLow() < 531430)
                   Building[29]->AddGameObject(pGo,ALLIANCE);
           }
           break;
       case 192488:
       case 192374:
       case 192375:
       case 192336:
       case 192255:
       case 192254:
       case 192349:
       case 192366:
       case 192367:
       case 192364:
       case 192370:
       case 192369:
       case 192368:
       case 192362:
       case 192363:
       case 192379:
       case 192378:
       case 192355:
       case 192354:
       case 192358:
       case 192359:
       case 192338:
       case 192339:
       case 192284:
       case 192285:
       case 192371:
       case 192372:
       case 192373:
       case 192360:
       case 192361:
       case 192356:
       case 192352:
       case 192353:
       case 192357:
       case 192350:
       case 192351:
           if(pGo->GetGUIDLow() >= 531200 &&  pGo->GetGUIDLow() < 531400)
               PrepareKeepGo(pGo,HORDE);
           else if(pGo->GetGUIDLow() >= 531500 && pGo->GetGUIDLow() < 531600)
           {
               if(pGo->GetGUIDLow() < 531510)
                   Building[27]->AddGameObject(pGo,HORDE);
               else if(pGo->GetGUIDLow() >= 531510 && pGo->GetGUIDLow() < 531520)
                   Building[28]->AddGameObject(pGo,HORDE);
               else if(pGo->GetGUIDLow() >= 531520 && pGo->GetGUIDLow() < 531530)
                   Building[29]->AddGameObject(pGo,HORDE);
           }
           break;
    // Tower GameObject
       case 192278:
       case 192414:
       case 192429:
       case 192430:
       case 192266:
       case 192423:
           if(pGo->GetGUIDLow() >= 531400 && pGo->GetGUIDLow() < 531500)
           {
               if(pGo->GetGUIDLow() < 531410)
                   Building[27]->AddGameObject(pGo,ALLIANCE);
               else if(pGo->GetGUIDLow() >= 531410 && pGo->GetGUIDLow() < 531420)
                   Building[28]->AddGameObject(pGo,ALLIANCE);
               else if(pGo->GetGUIDLow() >= 531420 && pGo->GetGUIDLow() < 531430)
                   Building[29]->AddGameObject(pGo,ALLIANCE);
           }
           break;
       case 192443:
       case 192267:
       case 192424:
           if(pGo->GetGUIDLow() >= 531500 && pGo->GetGUIDLow() < 531600)
           {
               if(pGo->GetGUIDLow() < 531510)
                   Building[27]->AddGameObject(pGo,HORDE);
               else if(pGo->GetGUIDLow() >= 531510 && pGo->GetGUIDLow() < 531520)
                   Building[28]->AddGameObject(pGo,HORDE);
               else if(pGo->GetGUIDLow() >= 531520 && pGo->GetGUIDLow() < 531530)
                   Building[29]->AddGameObject(pGo,HORDE);
           }
           break;
    // Teleports
       case GO_TELEPORT:
           TeleportGameObject.push_back(pGo->GetObjectGuid());
           UpdateTeleport(pGo);
           break;
       case GO_VEHICLE_TELEPORT:
           TeleportGameObject.push_back(pGo->GetObjectGuid());
           if(VehcileTeleport1.IsEmpty())
               VehcileTeleport1 = pGo->GetObjectGuid();
           else
               VehcileTeleport2 = pGo->GetObjectGuid();
           UpdateTeleport(pGo);
           break;
    // Go WS
       case GO_FACTORY_BANNER_NE:
           gBanner[0] = pGo->GetObjectGuid();
           if(GetAttacker() == ALLIANCE)
              pGo->SetGoArtKit(GO_ARTKIT_BANNER_ALLIANCE);
           else if(GetAttacker() == HORDE)
              pGo->SetGoArtKit(GO_ARTKIT_BANNER_HORDE);
           break;
       case GO_FACTORY_BANNER_NW:
           gBanner[1] = pGo->GetObjectGuid();
           if(GetAttacker() == ALLIANCE)
              pGo->SetGoArtKit(GO_ARTKIT_BANNER_ALLIANCE);
           else if(GetAttacker() == HORDE)
              pGo->SetGoArtKit(GO_ARTKIT_BANNER_HORDE);
           break;
       case GO_FACTORY_BANNER_SE:
           gBanner[2] = pGo->GetObjectGuid();
           if(GetAttacker() == ALLIANCE)
              pGo->SetGoArtKit(GO_ARTKIT_BANNER_ALLIANCE);
           else if(GetAttacker() == HORDE)
              pGo->SetGoArtKit(GO_ARTKIT_BANNER_HORDE);
           break;
       case GO_FACTORY_BANNER_SW:
           gBanner[3] = pGo->GetObjectGuid();
           if(GetAttacker() == ALLIANCE)
              pGo->SetGoArtKit(GO_ARTKIT_BANNER_ALLIANCE);
           else if(GetAttacker() == HORDE)
              pGo->SetGoArtKit(GO_ARTKIT_BANNER_HORDE);
           break;
       case GO_BUILDING_NE:
           gBuilding[0] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case GO_BUILDING_NW:
           gBuilding[1] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case GO_BUILDING_SE:
           gBuilding[2] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case GO_BUILDING_SW:
           gBuilding[3] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case GO_BUILDING_KEEP_WEST:
           gBuilding[4] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case GO_BUILDING_KEEP_EAST:
           gBuilding[5] = pGo->GetObjectGuid();
           pGo->Rebuild(GetPlayerInZone());
           break;
       case 192274:
       case 192406:
       case 192433:
           if(pGo->GetGUIDLow() >= 530010 && pGo->GetGUIDLow() < 530050)
           {
              if(pGo->GetGUIDLow() < 530020)
                  wsNe->AddGameObject(pGo,ALLIANCE);
              else if(pGo->GetGUIDLow() >= 530020 && pGo->GetGUIDLow() < 530030)
                  wsNw->AddGameObject(pGo,ALLIANCE);
              else if(pGo->GetGUIDLow() >= 530030 && pGo->GetGUIDLow() < 530040)
                  wsSe->AddGameObject(pGo,ALLIANCE);
              else if(pGo->GetGUIDLow() >= 530040 && pGo->GetGUIDLow() < 530050)
                  wsSw->AddGameObject(pGo,ALLIANCE);
           }
           break;
       case 192280:
       case 192435:
       case 192434:
       case 192432:
           if(pGo->GetGUIDLow() >= 530110 && pGo->GetGUIDLow() < 530150)
           {
              if(pGo->GetGUIDLow() < 530120)
                  wsNe->AddGameObject(pGo,HORDE);
              else if(pGo->GetGUIDLow() >= 530120 && pGo->GetGUIDLow() < 530130)
                  wsNw->AddGameObject(pGo,HORDE);
              else if(pGo->GetGUIDLow() >= 530130 && pGo->GetGUIDLow() < 530140)
                  wsSe->AddGameObject(pGo,HORDE);
              else if(pGo->GetGUIDLow() >= 530140 && pGo->GetGUIDLow() < 530150)
                  wsSw->AddGameObject(pGo,HORDE);
           }
           break;
       //Titan relic
       case GO_TITAN_RELIC:
           Relic = pGo->GetObjectGuid();
           break;
    }
}

void OutdoorPvPWG::HandleCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
       case NPC_CATAPULT:
       case NPC_DEMOLISHER:
       case NPC_SIEGE_ENGINE_A:
       case NPC_SIEGE_ENGINE_H:
           if(pCreature->getFaction() == NPC_FACTION_A)
               DeleteVehicle(pCreature,ALLIANCE);
           else if(pCreature->getFaction() == NPC_FACTION_H)
               DeleteVehicle(pCreature,HORDE);
           pCreature->ForcedDespawn();
           break;
    }

}

bool OutdoorPvPWG::HandleGameObjectUse(Player* pPlayer, GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
       case GO_TITAN_RELIC:
           if (pPlayer->GetTeam() == GetAttacker())
           {
            if (CanUseRelic)
              NewRound(true);
           }
           break;
    }

    return true;
}

bool OutdoorPvPWG::HandleEvent(uint32 uiEventId,GameObject* pGo)
{
   bool factory = false;
   WorldPvPWGWorkShopData* ws;
   switch(pGo->GetEntry())
   {
       case GO_FACTORY_BANNER_NE:
           ws = wsNe;
           factory = true;
           break;
       case GO_FACTORY_BANNER_NW:
           ws = wsNw;
           factory = true;
           break;
       case GO_FACTORY_BANNER_SE:
           ws = wsSe;
           factory = true;
           break;
       case GO_FACTORY_BANNER_SW:
           ws = wsSw;
           factory = true;
           break;
   }

   if(factory)
   {
        switch(uiEventId)
        {
            case EVENT_FACTORY_NE_PROGRESS_ALLIANCE:
            case EVENT_FACTORY_NW_PROGRESS_ALLIANCE:
            case EVENT_FACTORY_SE_PROGRESS_ALLIANCE:
            case EVENT_FACTORY_SW_PROGRESS_ALLIANCE:
                ws->ChangeControl(ALLIANCE);
				return true;
                break;
            case EVENT_FACTORY_NE_PROGRESS_HORDE:
            case EVENT_FACTORY_NW_PROGRESS_HORDE:
            case EVENT_FACTORY_SE_PROGRESS_HORDE:
            case EVENT_FACTORY_SW_PROGRESS_HORDE:
                ws->ChangeControl(HORDE);
				return true;
                break;
        }
   }
   else
	   return false;
}

void OutdoorPvPWG::EventPlayerDamageGO(Player *player, GameObject* target_obj, uint32 eventId, uint32 spellId)
{
     switch(target_obj->GetEntry())
     {
       //Wall
       case 190219:
           if(eventId == 19896)
             Building[0]->Damaged();
           else if (eventId == 19910)
             Building[0]->Destroyed(player);
           break;
       case 190220:
           if(eventId == 19897)
             Building[1]->Damaged();
           else if (eventId == 19911)
             Building[1]->Destroyed(player);
           break;
       case 191795:
           if(eventId == 19933)
             Building[2]->Damaged();
           else if (eventId == 19942)
             Building[2]->Destroyed(player);
           break;
       case 191796:
           if(eventId == 19941)
             Building[3]->Damaged();
           else if (eventId == 19950)
             Building[3]->Destroyed(player);
           break;
       case 191799:
           if(eventId == 19909)
             Building[4]->Damaged();
           else if (eventId == 19923)
             Building[4]->Destroyed(player);
           break;
       case 191800:
           if(eventId == 19935)
             Building[5]->Damaged();
           else if (eventId == 19944)
             Building[5]->Destroyed(player);
           break;
       case 191801:
           if(eventId == 19939)
             Building[6]->Damaged();
           else if (eventId == 19948)
             Building[6]->Destroyed(player);
           break;
       case 191802:
           if(eventId == 19898)
             Building[7]->Damaged();
           else if (eventId == 19912)
             Building[7]->Destroyed(player);
           break;
       case 191803:
           if(eventId == 19899)
             Building[8]->Damaged();
           else if (eventId == 19913)
             Building[8]->Destroyed(player);
           break;
       case 191804:
           if(eventId == 19936)
             Building[9]->Damaged();
           else if (eventId == 19945)
             Building[9]->Destroyed(player);
           break;
       case 191806:
           if(eventId == 19938)
             Building[10]->Damaged();
           else if (eventId == 19947)
             Building[10]->Destroyed(player);
           break;
       case 191807:
           if(eventId == 19906)
             Building[11]->Damaged();
           else if (eventId == 19920)
             Building[11]->Destroyed(player);
           break;
       case 191808:
           if(eventId == 19907)
             Building[12]->Damaged();
           else if (eventId == 19921)
             Building[12]->Destroyed(player);
           break;
       case 191809:
           if(eventId == 19908)
             Building[13]->Damaged();
           else if (eventId == 19922)
             Building[13]->Destroyed(player);
           break;
       case 190369:
           if(eventId == 19900)
             Building[14]->Damaged();
           else if (eventId == 19914)
             Building[14]->Destroyed(player);
           break;
       case 190370:
           if(eventId == 19905)
             Building[15]->Damaged();
           else if (eventId == 19919)
             Building[15]->Destroyed(player);
           break;
       case 190371:
           if(eventId == 19901)
             Building[16]->Damaged();
           else if (eventId == 19915)
             Building[16]->Destroyed(player);
           break;
       case 190372:
           if(eventId == 19904)
             Building[17]->Damaged();
           else if (eventId == 19918)
             Building[17]->Destroyed(player);
           break;
       case 190374:
           if(eventId == 19902)
             Building[18]->Damaged();
           else if (eventId == 19916)
             Building[18]->Destroyed(player);
           break;
       case 190376:
           if(eventId == 19903)
             Building[19]->Damaged();
           else if (eventId == 19917)
             Building[19]->Destroyed(player);
           break;
       //Keep Tower
       case 190221:
           if(eventId == 19657)
             Building[20]->Damaged();
           else if (eventId == 19661)
             Building[20]->Destroyed(player);
           break;
       case 190373:
           if(eventId == 19659)
             Building[21]->Damaged();
           else if (eventId == 19662)
             Building[21]->Destroyed(player);
           break;
       case 190377:
           if(eventId == 19660)
             Building[22]->Damaged();
           else if (eventId == 19664)
             Building[22]->Destroyed(player);
           break;
       case 190378:
           if(eventId == 19658)
             Building[23]->Damaged();
           else if (eventId == 19663)
             Building[23]->Destroyed(player);
           break;
       //Wall
       case 191797:
           if(eventId == 19934)
             Building[24]->Damaged();
           else if (eventId == 19943)
             Building[24]->Destroyed(player);
           break;
       case 191798:
           if(eventId == 19940)
             Building[25]->Damaged();
           else if (eventId == 19949)
             Building[25]->Destroyed(player);
           break;
       case 191805:
           if(eventId == 19937)
             Building[26]->Damaged();
           else if (eventId == 19946)
             Building[26]->Destroyed(player);
           break;
       //South tower
       case 190356:
           if(eventId == 19674)
             Building[27]->Damaged();
           else if (eventId == 19677)
             Building[27]->Destroyed(player);
           break;
       case 190357:
           if(eventId == 19673)
             Building[28]->Damaged();
           else if (eventId == 19676)
             Building[28]->Destroyed(player);
           break;
       case 190358:
           if(eventId == 19672)
             Building[29]->Damaged();
           else if (eventId == 19675)
             Building[29]->Destroyed(player);
           break;
       //Door of forteress
       case 190375:
           if(eventId == 19956)
             Building[30]->Damaged();
           else if (eventId == 19957)
             Building[30]->Destroyed(player);
           break;
       //Last door
       case 191810:
           if(eventId == 19448)
             Building[31]->Damaged();
           else if (eventId == 19607)
             Building[31]->Destroyed(player);
           break;
     }
}

void OutdoorPvPWG::SetBannerArtKit(GameObject* Go,uint32 uiArtkit)
{
   Go->SetGoArtKit(uiArtkit);
   Go->Refresh();
}

void OutdoorPvPWG::UpdateCounterVehicle(bool init)
{
    if (init)
    {
        VehicleCountH = 0;
        VehicleCountA = 0;
    }
    VehicleCountMaxH = 0;
    VehicleCountMaxA = 0;

    for (std::list<WorldPvPWGWorkShopData*>::iterator itr = WorkShopList.begin(); itr != WorkShopList.end(); ++itr)
    {
        if (WorldPvPWGWorkShopData * workshop = (*itr))
        {
            if (workshop->m_TeamControl == ALLIANCE)
                VehicleCountMaxA = VehicleCountMaxA + 4;
            else if (workshop->m_TeamControl == HORDE)
                VehicleCountMaxH = VehicleCountMaxH + 4;
        }
    }

    UpdateVehicleCountWG();
}

void OutdoorPvPWG::HandlePlayerKillInsideArea(Player* pPlayer, Unit* pVictim)
{
    UpdateAura(pPlayer);
}

// Others
void OutdoorPvPWG::DoCompleteOrIncrementAchievement(uint32 achievement, Player *player)
{
    AchievementEntry const* AE = GetAchievementStore()->LookupEntry(achievement);

    if (player)
        player->CompletedAchievement(AE);
}

void OutdoorPvPWG::CompleOneObjetiveQuest(Player* pPlayer,uint32 id)
{
    switch (id)
    {
        case 13222:
        case 13223:
            pPlayer->KilledMonsterCredit(31284,pPlayer->GetObjectGuid());
            break;
    }
}

void OutdoorPvPWG::AddDataWhenWin()
{
    if(GetDefender() == ALLIANCE)
    {
       if(!GetPlayersAlliance() == NULL)
       {
          CompleteQuest(GetPlayersAlliance(),13181);
          DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WIN_WG,GetPlayersAlliance());

          if(TimeBattle > 600 && TimeBattle - m_Timer < 600)
             DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WIN_WG,GetPlayersAlliance());
       }
    }
    else if(GetDefender() == HORDE)
    {
       if(!GetPlayersHorde() == NULL)
       {
          CompleteQuest(GetPlayersHorde(),13183);
          DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WIN_WG,GetPlayersHorde());

          if(TimeBattle > 600 && TimeBattle - m_Timer < 600)
             DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WIN_WG,GetPlayersHorde());
       }

    }
}

void OutdoorPvPWG::RecolatePlayers()
{
    if(GetDefender() == ALLIANCE)
    {
       if(!GetPlayersAlliance() == NULL)
          GetPlayersAlliance()->TeleportTo(571, 5345, 2842, 410, 3.14f);

       if(!GetPlayersHorde() == NULL)
          GetPlayersHorde()->TeleportTo(571, 5025.857422f, 3674.628906f, 362.737122f, 4.135169f);
    }
    else if(GetDefender() == HORDE)
    {

       if(!GetPlayersHorde() == NULL)
          GetPlayersHorde()->TeleportTo(571, 5345, 2842, 410, 3.14f);

        if(!GetPlayersAlliance() == NULL)
          GetPlayersAlliance()->TeleportTo(571, 5101.284f, 2186.564f, 373.549f, 3.812f);
    }
}

// World States
void OutdoorPvPWG::UpdateVehicleCountWG()
{
    SendUpdateWorldState(WS_VEHICLE_COUNT_H, VehicleCountH);
    SendUpdateWorldState(WS_VEHICLE_COUNT_MAX_H, VehicleCountMaxH);
    SendUpdateWorldState(WS_VEHICLE_COUNT_A, VehicleCountA);
    SendUpdateWorldState(WS_VEHICLE_COUNT_MAX_A, VehicleCountMaxA);
}

void OutdoorPvPWG::UpdateWSBuilding()
{
    for (GameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
    {
        SendUpdateWorldState((*itr)->m_WorldState,(*itr)->m_State);
    }
}

void OutdoorPvPWG::UpdateWSWorkShop()
{
    for (std::list<WorldPvPWGWorkShopData*>::iterator itr = WorkShopList.begin(); itr != WorkShopList.end(); ++itr)
    {
        SendUpdateWorldState((*itr)->m_WorldState,(*itr)->m_State);
    }
}

// Auras

void OutdoorPvPWG::UpdateTenacityStack()
{
    uint32 team = TEAM_NONE;
    uint32 allianceNum = CountPlayersAlliance();
    uint32 hordeNum = CountPlayersHorde();
    int32 newStack = 0;

    if (allianceNum && hordeNum)
    {
        if (allianceNum < hordeNum)
            newStack = int32((float(hordeNum) / float(allianceNum) - 1)*4); // positive, should cast on alliance
    } else if (allianceNum > hordeNum)
            newStack = int32((1 - float(allianceNum) / float(hordeNum))*4); // negative, should cast on horde

    if (newStack == m_tenacityStack)
        return;

    if (m_tenacityStack > 0 && newStack <= 0) // old buff was on alliance
        team = ALLIANCE;
    else if (m_tenacityStack < 0 && newStack >= 0) // old buff was on horde
        team = HORDE;

    m_tenacityStack = newStack;

    // Remove old buff
    if (team != TEAM_NONE)
    {
        if(team == ALLIANCE)
        {
          if(GetPlayersAlliance() != NULL)
            GetPlayersAlliance()->RemoveAurasDueToSpell(SPELL_TENACITY);

          for (std::list<ObjectGuid>::iterator itr = m_vehicleA.begin(); itr != m_vehicleA.end(); ++itr)
          {
              if(get_map)
              {
                  if(Creature* pCreature = m_Map->GetCreature((*itr)))
                     pCreature->RemoveAurasDueToSpell(SPELL_TENACITY_VEHICLE);
              }
          }
        }
        else
        {
         if(GetPlayersHorde() != NULL)
           GetPlayersHorde()->RemoveAurasDueToSpell(SPELL_TENACITY);

          for (std::list<ObjectGuid>::iterator itr = m_vehicleH.begin(); itr != m_vehicleH.end(); ++itr)
          {
              if(get_map)
              {
                  if(Creature* pCreature = m_Map->GetCreature((*itr)))
                     pCreature->RemoveAurasDueToSpell(SPELL_TENACITY_VEHICLE);
              }
          }
        }
    }

    // Apply new buff
    if (newStack)
    {
        team = newStack > 0 ? ALLIANCE : HORDE;
        if (newStack < 0)
            newStack = -newStack;
        if (newStack > 20)
            newStack = 20;

        if(team == ALLIANCE)
        {
          if(GetPlayersAlliance() != NULL)
          {
            if(SpellAuraHolderPtr pHolder = GetPlayersAlliance()->GetSpellAuraHolder(SPELL_TENACITY))
            {
               pHolder->SetStackAmount(newStack);
            }
          }

          for (std::list<ObjectGuid>::iterator itr = m_vehicleA.begin(); itr != m_vehicleA.end(); ++itr)
          {
              if(get_map)
              {
                  if(Creature* pCreature = m_Map->GetCreature((*itr)))
                  {
                     if(SpellAuraHolderPtr pHolder = pCreature->GetSpellAuraHolder(SPELL_TENACITY_VEHICLE))
                     {
                        pHolder->SetStackAmount(newStack);
                     }
                  }
              }
          }
        }
        else
        {
         if(GetPlayersHorde() != NULL)
         {
            if(SpellAuraHolderPtr pHolder = GetPlayersHorde()->GetSpellAuraHolder(SPELL_TENACITY))
            {
               pHolder->SetStackAmount(newStack);
            }
         }

          for (std::list<ObjectGuid>::iterator itr = m_vehicleH.begin(); itr != m_vehicleH.end(); ++itr)
          {
              if(get_map)
              {
                  if(Creature* pCreature = m_Map->GetCreature((*itr)))
                  {
                     if(SpellAuraHolderPtr pHolder = pCreature->GetSpellAuraHolder(SPELL_TENACITY_VEHICLE))
                     {
                        pHolder->SetStackAmount(newStack);
                     }
                  }
              }
          }
        }
    }
}

void OutdoorPvPWG::UpdateAura(Player* pPlayer)
{
       if(SpellAuraHolderPtr pHolder = pPlayer->GetSpellAuraHolder(SPELL_RECRUIT))
       {
          if(pHolder->GetStackAmount() >= 5)
          {
            pPlayer->RemoveAurasDueToSpell(SPELL_RECRUIT);
            pPlayer->CastSpell(pPlayer, SPELL_CORPORAL, true);
          }
          else
            pHolder->SetStackAmount(pHolder->GetStackAmount() + 1);
       }
       else if(SpellAuraHolderPtr pHolder = pPlayer->GetSpellAuraHolder(SPELL_CORPORAL))
       {
          if (pHolder->GetStackAmount() >= 5)
          {
            pPlayer->RemoveAurasDueToSpell(SPELL_CORPORAL);
            pPlayer->CastSpell(pPlayer, SPELL_LIEUTENANT, true);
          }
          else
            pHolder->SetStackAmount(pHolder->GetStackAmount() + 1);
       }
}

void OutdoorPvPWG::AddAuraResurrect(Player* pPlayer)
{
     if (pPlayer->GetTeam() == ALLIANCE && m_tenacityStack > 0 ||
         pPlayer->GetTeam() == HORDE && m_tenacityStack < 0)
     {
        if (!pPlayer->HasAura(SPELL_TENACITY))
            pPlayer->CastSpell(pPlayer, SPELL_TENACITY, true);

        int32 newStack = m_tenacityStack < 0 ? -m_tenacityStack : m_tenacityStack;
        if (newStack > 20)
            newStack = 20;

        if(SpellAuraHolderPtr pHolder = pPlayer->GetSpellAuraHolder(SPELL_TENACITY))
        {
           pHolder->SetStackAmount(newStack);
        }
     }
}

// GraveYard System

WorldSafeLocsEntry const* OutdoorPvPWG::GetClosestGraveYardWG(Player* player)
{
    WorldSafeLocsEntry const* good_entry = NULL;

    if(player->GetTeam() == ALLIANCE)
    {
      std::vector<uint8> nodes;
      for (uint8 i = 0; i < 6; ++i)
            nodes.push_back(i);

      // If so, select the closest node to place ghost on
      if (!nodes.empty())
      {
          float plr_x = player->GetPositionX();
          float plr_y = player->GetPositionY();

          float mindist = 99999999999999999999.0f;
          for (uint8 i = 0; i < nodes.size(); ++i)
          {
              if(GraveyardIdsAlliance[nodes[i]] != 0)
              {
                WorldSafeLocsEntry const*entry = sWorldSafeLocsStore.LookupEntry( GraveyardIdsAlliance[nodes[i]] );
                  if (!entry)
                      continue;
                float dist = (entry->x - plr_x)*(entry->x - plr_x)+(entry->y - plr_y)*(entry->y - plr_y);
                if (mindist > dist)
                {
                   mindist = dist;
                   good_entry = entry;
                }
              }
          }
          nodes.clear();
      }
    }
    else if(player->GetTeam() == HORDE)
    {
      std::vector<uint8> nodes;
      for (uint8 i = 0; i < 6; ++i)
            nodes.push_back(i);

      // If so, select the closest node to place ghost on
      if (!nodes.empty())
      {
          float plr_x = player->GetPositionX();
          float plr_y = player->GetPositionY();

          float mindist = 999999.0f;
          for (uint8 i = 0; i < nodes.size(); ++i)
          {
              if(GraveyardIdsHorde[nodes[i]] != 0)
              {
                WorldSafeLocsEntry const*entry = sWorldSafeLocsStore.LookupEntry( GraveyardIdsHorde[nodes[i]] );
                  if (!entry)
                      continue;
                float dist = (entry->x - plr_x)*(entry->x - plr_x)+(entry->y - plr_y)*(entry->y - plr_y);
                if (mindist > dist)
                {
                   mindist = dist;
                   good_entry = entry;
                }
              }
          }
          nodes.clear();
      }

    }

    return good_entry;

}

void OutdoorPvPWG::AddGraveYardWG(WorldPvPGraveYardWG* gy,uint32 faction)
{
    if(faction == ALLIANCE)
    {
        uint8 pos = 0;
        bool con = false;

        for (uint8 i = 0; i < 6; ++i)
        {
            if(GraveyardIdsAlliance[i] == 0 && !con)
            {
                pos = i;
                con = true;
            }
        }

        GraveyardIdsAlliance[pos] = gy->GetId();
        gyAllAlliance = gyAllAlliance + 1;
    }
    else if(faction == HORDE)
    {
        uint8 pos = 0;
        bool con = false;

        for (uint8 i = 0; i < 6; ++i)
        {
            if(GraveyardIdsHorde[i] == 0 && !con)
            {
                pos = i;
                con = true;
            }
        }

        GraveyardIdsHorde[pos] = gy->GetId();
        gyAllHorde = gyAllHorde + 1;
    }
}

void OutdoorPvPWG::DeleteGraveYardWG(WorldPvPGraveYardWG* gy,uint32 faction)
{
    uint32 id = gy->GetId();

    if(faction == ALLIANCE)
    {
        uint8 pos = 0;
        bool con = false;

        for (uint8 i = 0; i < 6; ++i)
        {
            if(GraveyardIdsAlliance[i] == id && !con)
            {
                pos = i;
                con = true;
            }
        }

        GraveyardIdsAlliance[pos] = 0;
        gyAllAlliance = gyAllAlliance - 1;
    }
    else if(faction == HORDE)
    {
        uint8 pos = 0;
        bool con = false;

        for (uint8 i = 0; i < 6; ++i)
        {
            if(GraveyardIdsHorde[i] == id && !con)
            {
                pos = i;
                con = true;
            }
        }

        GraveyardIdsHorde[pos] = 0;
        gyAllHorde = gyAllHorde -1;
    }
}

// Tower

void OutdoorPvPWG::AddDamagedTower(uint32 team)
{
    if (team == GetAttacker())
    {
        DamageTowerAtt = DamageTowerAtt + 1;
    }
    else if (team == GetDefender())
    {
        DamageTowerDef = DamageTowerDef + 1;
    }
}

void OutdoorPvPWG::AddBrokenTower(uint32 team, Player* pPlayer)
{
    if (team == GetAttacker())
    {
        DamageTowerAtt = DamageTowerAtt - 1;
        DestroyTowerAtt = DestroyTowerAtt + 1;

        if(GetAttacker() == ALLIANCE && pPlayer->GetTeam() == HORDE)
        {
            if(!GetPlayersAlliance() == NULL)
              GetPlayersAlliance()->RemoveAurasDueToSpell(SPELL_TOWER_CONTROL);
            if(!GetPlayersHorde() == NULL)
              GetPlayersHorde()->CastSpell(GetPlayersHorde(), SPELL_TOWER_CONTROL, true);

            CompleteQuest(pPlayer, 13539);
            DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WG_TOWER_DESTROY, pPlayer);
        }
        else if (GetAttacker() == HORDE && pPlayer->GetTeam() == ALLIANCE)
        {
            if(!GetPlayersHorde() == NULL)
             GetPlayersHorde()->RemoveAurasDueToSpell(SPELL_TOWER_CONTROL);
            if(!GetPlayersAlliance() == NULL)
             GetPlayersAlliance()->CastSpell(GetPlayersAlliance(), SPELL_TOWER_CONTROL, true);

             CompleteQuest(pPlayer, 13538);
             DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WG_TOWER_DESTROY, pPlayer);
        }
    }
    else
    {
        DamageTowerDef = DamageTowerDef - 1;
        DestroyTowerDef = DestroyTowerDef + 1;
    }
}

void OutdoorPvPWG::BrokenWallOrTower(uint32 team,Player* pPlayer)
{
    if (team == GetDefender())
    {
        if(GetAttacker() == ALLIANCE)
        {
          CompleOneObjetiveQuest(pPlayer, 13222);
        }
        else if(GetAttacker() == HORDE)
        {
          CompleOneObjetiveQuest(pPlayer, 13223);
        }
    }
}

// Vehicles

void OutdoorPvPWG::CreateVehicle(Creature* pCreature,uint32 npc_entry)
{
  if(pCreature->GetEntry() == NPC_DEMOLISHER_ENGINEER_A)
  {
      uint8 i = -1;

      switch (pCreature->GetGUIDLow())
      {
           case 530040:
               i = 0;
               break;
           case 530042:
               i = 1;
               break;
           case 530044:
               i = 2;
               break;
           case 530046:
               i = 3;
               break;
           case 530048:
               i = 4;
               break;
           case 530050:
               i = 5;
               break;
      }

       if(Creature* pVehicle = pCreature->SummonCreature(npc_entry,CoordVehicleSummon[i].x,CoordVehicleSummon[i].y,CoordVehicleSummon[i].z,CoordVehicleSummon[i].o,TEMPSUMMON_DEAD_DESPAWN,1))
           AddVehicle(pVehicle,ALLIANCE);

  }
  else if(pCreature->GetEntry() == NPC_DEMOLISHER_ENGINEER_H)
  {
      uint8 i = -1;

      switch (pCreature->GetGUIDLow())
      {
           case 530041:
               i = 0;
               break;
           case 530043:
               i = 1;
               break;
           case 530045:
               i = 2;
               break;
           case 530047:
               i = 3;
               break;
           case 530049:
               i = 4;
               break;
           case 530051:
               i = 5;
               break;
      }


       if(Creature* pVehicle = pCreature->SummonCreature(npc_entry,CoordVehicleSummon[i].x,CoordVehicleSummon[i].y,CoordVehicleSummon[i].z,CoordVehicleSummon[i].o,TEMPSUMMON_DEAD_DESPAWN,1))
          AddVehicle(pVehicle,HORDE);

   }
}

void OutdoorPvPWG::AddVehicle(Creature* pCreature,uint32 team)
{
    if(team == ALLIANCE)
    {
        pCreature->setFaction(NPC_FACTION_A);
        if(pCreature->GetEntry() == 28312)
        {
            if(VehicleKit* veh = pCreature->GetVehicleKit())
            {
                pCreature->GetVehicleKit()->RemoveAllPassengers();
                pCreature->GetVehicleKit()->InstallAllAccessories(pCreature->GetEntry());
            }
        }
        VehicleCountA = VehicleCountA + 1;
        m_vehicleA.push_back(pCreature->GetObjectGuid());
    }
    else if(team == HORDE)
    {
        pCreature->setFaction(NPC_FACTION_H);
        if(pCreature->GetEntry() == 32627)
        {
            if(VehicleKit* veh = pCreature->GetVehicleKit())
            {
                pCreature->GetVehicleKit()->RemoveAllPassengers();
                pCreature->GetVehicleKit()->InstallAllAccessories(pCreature->GetEntry());
            }
        }
        VehicleCountH = VehicleCountH + 1;
        m_vehicleH.push_back(pCreature->GetObjectGuid());
    }

    UpdateVehicleCountWG();
}

void OutdoorPvPWG::DeleteVehicle(Creature* pCreature,uint32 team)
{
    if(team == ALLIANCE)
        VehicleCountA = VehicleCountA - 1;
    else if(team == HORDE)
        VehicleCountH = VehicleCountH - 1;

    UpdateVehicleCountWG();
}

uint32 OutdoorPvPWG::GetCountVehicle(uint32 team)
{
    uint32 count = NULL;

    if(team == ALLIANCE)
        count = VehicleCountA;
    else if(team == HORDE)
        count = VehicleCountH;

    return count;
}

uint32 OutdoorPvPWG::GetCountMaxVehicle(uint32 team)
{
    uint32 count = NULL;

    if(team == ALLIANCE)
        count = VehicleCountMaxA;
    else if(team == HORDE)
        count = VehicleCountMaxH;

    return count;
}

// Players

Player* OutdoorPvPWG::GetPlayerInZone()
{
    for (GuidSet::iterator itr = m_sZonePlayers.begin(); itr != m_sZonePlayers.end(); ++itr)
    {
        if (!(*itr))
            continue;

        Player* pPlayer = sObjectMgr.GetPlayer(*itr);

        if (!pPlayer)
            continue;
    }

    return NULL;
}

Player* OutdoorPvPWG::GetPlayersAlliance()
{
    Player* pPlayers = NULL;

    for (GuidSet::iterator itr = m_sZonePlayersAlliance.begin(); itr != m_sZonePlayersAlliance.end(); ++itr)
    {
        Player* pPlayer = sObjectMgr.GetPlayer(*itr);
		pPlayers = pPlayer;
    }
    
	return pPlayers;
}

Player* OutdoorPvPWG::GetPlayersHorde()
{
    Player* pPlayers = NULL;

    for (GuidSet::iterator itr = m_sZonePlayersHorde.begin(); itr != m_sZonePlayersHorde.end(); ++itr)
    {
        Player* pPlayer = sObjectMgr.GetPlayer(*itr);
		pPlayers = pPlayer;
    }

	return pPlayers;
}

uint32 OutdoorPvPWG::CountPlayersAlliance()
{
  uint32 count = 0;

    for (GuidSet::iterator itr = m_sZonePlayersAlliance.begin(); itr != m_sZonePlayersAlliance.end(); ++itr)
      count = count + 1;

  return count;
}

uint32 OutdoorPvPWG::CountPlayersHorde()
{
  uint32 count = 0;

    for (GuidSet::iterator itr = m_sZonePlayersHorde.begin(); itr != m_sZonePlayersHorde.end(); ++itr)
       count = count + 1;

  return count;
}