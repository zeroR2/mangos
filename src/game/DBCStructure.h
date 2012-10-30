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

#ifndef MANGOS_DBCSTRUCTURE_H
#define MANGOS_DBCSTRUCTURE_H

#include "Common.h"
#include "DBCEnums.h"
#include "Path.h"
#include "Platform/Define.h"
#include "SpellClassMask.h"
#include "SharedDefines.h"

#include <map>
#include <set>
#include <vector>

// Structures using to access raw DBC data and required packing to portability

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

struct AreaTableEntry
{
    uint32  ID;                                             // 0        m_ID
    uint32  mapid;                                          // 1        m_ContinentID
    uint32  zone;                                           // 2        m_ParentAreaID
    uint32  exploreFlag;                                    // 3        m_AreaBit
    uint32  flags;                                          // 4        m_flags
                                                            // 5        m_SoundProviderPref
                                                            // 6        m_SoundProviderPrefUnderwater
                                                            // 7        m_AmbienceID
                                                            // 8        m_ZoneMusic
                                                            // 9        m_IntroSound
    uint32    area_level;                                   // 10
    char*     area_name[8];                                 // 11-18 
    uint32    team;                                         // 20
};

struct AreaTriggerEntry
{
    uint32  id;                                             // 0        m_ID
    uint32  mapid;                                          // 1        m_ContinentID
    float   x;                                              // 2        m_x
    float   y;                                              // 3        m_y
    float   z;                                              // 4        m_z
    float   radius;                                         // 5        m_radius
    float   box_x;                                          // 6        m_box_length
    float   box_y;                                          // 7        m_box_width
    float   box_z;                                          // 8        m_box_heigh
    float   box_orientation;                                // 9        m_box_yaw
};

struct AuctionHouseEntry
{
    uint32    houseId;                                      // 0        m_ID
    uint32    faction;                                      // 1        m_factionID
    uint32    depositPercent;                               // 2        m_depositRate
    uint32    cutPercent;                                   // 3        m_consignmentRate
    //char*     name[8];                                    // 4-11     m_name_lang
                                                            // 12 string flags
};

struct BankBagSlotPricesEntry
{
    uint32  ID;                                             // 0        m_ID
    uint32  price;                                          // 1        m_Cost
};

#define MAX_OUTFIT_ITEMS 12

struct CharStartOutfitEntry
{
    //uint32 Id;                                            // 0        m_ID
    uint32 RaceClassGender;                                 // 1        m_raceID m_classID m_sexID m_outfitID (UNIT_FIELD_BYTES_0 & 0x00FFFFFF) comparable (0 byte = race, 1 byte = class, 2 byte = gender)
    int32 ItemId[MAX_OUTFIT_ITEMS];                         // 2-25     m_ItemID
    //int32 ItemDisplayId[MAX_OUTFIT_ITEMS];                // 26-29    m_DisplayItemID not required at server side
    //int32 ItemInventorySlot[MAX_OUTFIT_ITEMS];            // 50-73    m_InventoryType not required at server side
    //uint32 Unknown1;                                      // 74 unique values (index-like with gaps ordered in other way as ids)
    //uint32 Unknown2;                                      // 75
    //uint32 Unknown3;                                      // 76
};

struct ChatChannelsEntry
{
    uint32  ChannelID;                                      // 0        m_ID
    uint32  flags;                                          // 1        m_flags
                                                            // 2        m_factionGroup
    char*   pattern[8];                                     // 3-10     m_name_lang
                                                            // 11 string flags
    //char*       name[8];                                  // 12-19    m_shortcut_lang
                                                            // 20 string flags
};

struct ChrClassesEntry
{
    uint32  ClassID;                                        // 0        m_ID
    //uint32 unk1;                                          // 1 unknown, all 1
    //uint32 flags;                                         // 2 unknown
    uint32  powerType;                                      // 3        m_DisplayPower
                                                            // 4        m_petNameToken
    char const* name[8];                                    // 5-12     m_name_lang
                                                            // 13 string flags                                                        // 14       m_filename
    uint32  spellfamily;                                    // 15       m_spellClassSet
    //uint32 flags2;                                        // 16       m_flags (0x1 HasRelicSlot)
};

struct ChrRacesEntry
{
    uint32      RaceID;                                     // 0        m_ID
                                                            // 1        m_flags
    uint32      FactionID;                                  // 2        m_factionID
                                                            // 3        m_ExplorationSoundID
    uint32      model_m;                                    // 4        m_MaleDisplayId
    uint32      model_f;                                    // 5        m_FemaleDisplayId
                                                            // 6        m_ClientPrefix
                                                            // 7        unused
    uint32      TeamID;                                     // 8        m_BaseLanguage (7-Alliance 1-Horde)
                                                            // 9        m_creatureType
                                                            // 10       unused, all 836
                                                            // 11       unused, all 1604
                                                            // 12       m_ResSicknessSpellID
                                                            // 13       m_SplashSoundID
    uint32      startingTaxiMask;                           // 14
                                                            // 15       m_clientFileString
    uint32      CinematicSequence;                          // 16       m_cinematicSequenceID
    char*       name[8];                                    // 17-24    m_name_lang used for DBC language detection/selection
                                                            // 25 string flags
                                                            // 26-27    m_facialHairCustomization[2]
                                                            // 28       m_hairCustomization
};


struct CinematicSequencesEntry
{
    uint32      Id;                                         // 0        m_ID
    //uint32      unk1;                                     // 1        m_soundID
    //uint32      cinematicCamera;                          // 2        m_camera[8]
};

struct CreatureDisplayInfoEntry
{
    uint32      Displayid;                                  // 0        m_ID
    uint32      ModelId;                                    // 1        m_modelID
                                                            // 2        m_soundID
    uint32      ExtendedDisplayInfoID;                      // 3        m_extendedDisplayInfoID -> CreatureDisplayInfoExtraEntry::DisplayExtraId
    float       scale;                                      // 4        m_creatureModelScale
                                                            // 5        m_creatureModelAlpha
                                                            // 6-8      m_textureVariation[3]
                                                            // 9        m_portraitTextureName
                                                            // 10       m_sizeClass
                                                            // 11       m_bloodID
                                                            // 12       m_NPCSoundID
};

struct CreatureDisplayInfoExtraEntry
{
    uint32      DisplayExtraId;                             // 0        m_ID CreatureDisplayInfoEntry::m_extendedDisplayInfoID
    uint32      Race;                                       // 1        m_DisplayRaceID
    //uint32    Gender;                                     // 2        m_DisplaySexID
    //uint32    SkinColor;                                  // 3        m_SkinID
    //uint32    FaceType;                                   // 4        m_FaceID
    //uint32    HairType;                                   // 5        m_HairStyleID
    //uint32    HairStyle;                                  // 6        m_HairColorID
    //uint32    BeardStyle;                                 // 7        m_FacialHairID
    //uint32    Equipment[10];                              // 8-17     m_NPCItemDisplay equipped static items EQUIPMENT_SLOT_HEAD..EQUIPMENT_SLOT_HANDS, client show its by self
    //char*                                                 // 18       m_BakeName CreatureDisplayExtra-*.blp
};

