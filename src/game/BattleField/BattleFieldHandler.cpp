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

#include "Common.h"
#include "../ObjectAccessor.h"
#include "../ObjectMgr.h"
#include "WorldPacket.h"
#include "../WorldSession.h"
#include "../Opcodes.h"
#include "BattleFieldMgr.h"
#include "BattleField.h"

//This send to player windows for invite player to join the war
void WorldSession::SendBattleFieldInvitePlayerToBattle(uint32 BattleId,uint32 ZoneId,uint32 p_time)
{
    //Send packet
    WorldPacket data(SMSG_BATTLEFIELD_MANAGER_ENTRY_INVITE, 12);
    data << uint32(BattleId);
    data << uint32(ZoneId);
    data << uint32((time(NULL)+p_time));

    //Sending the packet to player
    SendPacket(&data);
}

//This send invitation to player to join the queue
void WorldSession::SendBattleFieldInvitePlayerToQueue(uint32 BattleId)
{
    WorldPacket data(SMSG_BATTLEFIELD_MANAGER_QUEUE_INVITE, 5);

    data << uint32(BattleId);
    data << uint8(1);

    //Sending packet to player
    SendPacket(&data);
}

//This send packet for inform player that he join queue
void WorldSession::SendBattleFieldQueueInviteResponce(uint32 BattleId,uint32 ZoneId)
{
    WorldPacket data(SMSG_BATTLEFIELD_MANAGER_QUEUE_REQUEST_RESPONSE, 11);
    data << uint32(BattleId);
    data << uint32(ZoneId);
    data << uint8(1);
    data << uint8(0);
    data << uint8(1);

    SendPacket(&data);
}

//This is call when player accept to join war
void WorldSession::SendBattleFieldEntered(uint32 BattleId)
{
    WorldPacket data(SMSG_BATTLEFIELD_MANAGER_ENTERING, 7);
    data << uint32(BattleId);
    data << uint8(1);
    data << uint8(1);
    data << uint8(_player->isAFK()?1:0);

    SendPacket(&data);
}

//Send when player leave battle
void WorldSession::SendBattleFieldLeaveMessage(uint32 BattleId)
{
    WorldPacket data(SMSG_BATTLEFIELD_MANAGER_EJECTED, 7);
    data << uint32(BattleId);
    data << uint8(8);
    data << uint8(2);
    data << uint8(0);

    SendPacket(&data);
}

void WorldSession::HandleBattleFieldQueueInviteResponseOpcode(WorldPacket &recv_data)
{
    uint32 BattleId;
    uint8 Accepted;

    recv_data >> BattleId >> Accepted;
    BattleField* Bf= sBattleFieldMgr.GetScriptById(BattleId);
    if(!Bf)
        return;

    if(Accepted)
        Bf->AcceptInvitePlayerToQueue(_player);
}

void WorldSession::HandleBattleFieldEntryInviteResponseOpcode(WorldPacket &recv_data)
{
    uint32 BattleId;
    uint8 Accepted;

   recv_data >> BattleId >> Accepted;
   BattleField* Bf= sBattleFieldMgr.GetScriptById(BattleId);
    if(!Bf)
        return;

    if(Accepted)
        Bf->AcceptInvitePlayerToBattle(_player);
}

void WorldSession::HandleBattleFieldExitRequestOpcode(WorldPacket &recv_data)
{

}
