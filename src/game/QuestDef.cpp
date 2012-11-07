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

#include "QuestDef.h"
#include "Player.h"
#include "World.h"
#include "DBCStores.h"

Quest::Quest(Field * questRecord)
{
    QuestId = questRecord[0].GetUInt32();
    QuestMethod = questRecord[1].GetUInt32();
    ZoneOrSort = questRecord[2].GetInt32();
    MinLevel = questRecord[3].GetUInt32();
    QuestLevel = questRecord[4].GetInt32();
    Type = questRecord[5].GetUInt32();
    RequiredClasses = questRecord[6].GetUInt32();
    RequiredRaces = questRecord[7].GetUInt32();
    RequiredSkill = questRecord[8].GetUInt32();
    RequiredSkillValue = questRecord[9].GetUInt32();
    RepObjectiveFaction = questRecord[10].GetUInt32();
    RepObjectiveValue = questRecord[11].GetInt32();
    RequiredMinRepFaction = questRecord[12].GetUInt32();
    RequiredMinRepValue = questRecord[13].GetInt32();
    RequiredMaxRepFaction = questRecord[14].GetUInt32();
    RequiredMaxRepValue = questRecord[15].GetInt32();
    SuggestedPlayers = questRecord[16].GetUInt32();
    LimitTime = questRecord[17].GetUInt32();
    m_QuestFlags = questRecord[18].GetUInt16();
    m_SpecialFlags = questRecord[19].GetUInt16();
    CharTitleId = questRecord[20].GetUInt32();
    PlayersSlain = questRecord[21].GetUInt32();
    BonusTalents = questRecord[22].GetUInt32();
    PrevQuestId = questRecord[23].GetInt32();
    NextQuestId = questRecord[24].GetInt32();
    ExclusiveGroup = questRecord[25].GetInt32();
    NextQuestInChain = questRecord[26].GetUInt32();
    RewXPId = questRecord[27].GetUInt32();
    SrcItemId = questRecord[28].GetUInt32();
    SrcItemCount = questRecord[29].GetUInt32();
    SrcSpell = questRecord[30].GetUInt32();
    Title = questRecord[31].GetCppString();
    Details = questRecord[32].GetCppString();
    Objectives = questRecord[33].GetCppString();
    OfferRewardText = questRecord[34].GetCppString();
    RequestItemsText = questRecord[35].GetCppString();
    EndText = questRecord[36].GetCppString();
    CompletedText = questRecord[37].GetCppString();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ObjectiveText[i] = questRecord[38+i].GetCppString();

    for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
        ReqItemId[i] = questRecord[42+i].GetUInt32();

    for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
        ReqItemCount[i] = questRecord[48+i].GetUInt32();

    for (int i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        ReqSourceId[i] = questRecord[54+i].GetUInt32();

    for (int i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        ReqSourceCount[i] = questRecord[58+i].GetUInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqCreatureOrGOId[i] = questRecord[62+i].GetInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqCreatureOrGOCount[i] = questRecord[66+i].GetUInt32();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        ReqSpell[i] = questRecord[70+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        RewChoiceItemId[i] = questRecord[74+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        RewChoiceItemCount[i] = questRecord[80+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARDS_COUNT; ++i)
        RewItemId[i] = questRecord[86+i].GetUInt32();

    for (int i = 0; i < QUEST_REWARDS_COUNT; ++i)
        RewItemCount[i] = questRecord[90+i].GetUInt32();

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
        RewRepFaction[i] = questRecord[94+i].GetUInt32();

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
        RewRepValueId[i] = questRecord[99+i].GetInt32();

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
        RewRepValue[i] = questRecord[104+i].GetInt32();

    RewHonorAddition = questRecord[109].GetUInt32();
    RewHonorMultiplier = questRecord[110].GetFloat();
    RewOrReqMoney = questRecord[111].GetInt32();
    RewMoneyMaxLevel = questRecord[112].GetUInt32();
    RewSpell = questRecord[113].GetUInt32();
    RewSpellCast = questRecord[114].GetUInt32();
    RewMailTemplateId = questRecord[115].GetUInt32();
    RewMailDelaySecs = questRecord[116].GetUInt32();
    PointMapId = questRecord[117].GetUInt32();
    PointX = questRecord[118].GetFloat();
    PointY = questRecord[119].GetFloat();
    PointOpt = questRecord[120].GetUInt32();

    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
        DetailsEmote[i] = questRecord[121+i].GetUInt32();

    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
        DetailsEmoteDelay[i] = questRecord[125+i].GetUInt32();

    IncompleteEmote = questRecord[129].GetUInt32();
    CompleteEmote = questRecord[130].GetUInt32();

    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
        OfferRewardEmote[i] = questRecord[131+i].GetInt32();

    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
        OfferRewardEmoteDelay[i] = questRecord[135+i].GetInt32();

    QuestStartScript = questRecord[139].GetUInt32();
    QuestCompleteScript = questRecord[140].GetUInt32();

    m_isActive = true;

    m_reqitemscount = 0;
    m_reqCreatureOrGOcount = 0;
    m_rewitemscount = 0;
    m_rewchoiceitemscount = 0;

    for (int i=0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
    {
        if ( ReqItemId[i] )
            ++m_reqitemscount;
    }

    for (int i=0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        if ( ReqCreatureOrGOId[i] )
            ++m_reqCreatureOrGOcount;
    }

    for (int i=0; i < QUEST_REWARDS_COUNT; ++i)
    {
        if ( RewItemId[i] )
            ++m_rewitemscount;
    }

    for (int i=0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
    {
        if (RewChoiceItemId[i])
            ++m_rewchoiceitemscount;
    }
}

uint32 Quest::XPValue(Player *pPlayer) const
{
    if( pPlayer )
    {
        if( RewMoneyMaxLevel > 0 )
        {
            uint32 pLevel = pPlayer->getLevel();
            uint32 qLevel = QuestLevel;
            float fullxp = 0;
            if (qLevel >= 65)
                fullxp = RewMoneyMaxLevel / 6.0f;
            else if (qLevel == 64)
                fullxp = RewMoneyMaxLevel / 4.8f;
            else if (qLevel == 63)
                fullxp = RewMoneyMaxLevel / 3.6f;
            else if (qLevel == 62)
                fullxp = RewMoneyMaxLevel / 2.4f;
            else if (qLevel == 61)
                fullxp = RewMoneyMaxLevel / 1.2f;
            else if (qLevel > 0 && qLevel <= 60)
                fullxp = RewMoneyMaxLevel / 0.6f;

            if( pLevel <= qLevel +  5 )
                return uint32(ceilf(fullxp));
            else if( pLevel == qLevel +  6 )
                return uint32(ceilf(fullxp * 0.8f));
            else if( pLevel == qLevel +  7 )
                return uint32(ceilf(fullxp * 0.6f));
            else if( pLevel == qLevel +  8 )
                return uint32(ceilf(fullxp * 0.4f));
            else if( pLevel == qLevel +  9 )
                return uint32(ceilf(fullxp * 0.2f));
            else
                return uint32(ceilf(fullxp * 0.1f));
        }
    }
    return 0;
}

int32 Quest::GetRewOrReqMoney() const
{
    if (RewOrReqMoney <=0)
        return RewOrReqMoney;

    return int32(RewOrReqMoney * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY));
}

uint32 Quest::GetRewMoneyMaxLevel() const
{
    if (RewMoneyMaxLevel < abs(RewOrReqMoney))
        return uint32(RewOrReqMoney * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY));

    return uint32(RewMoneyMaxLevel * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY));
}

bool Quest::IsAllowedInRaid() const
{
    if (Type == QUEST_TYPE_RAID || Type == QUEST_TYPE_RAID_10 || Type == QUEST_TYPE_RAID_25)
        return true;

    return sWorld.getConfig(CONFIG_BOOL_QUEST_IGNORE_RAID);
}