struct CreatureFamilyEntry
{
    uint32    ID;                                           // 0
    float     minScale;                                     // 1
    uint32    minScaleLevel;                                // 2 0/1
    float     maxScale;                                     // 3
    uint32    maxScaleLevel;                                // 4 0/60
    uint32    skillLine[2];                                 // 5-6
    uint32    petFoodMask;                                  // 7
    char*     Name[8];
};

#define MAX_CREATURE_SPELL_DATA_SLOT 4

struct CreatureSpellDataEntry
{
    uint32    ID;                                           // 0        m_ID
    uint32    spellId[MAX_CREATURE_SPELL_DATA_SLOT];        // 1-4      m_spells[4]
    //uint32    availability[MAX_CREATURE_SPELL_DATA_SLOT]; // 4-7      m_availability[4]
};

struct CreatureTypeEntry
{
    uint32    ID;                                           // 0        m_ID
    //char*   Name[8];                                      // 1-8      m_name_lang
                                                            // 9 string flags

    //uint32    no_expirience;                              // 10       m_flags
};

struct DurabilityCostsEntry
{
    uint32    Itemlvl;                                      // 0        m_ID
    uint32    multiplier[29];                               // 1-29     m_weaponSubClassCost m_armorSubClassCost
};

struct DurabilityQualityEntry
{
    uint32    Id;                                           // 0        m_ID
    float     quality_mod;                                  // 1        m_data
};

struct EmotesEntry
{
    uint32  Id;                                             // 0        m_ID
    //char*   Name;                                         // 1        m_EmoteSlashCommand
    //uint32  AnimationId;                                  // 2        m_AnimID
    uint32  Flags;                                          // 3        m_EmoteFlags
    uint32  EmoteType;                                      // 4        m_EmoteSpecProc (determine how emote are shown)
    uint32  UnitStandState;                                 // 5        m_EmoteSpecProcParam
    //uint32  SoundId;                                      // 6        m_EventSoundID
};

struct EmotesTextEntry
{
    uint32  Id;                                             //          m_ID
                                                            //          m_name
    uint32  textid;                                         //          m_emoteID
                                                            //          m_emoteText
};

struct FactionEntry
{
    uint32      ID;                                         // 0        m_ID
    int32       reputationListID;                           // 1        m_reputationIndex
    uint32      BaseRepRaceMask[4];                         // 2-5      m_reputationRaceMask
    uint32      BaseRepClassMask[4];                        // 6-9      m_reputationClassMask
    int32       BaseRepValue[4];                            // 10-13    m_reputationBase
    uint32      ReputationFlags[4];                         // 14-17    m_reputationFlags
    uint32      team;                                       // 18       m_parentFactionID
    char*       name[8];                                    // 19-26    m_name_lang
                                                            // 27 string flags
    //char*     description[8];                             // 28-35    m_description_lang
                                                            // 36 string flags

    // helpers

    int GetIndexFitTo(uint32 raceMask, uint32 classMask) const
    {
        for (int i = 0; i < 4; ++i)
        {
            if ((BaseRepRaceMask[i] == 0 || (BaseRepRaceMask[i] & raceMask)) &&
                (BaseRepClassMask[i] == 0 || (BaseRepClassMask[i] & classMask)))
                return i;
        }

        return -1;
    }
};

struct FactionTemplateEntry
{
    uint32      ID;                                         // 0        m_ID
    uint32      faction;                                    // 1        m_faction
    uint32      factionFlags;                               // 2        m_flags
    uint32      ourMask;                                    // 3        m_factionGroup
    uint32      friendlyMask;                               // 4        m_friendGroup
    uint32      hostileMask;                                // 5        m_enemyGroup
    uint32      enemyFaction[4];                            // 6        m_enemies[4]
    uint32      friendFaction[4];                           // 10       m_friend[4]
    //-------------------------------------------------------  end structure

    // helpers
    bool IsFriendlyTo(FactionTemplateEntry const& entry) const
    {
        if(entry.faction)
        {
            for(int i = 0; i < 4; ++i)
                if (enemyFaction[i]  == entry.faction)
                    return false;
            for(int i = 0; i < 4; ++i)
                if (friendFaction[i] == entry.faction)
                    return true;
        }
        return (friendlyMask & entry.ourMask) || (ourMask & entry.friendlyMask);
    }
    bool IsHostileTo(FactionTemplateEntry const& entry) const
    {
        if(entry.faction)
        {
            for(int i = 0; i < 4; ++i)
                if (enemyFaction[i]  == entry.faction)
                    return true;
            for(int i = 0; i < 4; ++i)
                if (friendFaction[i] == entry.faction)
                    return false;
        }
        return (hostileMask & entry.ourMask) != 0;
    }
    bool IsHostileToPlayers() const { return (hostileMask & FACTION_MASK_PLAYER) !=0; }
    bool IsNeutralToAll() const
    {
        for(int i = 0; i < 4; ++i)
            if (enemyFaction[i] != 0)
                return false;
        return hostileMask == 0 && friendlyMask == 0;
    }
    bool IsContestedGuardFaction() const { return (factionFlags & FACTION_TEMPLATE_FLAG_CONTESTED_GUARD)!=0; }
};

// using bounds from 3.3.5a
struct GameObjectDisplayInfoEntry
{
    uint32      Displayid;                                  // 0        m_ID
    char* filename;                                         // 1        m_modelName
    // uint32 unknown2[10];                                 // 2-11     m_Sound
    float       minX;                                       // 12       m_geoBoxMinX (use first value as interact dist, mostly in hacks way)
    float       minY;                                       // 13       m_geoBoxMinY
    float       minZ;                                       // 14       m_geoBoxMinZ
    float       maxX;                                       // 15       m_geoBoxMaxX
    float       maxY;                                       // 16       m_geoBoxMaxY
    float       maxZ;                                       // 17       m_geoBoxMaxZ
    // uint32 unknown18;                                    // 18       m_objectEffectPackageID
};

// All Gt* DBC store data for 100 levels, some by 100 per class/race
#define GT_MAX_LEVEL    100

struct ItemBagFamilyEntry
{
    uint32   ID;                                            // 0        m_ID
    //char*     name[8]                                     // 1-8     m_name_lang
    //                                                      // 9        name flags
};

