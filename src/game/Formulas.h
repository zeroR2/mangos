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

#ifndef MANGOS_FORMULAS_H
#define MANGOS_FORMULAS_H

#include "World.h"

namespace MaNGOS
{
    namespace Honor
    {
        inline float hk_honor_at_level(uint32 level, uint32 count = 1)
        {
            return (float)ceil(count * (-0.53177f + 0.59357f * exp((level + 23.54042f) / 26.07859f)));
        }

        inline float DishonorableKillPoints(int level)
        {
            float result = 10;
            if(level >= 30 && level <= 35)
                result = result + 1.5 * (level - 29);
            if(level >= 36 && level <= 41)
                result = result + 9 + 2 * (level - 35);
            if(level >= 42 && level <= 50)
                result = result + 21 + 3.2 * (level - 41);
            if(level >= 51)
                result = result + 50 + 4 * (level - 50);
            if(result > 100)
                return 100.0;
            else
                return result;
        }

        inline float HonorableKillPoints( Player *killer, Player *victim, uint32 groupsize)
        {
            if (!killer || !victim || !groupsize)
                return 0.0;

            int total_kills  = killer->CalculateTotalKills(victim);
            float f;
            if(total_kills < 10) f = (10 - (float)total_kills)*0.1;
            else f = 0.0;

            uint32 k_level  = killer->getLevel();
            uint32 v_level  = victim->getLevel();
            uint32 k_grey   = MaNGOS::XP::GetGrayLevel(k_level);

            float diff_level;
            if((k_level - k_grey) != 0)
                diff_level = ((float)(v_level - k_grey)) / ((float)(k_level - k_grey));
            else return 999;	//Check, in character_honor we will see, if working incorrectly

            if(diff_level > 2.0) diff_level = 2.0;
            if(diff_level <= 0.0) return 888; //For Debug

            int32 v_rank = (victim->GetHonorRank() - 4); //We cut all up to the first positive rank
            if(v_rank < 0) v_rank = 0;
            if(v_rank < 0) return 777;

            float honor_points = f * diff_level * (190 + v_rank*10);
            //float honor_points = f * diff_level * (190);
            honor_points *= ((float)k_level) / 60.0; //factor of dependence on levels of the killer

            if(honor_points > 660) return 666; //For Debug
            else return honor_points / groupsize;
        }
    }

    namespace XP
    {
        enum XPColorChar { RED, ORANGE, YELLOW, GREEN, GRAY };

        inline uint32 GetGrayLevel(uint32 pl_level)
        {
            if (pl_level <= 5)
                return 0;
            else if (pl_level <= 39)
                return pl_level - 5 - pl_level / 10;
            else if (pl_level <= 59)
                return pl_level - 1 - pl_level / 5;
            else
                return pl_level - 9;
        }

        inline XPColorChar GetColorCode(uint32 pl_level, uint32 mob_level)
        {
            if (mob_level >= pl_level + 5)
                return RED;
            else if (mob_level >= pl_level + 3)
                return ORANGE;
            else if (mob_level >= pl_level - 2)
                return YELLOW;
            else if (mob_level > GetGrayLevel(pl_level))
                return GREEN;
            else
                return GRAY;
        }

        inline uint32 GetZeroDifference(uint32 pl_level)
        {
            if (pl_level < 8)  return 5;
            if (pl_level < 10) return 6;
            if (pl_level < 12) return 7;
            if (pl_level < 16) return 8;
            if (pl_level < 20) return 9;
            if (pl_level < 30) return 11;
            if (pl_level < 40) return 12;
            if (pl_level < 45) return 13;
            if (pl_level < 50) return 14;
            if (pl_level < 55) return 15;
            if (pl_level < 60) return 16;
            return 17;
        }

        inline uint32 BaseGain(uint32 pl_level, uint32 mob_level)
        {
            uint32 nBaseExp = 45;

            if (mob_level >= pl_level)
            {
                uint32 nLevelDiff = mob_level - pl_level;
                if (nLevelDiff > 4)
                    nLevelDiff = 4;
                return ((pl_level * 5 + nBaseExp) * (20 + nLevelDiff) / 10 + 1) / 2;
            }
            else
            {
                uint32 gray_level = GetGrayLevel(pl_level);
                if (mob_level > gray_level)
                {
                    uint32 ZD = GetZeroDifference(pl_level);
                    return (pl_level * 5 + nBaseExp) * (ZD + mob_level - pl_level) / ZD;
                }
                return 0;
            }
        }

        inline uint32 Gain(Player* pl, Unit* u)
        {
            if (u->GetTypeId() == TYPEID_UNIT && (
                        ((Creature*)u)->IsTotem() || ((Creature*)u)->IsPet() ||
                        (((Creature*)u)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_XP_AT_KILL)))
                return 0;

            uint32 xp_gain = BaseGain(pl->getLevel(), u->getLevel());
            if (xp_gain == 0)
                return 0;

            if (u->GetTypeId() == TYPEID_UNIT && ((Creature*)u)->IsElite())
                xp_gain *= 2;

            return (uint32)(xp_gain * sWorld.getConfig(CONFIG_FLOAT_RATE_XP_KILL));
        }

        inline float xp_in_group_rate(uint32 count, bool isRaid)
        {
            if (isRaid)
            {
                // FIX ME: must apply decrease modifiers dependent from raid size
                return 1.0f;
            }
            else
            {
                switch (count)
                {
                    case 0:
                    case 1:
                    case 2:
                        return 1.0f;
                    case 3:
                        return 1.166f;
                    case 4:
                        return 1.3f;
                    case 5:
                    default:
                        return 1.4f;
                }
            }
        }
    }
}
#endif
