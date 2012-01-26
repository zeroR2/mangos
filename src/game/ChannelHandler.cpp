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

#include "ObjectMgr.h"                                      // for normalizePlayerName
#include "ChannelMgr.h"

void WorldSession::HandleJoinChannelOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());

    uint32 channel_id;
    uint8 unknown1, unknown2;
    std::string channelname, pass;

    recvPacket >> channel_id >> unknown1 >> unknown2;
    recvPacket >> channelname;

    if(channelname.empty())
        return;

    recvPacket >> pass;
    // don't allow creating channels starting with a number (triggers a client-side bug)
    if (isdigit((unsigned char)channelname[0]))
    {
        WorldPacket data(SMSG_CHANNEL_NOTIFY, 1+channelname.size()+1);
        data << uint8(CHAT_INVALID_NAME_NOTICE);
        data << channelname;
        SendPacket(&data);
        return;
    }

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetJoinChannel(channelname, channel_id))
            chn->Join(GetPlayer()->GetObjectGuid(), pass.c_str());
}

void WorldSession::HandleLeaveChannelOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();

    uint32 unk;
    std::string channelname;
    recvPacket >> unk;                                      // channel id?
    recvPacket >> channelname;

    if(channelname.empty())
        return;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
    {
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->Leave(GetPlayer()->GetObjectGuid(), true);
        cMgr->LeftChannel(channelname);
    }
}

void WorldSession::HandleChannelListOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname;
    recvPacket >> channelname;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->List(GetPlayer());
}

void WorldSession::HandleChannelPasswordOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, pass;
    recvPacket >> channelname;

    recvPacket >> pass;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->Password(GetPlayer()->GetObjectGuid(), pass.c_str());
}

void WorldSession::HandleChannelSetOwnerOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, newp;
    recvPacket >> channelname;

    recvPacket >> newp;

    if(!normalizePlayerName(newp))
        return;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->SetOwner(GetPlayer()->GetObjectGuid(), newp.c_str());
}

void WorldSession::HandleChannelOwnerOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname;
    recvPacket >> channelname;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->SendWhoOwner(GetPlayer()->GetObjectGuid());
}

void WorldSession::HandleChannelModeratorOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if(!normalizePlayerName(otp))
        return;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->SetModerator(GetPlayer()->GetObjectGuid(), otp.c_str());
}

void WorldSession::HandleChannelUnmoderatorOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if(!normalizePlayerName(otp))
        return;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->UnsetModerator(GetPlayer()->GetObjectGuid(), otp.c_str());
}

void WorldSession::HandleChannelMuteOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if(!normalizePlayerName(otp))
        return;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->SetMute(GetPlayer()->GetObjectGuid(), otp.c_str());
}

void WorldSession::HandleChannelUnmuteOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();

    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if(!normalizePlayerName(otp))
        return;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->UnsetMute(GetPlayer()->GetObjectGuid(), otp.c_str());
}

void WorldSession::HandleChannelInviteOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if(!normalizePlayerName(otp))
        return;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->Invite(GetPlayer()->GetObjectGuid(), otp.c_str());
}

void WorldSession::HandleChannelKickOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;
    if(!normalizePlayerName(otp))
        return;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->Kick(GetPlayer()->GetObjectGuid(), otp.c_str());
}

void WorldSession::HandleChannelBanOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if(!normalizePlayerName(otp))
        return;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->Ban(GetPlayer()->GetObjectGuid(), otp.c_str());
}

void WorldSession::HandleChannelUnbanOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();

    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if(!normalizePlayerName(otp))
        return;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->UnBan(GetPlayer()->GetObjectGuid(), otp.c_str());
}

void WorldSession::HandleChannelAnnouncementsOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname;
    recvPacket >> channelname;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->Announce(GetPlayer()->GetObjectGuid());
}

void WorldSession::HandleChannelModerateOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname;
    recvPacket >> channelname;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->Moderate(GetPlayer()->GetObjectGuid());
}

void WorldSession::HandleChannelDisplayListQueryOpcode(WorldPacket &recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname;
    recvPacket >> channelname;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->List(GetPlayer());
}

void WorldSession::HandleGetChannelMemberCountOpcode(WorldPacket &recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname;
    recvPacket >> channelname;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
    {
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
        {
            WorldPacket data(SMSG_CHANNEL_MEMBER_COUNT, chn->GetName().size()+1+1+4);
            data << chn->GetName();
            data << uint8(chn->GetFlags());
            data << uint32(chn->GetNumPlayers());
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleSetChannelWatchOpcode(WorldPacket &recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname;
    recvPacket >> channelname;
    /*if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname, GetPlayer()))
            chn->JoinNotify(GetPlayer()->GetObjectGuid());*/
}