struct ItemDisplayInfoEntry
{
    uint32      ID;
    uint32      randomPropertyChance;
};
struct ItemRandomPropertiesEntry
{
    uint32    ID;                                           // 0        m_ID
    //char*     internalName                                // 1        m_Name
    uint32    enchant_id[3];                                // 2-4      m_Enchantment
                                                            // 5-6 unused, 0 only values, reserved for additional enchantments
    // char*     nameSuffix[8];                             // 7-14     m_name_lang
                                                            // 15 string flags

};

struct ItemSetEntry
{
    //uint32    id                                          // 0        m_ID
    char*     name[8];                                      // 1-8      m_name_lang
                                                            // 9 string flags
    //uint32    itemId[17];                                 // 10-26    m_itemID
    uint32    spells[8];                                    // 27-34    m_setSpellID
    uint32    items_to_triggerspell[8];                     // 35-42    m_setThreshold
    uint32    required_skill_id;                            // 43       m_requiredSkill
    uint32    required_skill_value;                         // 44       m_requiredSkillRank
};

// using from 3.3.5a
struct LiquidTypeEntry
{
    uint32 Id;                                              // 0
    //char*  Name;                                          // 1
    //uint32 Flags;                                         // 2            Water: 1|2|4|8, Magma: 8|16|32|64, Slime: 2|64|256, WMO Ocean: 1|2|4|8|512
    uint32 Type;                                            // 3            0: Water, 1: Ocean, 2: Magma, 3: Slime
    //uint32 SoundId;                                       // 4            Reference to SoundEntries.dbc
    uint32 SpellId;                                         // 5            Reference to Spell.dbc
    //float MaxDarkenDepth;                                 // 6            Only oceans got values here!
    //float FogDarkenIntensity;                             // 7            Only oceans got values here!
    //float AmbDarkenIntensity;                             // 8            Only oceans got values here!
    //float DirDarkenIntensity;                             // 9            Only oceans got values here!
    //uint32 LightID;                                       // 10           Only Slime (6) and Magma (7)
    //float ParticleScale;                                  // 11           0: Slime, 1: Water/Ocean, 4: Magma
    //uint32 ParticleMovement;                              // 12
    //uint32 ParticleTexSlots;                              // 13
    //uint32 LiquidMaterialID;                              // 14
    //char* Texture[6];                                     // 15-20
    //uint32 Color[2];                                      // 21-22
    //float Unk1[18];                                       // 23-40    Most likely these are attributes for the shaders. Water: (23, TextureTilesPerBlock),(24, Rotation) Magma: (23, AnimationX),(24, AnimationY)
    //uint32 Unk2[4];                                       // 41-44
};

#define MAX_LOCK_CASE 8

struct LockEntry
{
    uint32      ID;                                         // 0        m_ID
    uint32      Type[MAX_LOCK_CASE];                        // 1-8      m_Type
    uint32      Index[MAX_LOCK_CASE];                       // 9-16     m_Index
    uint32      Skill[MAX_LOCK_CASE];                       // 17-24    m_Skill
    //uint32      Action[MAX_LOCK_CASE];                    // 25-32    m_Action
};

struct MailTemplateEntry
{
    uint32      ID;                                         // 0        m_ID
    //char*       subject[8];                               // 1-8      m_subject_lang
                                                            // 9 string flags
};


struct MapEntry
{
    uint32  MapID;                                          // 0        m_ID
    //char*       internalname;                             // 1        m_Directory
    uint32  map_type;                                       // 2        m_InstanceType
    //uint32 isPvP;                                         // 3        m_PVP 0 or 1 for battlegrounds
    char*   name[8];                                        // 4-11     m_MapName_lang
                                                            // 12 string flags
                                                            // 13-15 unused (something PvPZone related - levels?)
                                                            // 16-18
    uint32  linked_zone;                                    // 19       m_areaTableID
    //char*     hordeIntro[8];                              // 20-27    m_MapDescription0_lang
                                                            // 28 string flags
    //char*     allianceIntro[8];                           // 29-36    m_MapDescription1_lang
                                                            // 37 string flags
    uint32  multimap_id;                                    // 38       m_LoadingScreenID (LoadingScreens.dbc)
                                                            // 39-40 not used
    //float   BattlefieldMapIconScale;                      // 41       m_minimapIconScale
    // Helpers

    bool IsDungeon() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID; }
    bool IsNonRaidDungeon() const { return map_type == MAP_INSTANCE; }
    bool Instanceable() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID || map_type == MAP_BATTLEGROUND; }
    bool IsRaid() const { return map_type == MAP_RAID; }
    bool IsBattleGround() const { return map_type == MAP_BATTLEGROUND; }

    bool IsMountAllowed() const
    {
        return !IsDungeon() ||
            MapID==309 || MapID==209 || MapID==509 || MapID==269;
    }

    bool IsContinent() const
    {
        return MapID == 0 || MapID == 1;
    }

    /*bool IsTransport() const
    {
        return map_type == MAP_COMMON && mapFlags == MAP_FLAG_INSTANCEABLE;
    }*/
};

struct QuestSortEntry
{
    uint32      id;                                         // 0        m_ID
    //char*       name[8];                                  // 1-8     m_SortName_lang
                                                            // 9 string flags
};

struct SkillRaceClassInfoEntry
{
    //uint32    id;                                         // 0        m_ID
    uint32    skillId;                                      // 1        m_skillID
    uint32    raceMask;                                     // 2        m_raceMask
    uint32    classMask;                                    // 3        m_classMask
    uint32    flags;                                        // 4        m_flags
    uint32    reqLevel;                                     // 5        m_minLevel
    //uint32    skillTierId;                                // 6        m_skillTierID
    //uint32    skillCostID;                                // 7        m_skillCostIndex
};

struct SkillLineEntry
{
    uint32    id;                                           // 0        m_ID
    int32     categoryId;                                   // 1        m_categoryID
    //uint32    skillCostID;                                // 2        m_skillCostsID
    char*     name[8];                                      // 3-10     m_displayName_lang
                                                            // 11 string flags
    //char*     description[8];                             // 12-19    m_description_lang
                                                            // 20 string flags
    uint32    spellIcon;                                    // 21       m_spellIconID
};

struct SkillLineAbilityEntry
{
    uint32    id;                                           // 0, INDEX
    uint32    skillId;                                      // 1
    uint32    spellId;                                      // 2
    uint32    racemask;                                     // 3
    uint32    classmask;                                    // 4
    //uint32    racemaskNot;                                // 5 always 0 in 2.4.2
    //uint32    classmaskNot;                               // 6 always 0 in 2.4.2
    uint32    req_skill_value;                              // 7 for trade skill.not for training.
    uint32    forward_spellid;                              // 8
    uint32    learnOnGetSkill;                              // 9 can be 1 or 2 for spells learned on get skill
    uint32    max_value;                                    // 10
    uint32    min_value;                                    // 11
                                                            // 12-13, unknown, always 0
    uint32    reqtrainpoints;                               // 14
};

struct SoundEntriesEntry
{
    uint32    Id;                                           // 0        m_ID
    //uint32    Type;                                       // 1        m_soundType
    //char*     InternalName;                               // 2        m_name
    //char*     FileName[10];                               // 3-12     m_File[10]
    //uint32    Unk13[10];                                  // 13-22    m_Freq[10]
    //char*     Path;                                       // 23       m_DirectoryBase
                                                            // 24       m_volumeFloat
                                                            // 25       m_flags
                                                            // 26       m_minDistance
                                                            // 27       m_distanceCutoff
                                                            // 28       m_EAXDef
};

// template arguments for declaration
#define CFM_ARGS_1  ClassFlag N1
#define CFM_ARGS_2  CFM_ARGS_1, ClassFlag N2
#define CFM_ARGS_3  CFM_ARGS_2, ClassFlag N3
#define CFM_ARGS_4  CFM_ARGS_3, ClassFlag N4
#define CFM_ARGS_5  CFM_ARGS_4, ClassFlag N5
#define CFM_ARGS_6  CFM_ARGS_5, ClassFlag N6
#define CFM_ARGS_7  CFM_ARGS_6, ClassFlag N7
#define CFM_ARGS_8  CFM_ARGS_7, ClassFlag N8
#define CFM_ARGS_9  CFM_ARGS_8, ClassFlag N9
#define CFM_ARGS_10 CFM_ARGS_9, ClassFlag N10

// template values for function calls
#define CFM_VALUES_1  N1
#define CFM_VALUES_2  CFM_VALUES_1, N2
#define CFM_VALUES_3  CFM_VALUES_2, N3
#define CFM_VALUES_4  CFM_VALUES_3, N4
#define CFM_VALUES_5  CFM_VALUES_4, N5
#define CFM_VALUES_6  CFM_VALUES_5, N6
#define CFM_VALUES_7  CFM_VALUES_6, N7
#define CFM_VALUES_8  CFM_VALUES_7, N8
#define CFM_VALUES_9  CFM_VALUES_8, N9
#define CFM_VALUES_10 CFM_VALUES_9, N10

struct ClassFamilyMask
{
    uint64 Flags;

    ClassFamilyMask() : Flags(0) {}
    explicit ClassFamilyMask(uint64 familyFlags) : Flags(familyFlags) {}

    // predefined empty object for safe return by reference
    static ClassFamilyMask const Null;

    bool Empty() const { return Flags == 0; }
    bool operator! () const { return Empty(); }
    operator void const* () const { return Empty() ? NULL : this; }// for allow normal use in if(mask)

    bool IsFitToFamilyMask(uint64 familyFlags) const
    {
        return (Flags & familyFlags);
    }

    bool IsFitToFamilyMask(ClassFamilyMask const& mask) const
    {
        return (Flags & mask.Flags);
    }

    uint64 operator& (uint64 mask) const                     // possible will removed at finish convertion code use IsFitToFamilyMask
    {
        return Flags & mask;
    }

    // test if specified bits are set (run-time)
    bool test(size_t offset) const
    {
        return reinterpret_cast<uint8 const*>(this)[offset >> 3] & (uint8(1) << (offset & 7));
    }

    // test if specified bits are set (compile-time)
    template <CFM_ARGS_1>
    bool test() const
    {
        return (Flags  & BitMask<uint64, true,  CFM_VALUES_1>::value);
    }

    template <CFM_ARGS_2>
    bool test() const
    {
        return (Flags  & BitMask<uint64, true,  CFM_VALUES_2>::value);
    }

    template <CFM_ARGS_3>
    bool test() const
    {
        return (Flags  & BitMask<uint64, true,  CFM_VALUES_3>::value);
    }

    template <CFM_ARGS_4>
    bool test() const
    {
        return (Flags  & BitMask<uint64, true,  CFM_VALUES_4>::value);
    }

    template <CFM_ARGS_5>
    bool test() const
    {
        return (Flags  & BitMask<uint64, true,  CFM_VALUES_5>::value);
    }

    template <CFM_ARGS_6>
    bool test() const
    {
        return (Flags  & BitMask<uint64, true,  CFM_VALUES_6>::value);
    }

    template <CFM_ARGS_7>
    bool test() const
    {
        return (Flags  & BitMask<uint64, true,  CFM_VALUES_7>::value);
    }

    template <CFM_ARGS_8>
    bool test() const
    {
        return (Flags  & BitMask<uint64, true,  CFM_VALUES_8>::value);
    }

    template <CFM_ARGS_9>
    bool test() const
    {
        return (Flags  & BitMask<uint64, true,  CFM_VALUES_9>::value);
    }

    template <CFM_ARGS_10>
    bool test() const
    {
        return (Flags  & BitMask<uint64, true,  CFM_VALUES_10>::value);
    }

    // named constructors (compile-time)
    template <CFM_ARGS_1>
    static ClassFamilyMask create()
    {
        return ClassFamilyMask(BitMask<uint64, true,  CFM_VALUES_1>::value);
    }

    template <CFM_ARGS_2>
    static ClassFamilyMask create()
    {
        return ClassFamilyMask(BitMask<uint64, true,  CFM_VALUES_2>::value);
    }

    template <CFM_ARGS_3>
    static ClassFamilyMask create()
    {
        return ClassFamilyMask(BitMask<uint64, true,  CFM_VALUES_3>::value);
    }

    template <CFM_ARGS_4>
    static ClassFamilyMask create()
    {
        return ClassFamilyMask(BitMask<uint64, true,  CFM_VALUES_4>::value);
    }

    template <CFM_ARGS_5>
    static ClassFamilyMask create()
    {
        return ClassFamilyMask(BitMask<uint64, true,  CFM_VALUES_5>::value);
    }

    template <CFM_ARGS_6>
    static ClassFamilyMask create()
    {
        return ClassFamilyMask(BitMask<uint64, true,  CFM_VALUES_6>::value);
    }

    template <CFM_ARGS_7>
    static ClassFamilyMask create()
    {
        return ClassFamilyMask(BitMask<uint64, true,  CFM_VALUES_7>::value);
    }

    template <CFM_ARGS_8>
    static ClassFamilyMask create()
    {
        return ClassFamilyMask(BitMask<uint64, true,  CFM_VALUES_8>::value);
    }

    template <CFM_ARGS_9>
    static ClassFamilyMask create()
    {
        return ClassFamilyMask(BitMask<uint64, true,  CFM_VALUES_9>::value);
    }

    template <CFM_ARGS_10>
    static ClassFamilyMask create()
    {
        return ClassFamilyMask(BitMask<uint64, true,  CFM_VALUES_10>::value);
    }

    // comparison operators
    bool operator== (ClassFamilyMask const& rhs) const
    {
        return Flags == rhs.Flags;
    }

    bool operator!= (ClassFamilyMask const& rhs) const
    {
        return Flags != rhs.Flags;
    }

    // bitwise operators
    ClassFamilyMask operator& (ClassFamilyMask const& rhs) const
    {
        return ClassFamilyMask(Flags & rhs.Flags);
    }

    ClassFamilyMask operator| (ClassFamilyMask const& rhs) const
    {
        return ClassFamilyMask(Flags | rhs.Flags);
    }

    ClassFamilyMask operator^ (ClassFamilyMask const& rhs) const
    {
        return ClassFamilyMask(Flags ^ rhs.Flags);
    }

    ClassFamilyMask operator~ () const
    {
        return ClassFamilyMask(~Flags);
    }

    // assignation operators
    ClassFamilyMask& operator= (ClassFamilyMask const& rhs)
    {
        Flags  = rhs.Flags;
        return *this;
    }

    ClassFamilyMask& operator&= (ClassFamilyMask const& rhs)
    {
        Flags  &= rhs.Flags;
        return *this;
    }

    ClassFamilyMask& operator|= (ClassFamilyMask const& rhs)
    {
        Flags  |= rhs.Flags;
        return *this;
    }

    ClassFamilyMask& operator^= (ClassFamilyMask const& rhs)
    {
        Flags  ^= rhs.Flags;
        return *this;
    }

    // templates used for compile-time mask calculation
private:
    enum { LOW_WORD_SIZE = 64 };

    template <typename T, int Val, bool IsLow, bool InRange>
    struct DoShift
    {
        static T const value = T(1) << Val;
    };

    template <typename T, int Val>
    struct DoShift<T, Val, false, true>
    {
        static T const value = T(1) << (Val - LOW_WORD_SIZE);
    };

    template <typename T, int Val, bool IsLow>
    struct DoShift<T, Val, IsLow, false>
    {
        static T const value = 0;
    };

    template <int N, bool IsLow>
    struct IsInRange
    {
        static bool const value = IsLow ? N < LOW_WORD_SIZE : N >= LOW_WORD_SIZE;
    };

    template <typename T, bool IsLow, int N1, int N2 = -1, int N3 = -1, int N4 = -1, int N5 = -1, int N6 = -1, int N7 = -1, int N8 = -1, int N9 = -1, int N10 = -1>
    struct BitMask
    {
        static T const value = DoShift<T, N1, IsLow, IsInRange<N1, IsLow>::value>::value | BitMask<T, IsLow, N2, N3, N4, N5, N6, N7, N8, N9, N10, -1>::value;
    };

    template <typename T, bool IsLow>
    struct BitMask<T, IsLow, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1>
    {
        static T const value = 0;
    };
};

struct MANGOS_DLL_SPEC SpellEntry;

struct SpellEffectEntry
{
    SpellEffectEntry(SpellEntry const* spellEntry, SpellEffectIndex i);
    SpellEffectEntry() {};
    SpellEffectEntry(SpellEffectEntry const& effect);

    //uint32        Id;                                         // 0        m_ID
    uint32        Effect;                                       // 73-75    m_effect
    float         EffectMultipleValue;                          // 106-108  m_effectAmplitude
    uint32        EffectApplyAuraName;                          // 100-102  m_effectAura
    uint32        EffectAmplitude;                              // 103-105  m_effectAuraPeriod
    int32         EffectBasePoints;                             // 82-84    m_effectBasePoints (don't must be used in spell/auras explicitly, must be used cached Spell::m_currentBasePoints)
    float         DmgMultiplier;                                // 156-158  m_effectChainAmplitude
    uint32        EffectChainTarget;                            // 109-111  m_effectChainTargets
    int32         EffectDieSides;                               // 76-78    m_effectDieSides
    uint32        EffectItemType;                               // 112-114  m_effectItemType
    uint32        EffectMechanic;                               // 85-87    m_effectMechanic
    int32         EffectMiscValue;                              // 115-117  m_effectMiscValue
    float         EffectPointsPerComboPoint;                    // 124-126  m_effectPointsPerCombo
    uint32        EffectRadiusIndex;                            // 94-96    m_effectRadiusIndex - spellradius.dbc
    float         EffectRealPointsPerLevel;                     // 79-81    m_effectRealPointsPerLevel
    uint32        EffectTriggerSpell;                           // 121-123  m_effectTriggerSpell
    uint32        EffectImplicitTargetA;                        // 88-90    m_implicitTargetA
    uint32        EffectImplicitTargetB;                        // 91-93    m_implicitTargetB
    // helpers

    int32 CalculateSimpleValue() const { return EffectBasePoints; };

    void Initialize(const SpellEntry* spellEntry, SpellEffectIndex i);

};

#define MAX_SPELL_REAGENTS 8
#define MAX_SPELL_TOTEMS 2

struct SpellEntry
{
    uint32    Id;                                           // 0 normally counted from 0 field (but some tools start counting from 1, check this before tool use for data view!)
    uint32    School;                                       // 1 not schoolMask from 2.x - just school type so everything linked with SpellEntry::SchoolMask must be rewrited
    uint32    Category;                                     // 2
    // uint32 castUI;                                       // 3 not used
    uint32    Dispel;                                       // 4
    uint32    Mechanic;                                     // 5
    uint32    Attributes;                                   // 6
    uint32    AttributesEx;                                 // 7
    uint32    AttributesEx2;                                // 8
    uint32    AttributesEx3;                                // 9
    uint32    AttributesEx4;                                // 10
    uint32    Stances;                                      // 11
    uint32    StancesNot;                                   // 12
    uint32    Targets;                                      // 13
    uint32    TargetCreatureType;                           // 14
    uint32    RequiresSpellFocus;                           // 15
    uint32    CasterAuraState;                              // 16
    uint32    TargetAuraState;                              // 17
    uint32    CastingTimeIndex;                             // 18
    uint32    RecoveryTime;                                 // 19
    uint32    CategoryRecoveryTime;                         // 20
    uint32    InterruptFlags;                               // 21
    uint32    AuraInterruptFlags;                           // 22
    uint32    ChannelInterruptFlags;                        // 23
    uint32    procFlags;                                    // 24
    uint32    procChance;                                   // 25
    uint32    procCharges;                                  // 26
    uint32    maxLevel;                                     // 27
    uint32    baseLevel;                                    // 28
    uint32    spellLevel;                                   // 29
    uint32    DurationIndex;                                // 30
    uint32    powerType;                                    // 31
    uint32    manaCost;                                     // 32
    uint32    manaCostPerlevel;                             // 33
    uint32    manaPerSecond;                                // 34
    uint32    manaPerSecondPerLevel;                        // 35
    uint32    rangeIndex;                                   // 36
    float     speed;                                        // 37
    uint32    modalNextSpell;                               // 38 not used
    uint32    StackAmount;                                  // 39
    uint32    Totem[MAX_SPELL_TOTEMS];                      // 40-41
    int32     Reagent[MAX_SPELL_REAGENTS];                  // 42-49
    uint32    ReagentCount[MAX_SPELL_REAGENTS];             // 50-57
    int32     EquippedItemClass;                            // 58 (value)
    int32     EquippedItemSubClassMask;                     // 59 (mask)
    int32     EquippedItemInventoryTypeMask;                // 60 (mask)
    uint32    Effect[MAX_EFFECT_INDEX];                     // 61-63
    int32     EffectDieSides[MAX_EFFECT_INDEX];             // 64-66
    uint32    EffectBaseDice[MAX_EFFECT_INDEX];             // 67-69
    float     EffectDicePerLevel[MAX_EFFECT_INDEX];         // 70-72
    float     EffectRealPointsPerLevel[MAX_EFFECT_INDEX];   // 73-75
    int32     EffectBasePoints[MAX_EFFECT_INDEX];           // 76-78 (don't must be used in spell/auras explicitly, must be used cached Spell::m_currentBasePoints)
    uint32    EffectMechanic[MAX_EFFECT_INDEX];             // 79-81
    uint32    EffectImplicitTargetA[MAX_EFFECT_INDEX];      // 82-84
    uint32    EffectImplicitTargetB[MAX_EFFECT_INDEX];      // 85-87
    uint32    EffectRadiusIndex[MAX_EFFECT_INDEX];          // 88-90 - spellradius.dbc
    uint32    EffectApplyAuraName[MAX_EFFECT_INDEX];        // 91-93
    uint32    EffectAmplitude[MAX_EFFECT_INDEX];            // 94-96
    float     EffectMultipleValue[MAX_EFFECT_INDEX];        // 97-99
    uint32    EffectChainTarget[MAX_EFFECT_INDEX];          // 100-102
    uint32    EffectItemType[MAX_EFFECT_INDEX];             // 103-105
    int32     EffectMiscValue[MAX_EFFECT_INDEX];            // 106-108
    uint32    EffectTriggerSpell[MAX_EFFECT_INDEX];         // 109-111
    float     EffectPointsPerComboPoint[MAX_EFFECT_INDEX];  // 112-114
    uint32    SpellVisual;                                  // 115
    //uint32    SpellVisual2                                // 116 not used
    uint32    SpellIconID;                                  // 117
    uint32    activeIconID;                                 // 118
    //uint32    spellPriority;                              // 119
    char*     SpellName[8];                                 // 120-127
    //uint32    SpellNameFlag;                              // 128
    char*     Rank[8];                                      // 129-136
    //uint32    RankFlags;                                  // 137
    //char*     Description[8];                             // 138-145 not used
    //uint32    DescriptionFlags;                           // 146     not used
    //char*     ToolTip[8];                                 // 147-154 not used
    //uint32    ToolTipFlags;                               // 155     not used
    uint32    ManaCostPercentage;                           // 156
    uint32    StartRecoveryCategory;                        // 157
    uint32    StartRecoveryTime;                            // 158
    uint32    MaxTargetLevel;                               // 159
    uint32    SpellFamilyName;                              // 160
    ClassFamilyMask SpellFamilyFlags;                       // 161+162
    uint32    MaxAffectedTargets;                           // 163
    uint32    DmgClass;                                     // 164 defenseType
    uint32    PreventionType;                               // 165
    //uint32    StanceBarOrder;                             // 166 not used
    float     DmgMultiplier[MAX_EFFECT_INDEX];              // 167-169
    //uint32    MinFactionId;                               // 170 not used, and 0 in 2.4.2
    //uint32    MinReputation;                              // 171 not used, and 0 in 2.4.2
    //uint32    RequiredAuraVision;                         // 172 not used

    // helpers
    int32 CalculateSimpleValue(SpellEffectIndex eff) const { return EffectBasePoints[eff] + int32(1); }

    bool IsFitToFamilyMask(uint64 familyFlags) const
    {
        return GetSpellFamilyFlags().IsFitToFamilyMask(familyFlags);
    }

    bool IsFitToFamily(SpellFamily family, uint64 familyFlags) const
    {
        return SpellFamily(SpellFamilyName) == family && IsFitToFamilyMask(familyFlags);
    }

    bool IsFitToFamilyMask(ClassFamilyMask const& mask) const
    {
        return GetSpellFamilyFlags().IsFitToFamilyMask(mask);
    }

    bool IsFitToFamily(SpellFamily family, ClassFamilyMask const& mask) const
    {
        return SpellFamily(SpellFamilyName) == family && IsFitToFamilyMask(mask);
    }

    // compile time version
    template <SpellFamily family, CFM_ARGS_1>
    bool IsFitToFamily() const
    {
        return SpellFamily(SpellFamilyName) == family && GetSpellFamilyFlags().test<CFM_VALUES_1>();
    }

    template <SpellFamily family, CFM_ARGS_2>
    bool IsFitToFamily() const
    {
        return SpellFamily(SpellFamilyName) == family && GetSpellFamilyFlags().test<CFM_VALUES_2>();
    }

    template <SpellFamily family, CFM_ARGS_3>
    bool IsFitToFamily() const
    {
        return SpellFamily(SpellFamilyName) == family && GetSpellFamilyFlags().test<CFM_VALUES_3>();
    }

    template <SpellFamily family, CFM_ARGS_4>
    bool IsFitToFamily() const
    {
        return SpellFamily(SpellFamilyName) == family && GetSpellFamilyFlags().test<CFM_VALUES_4>();
    }

    template <SpellFamily family, CFM_ARGS_5>
    bool IsFitToFamily() const
    {
        return SpellFamily(SpellFamilyName) == family && GetSpellFamilyFlags().test<CFM_VALUES_5>();
    }

    template <SpellFamily family, CFM_ARGS_6>
    bool IsFitToFamily() const
    {
        return SpellFamily(SpellFamilyName) == family && GetSpellFamilyFlags().test<CFM_VALUES_6>();
    }

    template <SpellFamily family, CFM_ARGS_7>
    bool IsFitToFamily() const
    {
        return SpellFamily(SpellFamilyName) == family && GetSpellFamilyFlags().test<CFM_VALUES_7>();
    }

    template <SpellFamily family, CFM_ARGS_8>
    bool IsFitToFamily() const
    {
        return SpellFamily(SpellFamilyName) == family && GetSpellFamilyFlags().test<CFM_VALUES_8>();
    }

    template <SpellFamily family, CFM_ARGS_9>
    bool IsFitToFamily() const
    {
        return SpellFamily(SpellFamilyName) == family && GetSpellFamilyFlags().test<CFM_VALUES_9>();
    }

    template <SpellFamily family, CFM_ARGS_10>
    bool IsFitToFamily() const
    {
        return SpellFamily(SpellFamilyName) == family && GetSpellFamilyFlags().test<CFM_VALUES_10>();
    }

    inline bool HasAttribute(SpellAttributes attribute) const { return Attributes & attribute; }
    inline bool HasAttribute(SpellAttributesEx attribute) const { return AttributesEx & attribute; }
    inline bool HasAttribute(SpellAttributesEx2 attribute) const { return AttributesEx2 & attribute; }
    inline bool HasAttribute(SpellAttributesEx3 attribute) const { return AttributesEx3 & attribute; }
    inline bool HasAttribute(SpellAttributesEx4 attribute) const { return AttributesEx4 & attribute; }

    inline uint32 GetMechanic() const { return Mechanic; };
    inline uint32 GetManaCost() const { return manaCost; };
    inline uint32 GetSpellFamilyName() const { return SpellFamilyName; };
    inline uint32 GetAuraInterruptFlags() const { return AuraInterruptFlags; };
    inline uint32 GetStackAmount() const { return StackAmount; };
    inline uint32 GetEffectImplicitTargetAByIndex(SpellEffectIndex j) const { return EffectImplicitTargetA[j];};
    inline uint32 GetEffectImplicitTargetBByIndex(SpellEffectIndex j) const { return EffectImplicitTargetB[j];};
    inline uint32 GetEffectApplyAuraNameByIndex(SpellEffectIndex j) const   { return EffectApplyAuraName[j];};
    inline uint32 GetEffectMiscValue(SpellEffectIndex j) const              { return EffectMiscValue[j];};
    inline ClassFamilyMask GetSpellFamilyFlags() const                      { return SpellFamilyFlags; };

    SpellEffectEntry const* GetSpellEffect(SpellEffectIndex j) const;

    private:
        // prevent creating custom entries (copy data from original in fact)
        SpellEntry(SpellEntry const&);                      // DON'T must have implementation

        // catch wrong uses
        template<typename T>
        bool IsFitToFamilyMask(SpellFamily family, T t) const;
};

// A few fields which are required for automated convertion
// NOTE that these fields are count by _skipping_ the fields that are unused!
#define LOADED_SPELLDBC_FIELD_POS_EQUIPPED_ITEM_CLASS  65   // Must be converted to -1
#define LOADED_SPELLDBC_FIELD_POS_SPELLNAME_0          132  // Links to "MaNGOS server-side spell"

struct SpellCastTimesEntry
{
    uint32    ID;                                           // 0        m_ID
    int32     CastTime;                                     // 1        m_base
    //float     CastTimePerLevel;                           // 2        m_perLevel
    //int32     MinCastTime;                                // 3        m_minimum
};

struct SpellFocusObjectEntry
{
    uint32    ID;                                           // 0        m_ID
    //char*     Name[8];                                    // 1-8      m_name_lang
                                                            // 9 string flags
};

struct SpellRadiusEntry
{
    uint32    ID;                                           //          m_ID
    float     Radius;                                       //          m_radius
                                                            //          m_radiusPerLevel
    //float     RadiusMax;                                  //          m_radiusMax
};

struct SpellRangeEntry
{
    uint32    ID;                                           // 0        m_ID
    float     minRange;                                     // 1        m_rangeMin
    float     maxRange;                                     // 2        m_rangeMax
    //uint32  Flags;                                        // 3        m_flags
    //char*  Name[8];                                       // 4-11     m_displayName_lang
    //uint32 NameFlags;                                     // 12 string flags
    //char*  ShortName[8];                                  // 13-20    m_displayNameShort_lang
    //uint32 NameFlags;                                     // 21 string flags
};

struct SpellShapeshiftFormEntry
{
    uint32 ID;                                              // 0        m_ID
    //uint32 buttonPosition;                                // 1        m_bonusActionBar
    //char*  Name[8];                                       // 2-9      m_name_lang
    //uint32 NameFlags;                                     // 10 string flags
    uint32 flags1;                                          // 11       m_flags
    int32  creatureType;                                    // 12       m_creatureType <=0 humanoid, other normal creature types
    //uint32 unk1;                                          // 13       m_attackIconID
};

struct SpellDurationEntry
{
    uint32    ID;                                           //          m_ID
    int32     Duration[3];                                  //          m_duration, m_durationPerLevel, m_maxDuration
};

struct SpellItemEnchantmentEntry
{
    uint32      ID;                                         // 0        m_ID
    uint32      type[3];                                    // 1-3      m_effect[3]
    uint32      amount[3];                                  // 4-6      m_effectPointsMin[3]
    //uint32      amount2[3]                                // 7-9      m_effectPointsMax[3]
    uint32      spellid[3];                                 // 10-12    m_effectArg[3]
    char*       description[8];                             // 13-20    m_name_lang[8]
                                                            // 21 string flags
    uint32      aura_id;                                    // 22       m_itemVisual
    uint32      slot;                                       // 23       m_flags
};

struct StableSlotPricesEntry
{
    uint32 Slot;                                            //          m_ID
    uint32 Price;                                           //          m_cost
};

#define MAX_TALENT_RANK 5

struct TalentEntry
{
    uint32    TalentID;                                     // 0        m_ID
    uint32    TalentTab;                                    // 1        m_tabID (TalentTab.dbc)
    uint32    Row;                                          // 2        m_tierID
    uint32    Col;                                          // 3        m_columnIndex
    uint32    RankID[MAX_TALENT_RANK];                      // 4-8      m_spellRank
                                                            // 9-12 part of prev field
    uint32    DependsOn;                                    // 13       m_prereqTalent (Talent.dbc)
                                                            // 14-15 part of prev field
    uint32    DependsOnRank;                                // 16       m_prereqRank
                                                            // 17-18 part of prev field
    //uint32  needAddInSpellBook;                           // 19       m_flags also need disable higest ranks on reset talent tree
    uint32    DependsOnSpell;                               // 20       m_requiredSpellID req.spell
};

struct TalentTabEntry
{
    uint32  TalentTabID;                                    // 0        m_ID
    //char* name[8];                                        // 1-8      m_name_lang
    //uint32  nameFlags;                                    // 9 string flags
    //unit32  spellicon;                                    // 10       m_spellIconID
                                                            // 11       m_raceMask
    uint32  ClassMask;                                      // 12       m_classMask
    uint32  tabpage;                                        // 13       m_orderIndex
    //char* internalname;                                   // 14       m_backgroundFile
};

struct TaxiNodesEntry
{
    uint32    ID;                                           // 0        m_ID
    uint32    map_id;                                       // 1        m_ContinentID
    float     x;                                            // 2        m_x
    float     y;                                            // 3        m_y
    float     z;                                            // 4        m_z
    char*     name[8];                                      // 5-12     m_Name_lang
                                                            // 13 string flags
    uint32    MountCreatureID[2];                           // 14-15    m_MountCreatureID[2] horde[14]-alliance[15]
};

struct TaxiPathEntry
{
    uint32    ID;
    uint32    from;
    uint32    to;
    uint32    price;
};

struct TaxiPathNodeEntry
{
                                                            // 0        m_ID
    uint32    path;                                         // 1        m_PathID
    uint32    index;                                        // 2        m_NodeIndex
    uint32    mapid;                                        // 3        m_ContinentID
    float     x;                                            // 4        m_LocX
    float     y;                                            // 5        m_LocY
    float     z;                                            // 6        m_LocZ
    uint32    actionFlag;                                   // 7        m_flags
    uint32    delay;                                        // 8        m_delay
};

struct WMOAreaTableEntry
{
    uint32 Id;                                              // 0        m_ID index
    int32 rootId;                                           // 1        m_WMOID used in root WMO
    int32 adtId;                                            // 2        m_NameSetID used in adt file
    int32 groupId;                                          // 3        m_WMOGroupID used in group WMO
    //uint32 field4;                                        // 4        m_SoundProviderPref
    //uint32 field5;                                        // 5        m_SoundProviderPrefUnderwater
    //uint32 field6;                                        // 6        m_AmbienceID
    //uint32 field7;                                        // 7        m_ZoneMusic
    //uint32 field8;                                        // 8        m_IntroSound
    uint32 Flags;                                           // 9        m_flags (used for indoor/outdoor determination)
    uint32 areaId;                                          // 10       m_AreaTableID (AreaTable.dbc)
    //char *Name[8];                                        //          m_AreaName_lang
    //uint32 nameFlags;
};

struct WorldMapAreaEntry
{
    //uint32  ID;                                           // 0        m_ID
    uint32  map_id;                                         // 1        m_mapID
    uint32  area_id;                                        // 2        m_areaID index (continent 0 areas ignored)
    //char* internal_name                                   // 3        m_areaName
    float   y1;                                             // 4        m_locLeft
    float   y2;                                             // 5        m_locRight
    float   x1;                                             // 6        m_locTop
    float   x2;                                             // 7        m_locBottom
};

#define MAX_WORLD_MAP_OVERLAY_AREA_IDX 4

struct WorldMapOverlayEntry
{
    uint32    ID;                                           // 0        m_ID
    //uint32    worldMapAreaId;                             // 1        m_mapAreaID (WorldMapArea.dbc)
    uint32    areatableID[MAX_WORLD_MAP_OVERLAY_AREA_IDX];  // 2-5      m_areaID
                                                            // 6        m_mapPointX
                                                            // 7        m_mapPointY
    //char* internal_name                                   // 8        m_textureName
                                                            // 9        m_textureWidth
                                                            // 10       m_textureHeight
                                                            // 11       m_offsetX
                                                            // 12       m_offsetY
                                                            // 13       m_hitRectTop
                                                            // 14       m_hitRectLeft
                                                            // 15       m_hitRectBottom
                                                            // 16       m_hitRectRight
};

struct WorldSafeLocsEntry
{
    uint32    ID;                                           // 0        m_ID
    uint32    map_id;                                       // 1        m_continent
    float     x;                                            // 2        m_locX
    float     y;                                            // 3        m_locY
    float     z;                                            // 4        m_locZ
    //char*   name[8]                                       // 5-12     m_AreaName_lang
                                                            // 13 string flags
};


struct WorldStateEntry
{
    uint32    ID;                                           // 0        m_ID
    uint32    map_id;                                       // 1        WorldState bind map
    uint32    m_zone;                                       // 2        WorldState bind zone (0 - on battlegrounds)
    uint32    m_flags;                                      // 3
//    char*     m_uiIcon;                                   // 4
    char*     m_uiMessage1[8];                              // 5-12
//    uint32    m_flags1;                                   // 13       string flags
//    char*     m_uiMessage2[8]                             // 14-21
//    uint32    m_flags2;                                   // 22       string flags
    uint32    m_state;                                      // 23       WorldState ID (not unique!) 0 - for battleground states.
//    char*     m_uiIcon1;                                  // 24
//    char*     m_uiIcon2;                                  // 25
//    char*     m_uiMessage3[8]                             // 26-33
//    uint32    m_flags3;                                   // 34       string flags
    char*     m_uiType;                                     // 35       only CAPTUREPOINT type, or NULL
    uint32    m_linked1;                                    // 36
    uint32    m_linked2;                                    // 37
//    uint32    m_unk62;                                    // 38       only 0
};

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

typedef std::set<uint32> SpellCategorySet;
typedef std::map<uint32,SpellCategorySet > SpellCategoryStore;
typedef std::set<uint32> PetFamilySpellsSet;
typedef std::map<uint32,PetFamilySpellsSet > PetFamilySpellsStore;

struct TalentSpellPos
{
    TalentSpellPos() : talent_id(0), rank(0) {}
    TalentSpellPos(uint16 _talent_id, uint8 _rank) : talent_id(_talent_id), rank(_rank) {}

    uint16 talent_id;
    uint8  rank;
};

typedef std::map<uint32,TalentSpellPos> TalentSpellPosMap;

struct SpellEffect
{
    SpellEffectEntry effects[MAX_EFFECT_INDEX];

    SpellEffect()
    {
    }

    ~SpellEffect()
    {
    }
};

typedef std::map<uint32, SpellEffect> SpellEffectMap;

struct TaxiPathBySourceAndDestination
{
    TaxiPathBySourceAndDestination() : ID(0),price(0) {}
    TaxiPathBySourceAndDestination(uint32 _id,uint32 _price) : ID(_id),price(_price) {}

    uint32    ID;
    uint32    price;
};
typedef std::map<uint32,TaxiPathBySourceAndDestination> TaxiPathSetForSource;
typedef std::map<uint32,TaxiPathSetForSource> TaxiPathSetBySource;

struct TaxiPathNodePtr
{
    TaxiPathNodePtr() : i_ptr(NULL) {}
    TaxiPathNodePtr(TaxiPathNodeEntry const* ptr) : i_ptr(ptr) {}

    TaxiPathNodeEntry const* i_ptr;

    operator TaxiPathNodeEntry const& () const { return *i_ptr; }
};

typedef Path<TaxiPathNodePtr,TaxiPathNodeEntry const> TaxiPathNodeList;
typedef std::vector<TaxiPathNodeList> TaxiPathNodesByPath;

#define TaxiMaskSize 8
typedef uint32 TaxiMask[TaxiMaskSize];
#endif
