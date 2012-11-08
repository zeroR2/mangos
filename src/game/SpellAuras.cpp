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
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "UpdateMask.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Unit.h"
#include "Spell.h"
#include "DynamicObject.h"
#include "Group.h"
#include "UpdateData.h"
#include "ObjectAccessor.h"
#include "Policies/SingletonImp.h"
#include "Totem.h"
#include "Creature.h"
#include "Formulas.h"
#include "BattleGround/BattleGround.h"
#include "OutdoorPvP/OutdoorPvP.h"
#include "CreatureAI.h"
#include "ScriptMgr.h"
#include "Util.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "InstanceData.h"
#include "Language.h"
#include "MapManager.h"

#define NULL_AURA_SLOT 0xFF

pAuraHandler AuraHandler[TOTAL_AURAS]=
{
    &Aura::HandleNULL,                                      //  0 SPELL_AURA_NONE
    &Aura::HandleBindSight,                                 //  1 SPELL_AURA_BIND_SIGHT
    &Aura::HandleModPossess,                                //  2 SPELL_AURA_MOD_POSSESS
    &Aura::HandlePeriodicDamage,                            //  3 SPELL_AURA_PERIODIC_DAMAGE
    &Aura::HandleAuraDummy,                                 //  4 SPELL_AURA_DUMMY
    &Aura::HandleModConfuse,                                //  5 SPELL_AURA_MOD_CONFUSE
    &Aura::HandleModCharm,                                  //  6 SPELL_AURA_MOD_CHARM
    &Aura::HandleModFear,                                   //  7 SPELL_AURA_MOD_FEAR
    &Aura::HandlePeriodicHeal,                              //  8 SPELL_AURA_PERIODIC_HEAL
    &Aura::HandleModAttackSpeed,                            //  9 SPELL_AURA_MOD_ATTACKSPEED
    &Aura::HandleModThreat,                                 // 10 SPELL_AURA_MOD_THREAT
    &Aura::HandleModTaunt,                                  // 11 SPELL_AURA_MOD_TAUNT
    &Aura::HandleAuraModStun,                               // 12 SPELL_AURA_MOD_STUN
    &Aura::HandleModDamageDone,                             // 13 SPELL_AURA_MOD_DAMAGE_DONE
    &Aura::HandleNoImmediateEffect,                         // 14 SPELL_AURA_MOD_DAMAGE_TAKEN   implemented in Unit::MeleeDamageBonusTaken and Unit::SpellBaseDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         // 15 SPELL_AURA_DAMAGE_SHIELD      implemented in Unit::DealMeleeDamage
    &Aura::HandleModStealth,                                // 16 SPELL_AURA_MOD_STEALTH
    &Aura::HandleNoImmediateEffect,                         // 17 SPELL_AURA_MOD_STEALTH_DETECT
    &Aura::HandleInvisibility,                              // 18 SPELL_AURA_MOD_INVISIBILITY
    &Aura::HandleInvisibilityDetect,                        // 19 SPELL_AURA_MOD_INVISIBILITY_DETECTION
    &Aura::HandleAuraModTotalHealthPercentRegen,            // 20 SPELL_AURA_OBS_MOD_HEALTH
    &Aura::HandleAuraModTotalManaPercentRegen,              // 21 SPELL_AURA_OBS_MOD_MANA
    &Aura::HandleAuraModResistance,                         // 22 SPELL_AURA_MOD_RESISTANCE
    &Aura::HandlePeriodicTriggerSpell,                      // 23 SPELL_AURA_PERIODIC_TRIGGER_SPELL
    &Aura::HandlePeriodicEnergize,                          // 24 SPELL_AURA_PERIODIC_ENERGIZE
    &Aura::HandleAuraModPacify,                             // 25 SPELL_AURA_MOD_PACIFY
    &Aura::HandleAuraModRoot,                               // 26 SPELL_AURA_MOD_ROOT
    &Aura::HandleAuraModSilence,                            // 27 SPELL_AURA_MOD_SILENCE
    &Aura::HandleNoImmediateEffect,                         // 28 SPELL_AURA_REFLECT_SPELLS        implement in Unit::SpellHitResult
    &Aura::HandleAuraModStat,                               // 29 SPELL_AURA_MOD_STAT
    &Aura::HandleAuraModSkill,                              // 30 SPELL_AURA_MOD_SKILL
    &Aura::HandleAuraModIncreaseSpeed,                      // 31 SPELL_AURA_MOD_INCREASE_SPEED
    &Aura::HandleAuraModIncreaseMountedSpeed,               // 32 SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED
    &Aura::HandleAuraModDecreaseSpeed,                      // 33 SPELL_AURA_MOD_DECREASE_SPEED
    &Aura::HandleAuraModIncreaseHealth,                     // 34 SPELL_AURA_MOD_INCREASE_HEALTH
    &Aura::HandleAuraModIncreaseEnergy,                     // 35 SPELL_AURA_MOD_INCREASE_ENERGY
    &Aura::HandleAuraModShapeshift,                         // 36 SPELL_AURA_MOD_SHAPESHIFT
    &Aura::HandleAuraModEffectImmunity,                     // 37 SPELL_AURA_EFFECT_IMMUNITY
    &Aura::HandleAuraModStateImmunity,                      // 38 SPELL_AURA_STATE_IMMUNITY
    &Aura::HandleAuraModSchoolImmunity,                     // 39 SPELL_AURA_SCHOOL_IMMUNITY
    &Aura::HandleAuraModDmgImmunity,                        // 40 SPELL_AURA_DAMAGE_IMMUNITY
    &Aura::HandleAuraModDispelImmunity,                     // 41 SPELL_AURA_DISPEL_IMMUNITY
    &Aura::HandleAuraProcTriggerSpell,                      // 42 SPELL_AURA_PROC_TRIGGER_SPELL  implemented in Unit::ProcDamageAndSpellFor and Unit::HandleProcTriggerSpell
    &Aura::HandleNoImmediateEffect,                         // 43 SPELL_AURA_PROC_TRIGGER_DAMAGE implemented in Unit::ProcDamageAndSpellFor
    &Aura::HandleAuraTrackCreatures,                        // 44 SPELL_AURA_TRACK_CREATURES
    &Aura::HandleAuraTrackResources,                        // 45 SPELL_AURA_TRACK_RESOURCES
    &Aura::HandleUnused,                                    // 46 SPELL_AURA_46
    &Aura::HandleAuraModParryPercent,                       // 47 SPELL_AURA_MOD_PARRY_PERCENT
    &Aura::HandleUnused,                                    // 48 SPELL_AURA_48
    &Aura::HandleAuraModDodgePercent,                       // 49 SPELL_AURA_MOD_DODGE_PERCENT
    &Aura::HandleUnused,                                    // 50 SPELL_AURA_MOD_BLOCK_SKILL    obsolete?
    &Aura::HandleAuraModBlockPercent,                       // 51 SPELL_AURA_MOD_BLOCK_PERCENT
    &Aura::HandleAuraModCritPercent,                        // 52 SPELL_AURA_MOD_CRIT_PERCENT
    &Aura::HandlePeriodicLeech,                             // 53 SPELL_AURA_PERIODIC_LEECH
    &Aura::HandleModHitChance,                              // 54 SPELL_AURA_MOD_HIT_CHANCE
    &Aura::HandleModSpellHitChance,                         // 55 SPELL_AURA_MOD_SPELL_HIT_CHANCE
    &Aura::HandleAuraTransform,                             // 56 SPELL_AURA_TRANSFORM
    &Aura::HandleModSpellCritChance,                        // 57 SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    &Aura::HandleAuraModIncreaseSwimSpeed,                  // 58 SPELL_AURA_MOD_INCREASE_SWIM_SPEED
    &Aura::HandleNoImmediateEffect,                         // 59 SPELL_AURA_MOD_DAMAGE_DONE_CREATURE implemented in Unit::MeleeDamageBonusDone and Unit::SpellDamageBonusDone
    &Aura::HandleAuraModPacifyAndSilence,                   // 60 SPELL_AURA_MOD_PACIFY_SILENCE
    &Aura::HandleAuraModScale,                              // 61 SPELL_AURA_MOD_SCALE
    &Aura::HandlePeriodicHealthFunnel,                      // 62 SPELL_AURA_PERIODIC_HEALTH_FUNNEL
    &Aura::HandleUnused,                                    // 63 SPELL_AURA_PERIODIC_MANA_FUNNEL obsolete?
    &Aura::HandlePeriodicManaLeech,                         // 64 SPELL_AURA_PERIODIC_MANA_LEECH
    &Aura::HandleModCastingSpeed,                           // 65 SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK
    &Aura::HandleFeignDeath,                                // 66 SPELL_AURA_FEIGN_DEATH
    &Aura::HandleAuraModDisarm,                             // 67 SPELL_AURA_MOD_DISARM
    &Aura::HandleAuraModStalked,                            // 68 SPELL_AURA_MOD_STALKED
    &Aura::HandleSchoolAbsorb,                              // 69 SPELL_AURA_SCHOOL_ABSORB implemented in Unit::CalculateDamageAbsorbAndResist
    &Aura::HandleUnused,                                    // 70 SPELL_AURA_EXTRA_ATTACKS      Useless, used by only one spell that has only visual effect
    &Aura::HandleModSpellCritChanceShool,                   // 71 SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    &Aura::HandleModPowerCostPCT,                           // 72 SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT
    &Aura::HandleModPowerCost,                              // 73 SPELL_AURA_MOD_POWER_COST_SCHOOL
    &Aura::HandleNoImmediateEffect,                         // 74 SPELL_AURA_REFLECT_SPELLS_SCHOOL  implemented in Unit::SpellHitResult
    &Aura::HandleNoImmediateEffect,                         // 75 SPELL_AURA_MOD_LANGUAGE
    &Aura::HandleFarSight,                                  // 76 SPELL_AURA_FAR_SIGHT
    &Aura::HandleModMechanicImmunity,                       // 77 SPELL_AURA_MECHANIC_IMMUNITY
    &Aura::HandleAuraMounted,                               // 78 SPELL_AURA_MOUNTED
    &Aura::HandleModDamagePercentDone,                      // 79 SPELL_AURA_MOD_DAMAGE_PERCENT_DONE
    &Aura::HandleModPercentStat,                            // 80 SPELL_AURA_MOD_PERCENT_STAT
    &Aura::HandleNoImmediateEffect,                         // 81 SPELL_AURA_SPLIT_DAMAGE_PCT       implemented in Unit::CalculateDamageAbsorbAndResist
    &Aura::HandleWaterBreathing,                            // 82 SPELL_AURA_WATER_BREATHING
    &Aura::HandleModBaseResistance,                         // 83 SPELL_AURA_MOD_BASE_RESISTANCE
    &Aura::HandleModRegen,                                  // 84 SPELL_AURA_MOD_REGEN
    &Aura::HandleModPowerRegen,                             // 85 SPELL_AURA_MOD_POWER_REGEN
    &Aura::HandleChannelDeathItem,                          // 86 SPELL_AURA_CHANNEL_DEATH_ITEM
    &Aura::HandleNoImmediateEffect,                         // 87 SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN implemented in Unit::MeleeDamageBonusTaken and Unit::SpellDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         // 88 SPELL_AURA_MOD_HEALTH_REGEN_PERCENT implemented in Player::RegenerateHealth
    &Aura::HandlePeriodicDamagePCT,                         // 89 SPELL_AURA_PERIODIC_DAMAGE_PERCENT
    &Aura::HandleUnused,                                    // 90 SPELL_AURA_MOD_RESIST_CHANCE  Useless
    &Aura::HandleNoImmediateEffect,                         // 91 SPELL_AURA_MOD_DETECT_RANGE implemented in Creature::GetAttackDistance
    &Aura::HandlePreventFleeing,                            // 92 SPELL_AURA_PREVENTS_FLEEING
    &Aura::HandleModUnattackable,                           // 93 SPELL_AURA_MOD_UNATTACKABLE
    &Aura::HandleInterruptRegen,                            // 94 SPELL_AURA_INTERRUPT_REGEN implemented in Player::RegenerateAll
    &Aura::HandleAuraGhost,                                 // 95 SPELL_AURA_GHOST
    &Aura::HandleNoImmediateEffect,                         // 96 SPELL_AURA_SPELL_MAGNET implemented in Unit::SelectMagnetTarget
    &Aura::HandleManaShield,                                // 97 SPELL_AURA_MANA_SHIELD implemented in Unit::CalculateDamageAbsorbAndResist
    &Aura::HandleAuraModSkill,                              // 98 SPELL_AURA_MOD_SKILL_TALENT
    &Aura::HandleAuraModAttackPower,                        // 99 SPELL_AURA_MOD_ATTACK_POWER
    &Aura::HandleAurasVisible,                              //100 SPELL_AURA_AURAS_VISIBLE
    &Aura::HandleModResistancePercent,                      //101 SPELL_AURA_MOD_RESISTANCE_PCT
    &Aura::HandleNoImmediateEffect,                         //102 SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonusDone
    &Aura::HandleAuraModTotalThreat,                        //103 SPELL_AURA_MOD_TOTAL_THREAT
    &Aura::HandleAuraWaterWalk,                             //104 SPELL_AURA_WATER_WALK
    &Aura::HandleAuraFeatherFall,                           //105 SPELL_AURA_FEATHER_FALL
    &Aura::HandleAuraHover,                                 //106 SPELL_AURA_HOVER
    &Aura::HandleAddModifier,                               //107 SPELL_AURA_ADD_FLAT_MODIFIER
    &Aura::HandleAddModifier,                               //108 SPELL_AURA_ADD_PCT_MODIFIER
    &Aura::HandleAddTargetTrigger,                          //109 SPELL_AURA_ADD_TARGET_TRIGGER
    &Aura::HandleModPowerRegenPCT,                          //110 SPELL_AURA_MOD_POWER_REGEN_PERCENT
    &Aura::HandleUnused,                                    //111 SPELL_AURA_ADD_CASTER_HIT_TRIGGER
    &Aura::HandleNoImmediateEffect,                         //112 SPELL_AURA_OVERRIDE_CLASS_SCRIPTS implemented in diff functions.
    &Aura::HandleNoImmediateEffect,                         //113 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         //114 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         //115 SPELL_AURA_MOD_HEALING                 implemented in Unit::SpellBaseHealingBonusTaken
    &Aura::HandleNoImmediateEffect,                         //116 SPELL_AURA_MOD_REGEN_DURING_COMBAT     imppemented in Player::RegenerateAll and Player::RegenerateHealth
    &Aura::HandleNoImmediateEffect,                         //117 SPELL_AURA_MOD_MECHANIC_RESISTANCE     implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         //118 SPELL_AURA_MOD_HEALING_PCT             implemented in Unit::SpellHealingBonusTaken
    &Aura::HandleUnused,                                    //119 SPELL_AURA_SHARE_PET_TRACKING useless
    &Aura::HandleAuraUntrackable,                           //120 SPELL_AURA_UNTRACKABLE
    &Aura::HandleAuraEmpathy,                               //121 SPELL_AURA_EMPATHY
    &Aura::HandleModOffhandDamagePercent,                   //122 SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT
    &Aura::HandleNoImmediateEffect,                         //123 SPELL_AURA_MOD_TARGET_RESISTANCE  implemented in Unit::CalculateDamageAbsorbAndResist and Unit::CalcArmorReducedDamage
    &Aura::HandleAuraModRangedAttackPower,                  //124 SPELL_AURA_MOD_RANGED_ATTACK_POWER
    &Aura::HandleNoImmediateEffect,                         //125 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         //126 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonusTaken
    &Aura::HandleNoImmediateEffect,                         //127 SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonusDone
    &Aura::HandleModPossessPet,                             //128 SPELL_AURA_MOD_POSSESS_PET
    &Aura::HandleAuraModIncreaseSpeed,                      //129 SPELL_AURA_MOD_SPEED_ALWAYS
    &Aura::HandleAuraModIncreaseMountedSpeed,               //130 SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS
    &Aura::HandleNoImmediateEffect,                         //131 SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonusDone
    &Aura::HandleAuraModIncreaseEnergyPercent,              //132 SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT
    &Aura::HandleAuraModIncreaseHealthPercent,              //133 SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT
    &Aura::HandleAuraModRegenInterrupt,                     //134 SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
    &Aura::HandleModHealingDone,                            //135 SPELL_AURA_MOD_HEALING_DONE
    &Aura::HandleNoImmediateEffect,                         //136 SPELL_AURA_MOD_HEALING_DONE_PERCENT   implemented in Unit::SpellHealingBonusDone
    &Aura::HandleModTotalPercentStat,                       //137 SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE
    &Aura::HandleModMeleeSpeedPct,                          //138 SPELL_AURA_MOD_MELEE_HASTE
    &Aura::HandleForceReaction,                             //139 SPELL_AURA_FORCE_REACTION
    &Aura::HandleAuraModRangedHaste,                        //140 SPELL_AURA_MOD_RANGED_HASTE
    &Aura::HandleRangedAmmoHaste,                           //141 SPELL_AURA_MOD_RANGED_AMMO_HASTE
    &Aura::HandleAuraModBaseResistancePCT,                  //142 SPELL_AURA_MOD_BASE_RESISTANCE_PCT
    &Aura::HandleAuraModResistanceExclusive,                //143 SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE
    &Aura::HandleAuraSafeFall,                              //144 SPELL_AURA_SAFE_FALL                         implemented in WorldSession::HandleMovementOpcodes
    &Aura::HandleUnused,                                    //145 SPELL_AURA_CHARISMA obsolete?
    &Aura::HandleUnused,                                    //146 SPELL_AURA_PERSUADED obsolete?
    &Aura::HandleModMechanicImmunityMask,                   //147 SPELL_AURA_MECHANIC_IMMUNITY_MASK            implemented in Unit::IsImmuneToSpell and Unit::IsImmuneToSpellEffect (check part)
    &Aura::HandleAuraRetainComboPoints,                     //148 SPELL_AURA_RETAIN_COMBO_POINTS
    &Aura::HandleNoImmediateEffect,                         //149 SPELL_AURA_RESIST_PUSHBACK
    &Aura::HandleShieldBlockValue,                          //150 SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT
    &Aura::HandleAuraTrackStealthed,                        //151 SPELL_AURA_TRACK_STEALTHED
    &Aura::HandleNoImmediateEffect,                         //152 SPELL_AURA_MOD_DETECTED_RANGE         implemented in Creature::GetAttackDistance
    &Aura::HandleNoImmediateEffect,                         //153 SPELL_AURA_SPLIT_DAMAGE_FLAT          implemented in Unit::CalculateDamageAbsorbAndResist
    &Aura::HandleNoImmediateEffect,                         //154 SPELL_AURA_MOD_STEALTH_LEVEL          implemented in Unit::isVisibleForOrDetect
    &Aura::HandleNoImmediateEffect,                         //155 SPELL_AURA_MOD_WATER_BREATHING        implemented in Player::getMaxTimer
    &Aura::HandleNoImmediateEffect,                         //156 SPELL_AURA_MOD_REPUTATION_GAIN        implemented in Player::CalculateReputationGain
    &Aura::HandleUnused,                                    //157 SPELL_AURA_PET_DAMAGE_MULTI (single test like spell 20782, also single for 214 aura)
    &Aura::HandleShieldBlockValue,                          //158 SPELL_AURA_MOD_SHIELD_BLOCKVALUE
    &Aura::HandleNoImmediateEffect,                         //159 SPELL_AURA_NO_PVP_CREDIT      only for Honorless Target spell
    &Aura::HandleNoImmediateEffect,                         //160 SPELL_AURA_MOD_AOE_AVOIDANCE                 implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         //161 SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT
    &Aura::HandleAuraPowerBurn,                             //162 SPELL_AURA_POWER_BURN_MANA
    &Aura::HandleUnused,                                    //163 SPELL_AURA_MOD_CRIT_DAMAGE_BONUS
    &Aura::HandleUnused,                                    //164 useless, only one test spell
    &Aura::HandleNoImmediateEffect,                         //165 SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonusDone
    &Aura::HandleAuraModAttackPowerPercent,                 //166 SPELL_AURA_MOD_ATTACK_POWER_PCT
    &Aura::HandleAuraModRangedAttackPowerPercent,           //167 SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT
    &Aura::HandleNoImmediateEffect,                         //168 SPELL_AURA_MOD_DAMAGE_DONE_VERSUS            implemented in Unit::SpellDamageBonusDone, Unit::MeleeDamageBonusDone
    &Aura::HandleNoImmediateEffect,                         //169 SPELL_AURA_MOD_CRIT_PERCENT_VERSUS           implemented in Unit::DealDamageBySchool, Unit::DoAttackDamage, Unit::SpellCriticalBonus
    &Aura::HandleNULL,                                      //170 SPELL_AURA_DETECT_AMORE       only for Detect Amore spell
    &Aura::HandleAuraModIncreaseSpeed,                      //171 SPELL_AURA_MOD_SPEED_NOT_STACK
    &Aura::HandleAuraModIncreaseMountedSpeed,               //172 SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK
    &Aura::HandleUnused,                                    //173 SPELL_AURA_ALLOW_CHAMPION_SPELLS  only for Proclaim Champion spell
    &Aura::HandleModSpellDamagePercentFromStat,             //174 SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT  implemented in Unit::SpellBaseDamageBonusDone (in 1.12.* only spirit)
    &Aura::HandleModSpellHealingPercentFromStat,            //175 SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT implemented in Unit::SpellBaseHealingBonusDone (in 1.12.* only spirit)
    &Aura::HandleSpiritOfRedemption,                        //176 SPELL_AURA_SPIRIT_OF_REDEMPTION   only for Spirit of Redemption spell, die at aura end
    &Aura::HandleNULL,                                      //177 SPELL_AURA_AOE_CHARM
    &Aura::HandleNoImmediateEffect,                         //178 SPELL_AURA_MOD_DEBUFF_RESISTANCE          implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         //179 SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE implemented in Unit::SpellCriticalBonus
    &Aura::HandleNoImmediateEffect,                         //180 SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS   implemented in Unit::SpellDamageBonusDone
    &Aura::HandleUnused,                                    //181 SPELL_AURA_MOD_FLAT_SPELL_CRIT_DAMAGE_VERSUS unused
    &Aura::HandleAuraModResistenceOfIntellectPercent,       //182 SPELL_AURA_MOD_RESISTANCE_OF_INTELLECT_PERCENT
    &Aura::HandleNoImmediateEffect,                         //183 SPELL_AURA_MOD_CRITICAL_THREAT only used in 28746, implemented in ThreatCalcHelper::CalcThreat
    &Aura::HandleNoImmediateEffect,                         //184 SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE  implemented in Unit::RollMeleeOutcomeAgainst
    &Aura::HandleNoImmediateEffect,                         //185 SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE implemented in Unit::RollMeleeOutcomeAgainst
    &Aura::HandleNoImmediateEffect,                         //186 SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE  implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         //187 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE  implemented in Unit::GetUnitCriticalChance
    &Aura::HandleNoImmediateEffect,                         //188 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE implemented in Unit::GetUnitCriticalChance
    &Aura::HandleUnused,                                    //189 SPELL_AURA_MOD_RATING (not used in 1.12.1)
    &Aura::HandleNoImmediateEffect,                         //190 SPELL_AURA_MOD_FACTION_REPUTATION_GAIN     implemented in Player::CalculateReputationGain
    &Aura::HandleAuraModUseNormalSpeed,                     //191 SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED
};

static AuraType const frozenAuraTypes[] = { SPELL_AURA_MOD_ROOT, SPELL_AURA_MOD_STUN, SPELL_AURA_NONE };

Aura::Aura(AuraClassType type, SpellEntry const* spellproto, SpellEffectIndex eff, int32 *currentBasePoints, SpellAuraHolderPtr holder, Unit *target, Unit *caster, Item* castItem) :
m_periodicTimer(0), m_periodicTick(0), m_removeMode(AURA_REMOVE_BY_DEFAULT),
m_effIndex(eff), m_deleted(false), m_positive(false), m_isPeriodic(false), m_isAreaAura(false),
m_isPersistent(false), m_spellAuraHolder(holder), m_classType(type)
{
    MANGOS_ASSERT(target);
    MANGOS_ASSERT(spellproto && spellproto == sSpellStore.LookupEntry( spellproto->Id ) && "`info` must be pointer to sSpellStore element");

    m_currentBasePoints = currentBasePoints ? *currentBasePoints : spellproto->CalculateSimpleValue(eff);

    m_positive = IsPositiveEffect(spellproto, m_effIndex);
    m_applyTime = time(NULL);

    int32 damage;
    if (!caster)
        damage = m_currentBasePoints;
    else
        damage = caster->CalculateSpellDamage(target, spellproto, m_effIndex, &m_currentBasePoints);

    damage *= holder->GetStackAmount();

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Aura: construct Spellid : %u, Aura : %u Target : %d Damage : %d", spellproto->Id, spellproto->EffectApplyAuraName[eff], spellproto->EffectImplicitTargetA[eff], damage);

    SetModifier(AuraType(spellproto->EffectApplyAuraName[eff]), damage, spellproto->EffectAmplitude[eff], spellproto->EffectMiscValue[eff]);

    Player* modOwner = caster ? caster->GetSpellModOwner() : NULL;

    // Apply periodic time mod
    if (modOwner && m_modifier.periodictime)
    {
        modOwner->ApplySpellMod(spellproto->Id, SPELLMOD_ACTIVATION_TIME, m_modifier.periodictime);
        uint32 newperiodictime  = modOwner->CalculateAuraPeriodicTimeWithHaste(spellproto, m_modifier.periodictime);
        if (newperiodictime != m_modifier.periodictime)
            m_modifier.periodictime = newperiodictime;
    }

    // <sid> with SPELL_ATTR_EX5_START_PERIODIC_AT_APPLY only 1 spell
    // may be some totems
    switch (spellproto->Id)
    {
        case 18400: 
            m_periodicTimer = m_modifier.periodictime;
            break;
        default:
            break;
    }

    // Calculate CrowdControl damage start value
    if (IsCrowdControlAura(m_modifier.m_auraname))
    {
        if (uint32 cc_base_damage = CalculateCrowdControlBreakDamage())
            m_modifier.m_baseamount = cc_base_damage;
    }

    m_stacking = IsEffectStacking();

    switch (type)
    {
        case AURA_CLASS_AREA_AURA:
        {
            AreaAura(spellproto, eff, currentBasePoints, holder, target, caster, castItem);
            break;
        }
        case AURA_CLASS_PERSISTENT_AREA_AURA:
        {
            PersistentAreaAura(spellproto, eff, currentBasePoints, holder, target, caster, castItem);
            break;
        }
        case AURA_CLASS_SINGLE_ENEMY_AURA:
        {
            SingleEnemyTargetAura(spellproto, eff, currentBasePoints, holder, target, caster, castItem);
            break;
        }
        case AURA_CLASS_AURA:
        default:
            break;
    }
}

Aura::~Aura()
{
}

ObjectGuid const& Aura::GetCastItemGuid() const { return GetHolder() ? GetHolder()->GetCastItemGuid() : ObjectGuid::Null; }

ObjectGuid const& Aura::GetCasterGuid() const { return GetHolder() ? GetHolder()->GetCasterGuid() : ObjectGuid::Null; }

ObjectGuid const& Aura::GetAffectiveCasterGuid() const { return GetHolder() ? GetHolder()->GetAffectiveCasterGuid() : ObjectGuid::Null; }

void Aura::AreaAura(SpellEntry const* spellproto, SpellEffectIndex eff, int32 *currentBasePoints, SpellAuraHolderPtr holder, Unit *target,Unit *caster, Item* castItem)
{
    m_isAreaAura = true;

    // caster==NULL in constructor args if target==caster in fact
    Unit* caster_ptr = caster ? caster : target;

    m_radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(spellproto->EffectRadiusIndex[m_effIndex]));
    if (Player* modOwner = caster_ptr->GetSpellModOwner())
        modOwner->ApplySpellMod(spellproto->Id, SPELLMOD_RADIUS, m_radius);

    switch(spellproto->Effect[eff])
    {
        case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
            m_areaAuraType = AREA_AURA_PARTY;
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_PET:
            m_areaAuraType = AREA_AURA_PET;
            break;
        default:
            sLog.outError("Wrong spell effect in AreaAura constructor");
            MANGOS_ASSERT(false);
            break;
    }

    // totems are immune to any kind of area auras
    if (target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->IsTotem())
        m_modifier.m_auraname = SPELL_AURA_NONE;
}

void Aura::PersistentAreaAura(SpellEntry const* spellproto, SpellEffectIndex eff, int32 *currentBasePoints, SpellAuraHolderPtr holder, Unit *target,Unit *caster, Item* castItem)
{
    m_isPersistent = true;
}

void Aura::SingleEnemyTargetAura(SpellEntry const* spellproto, SpellEffectIndex eff, int32 *currentBasePoints, SpellAuraHolderPtr holder, Unit *target, Unit *caster, Item* castItem)
{
    if (caster)
        m_castersTargetGuid = caster->GetTypeId()==TYPEID_PLAYER ? ((Player*)caster)->GetSelectionGuid() : caster->GetTargetGuid();
}

Unit* Aura::GetTriggerTarget() const
{
    if (GetAuraClassType() != AURA_CLASS_SINGLE_ENEMY_AURA)
        return m_spellAuraHolder->GetTarget();

    // search for linked dummy aura with the correct target
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (i != GetEffIndex())
            if (Aura *aur = GetHolder()->GetAuraByEffectIndex(SpellEffectIndex(i)))
                if (aur->GetSpellProto()->EffectApplyAuraName[i] == SPELL_AURA_DUMMY)
                    return aur->GetTarget();

    return ObjectAccessor::GetUnit(*(m_spellAuraHolder->GetTarget()), m_castersTargetGuid);
}

Aura* SpellAuraHolder::CreateAura(SpellEntry const* spellproto, SpellEffectIndex eff, int32* currentBasePoints, SpellAuraHolderPtr holder, Unit *target, Unit *caster, Item* castItem)
{
    if (!spellproto || spellproto != m_spellProto)
        return (Aura*)NULL;

    uint32 triggeredSpellId = spellproto->EffectTriggerSpell[eff];
    if (IsAreaAuraEffect(spellproto->Effect[eff]))
    {
        return CreateAura(AURA_CLASS_AREA_AURA, eff, currentBasePoints, holder, target, caster, castItem);
    }
    else if (SpellEntry const* triggeredSpellInfo = sSpellStore.LookupEntry(triggeredSpellId))
    {
        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (triggeredSpellInfo->EffectImplicitTargetA[i] == TARGET_SINGLE_ENEMY)
            {
                return CreateAura(AURA_CLASS_SINGLE_ENEMY_AURA, eff, currentBasePoints, holder, target, caster, castItem);
            }
        }
    }
    // else - normal aura

    return CreateAura(AURA_CLASS_AURA, eff, currentBasePoints, holder, target, caster, castItem);
}

Aura* SpellAuraHolder::CreateAura(AuraClassType type, SpellEffectIndex eff, int32* currentBasePoints, SpellAuraHolderPtr holder, Unit* target, Unit* caster, Item* castItem)
{
    AddAura(Aura(type, m_spellProto, eff, currentBasePoints, holder, target, caster, castItem),eff);

    return GetAuraByEffectIndex(eff);
}

SpellAuraHolderPtr CreateSpellAuraHolder(SpellEntry const* spellproto, Unit *target, WorldObject *caster, Item *castItem)
{
     SpellAuraHolderPtr holderPtr = SpellAuraHolderPtr(new SpellAuraHolder(spellproto, target, caster, castItem));
     return holderPtr;
}

void Aura::SetModifier(AuraType t, int32 a, uint32 pt, int32 miscValue)
{
    m_modifier.m_auraname = t;
    m_modifier.m_amount = a;
    m_modifier.m_miscvalue = miscValue;
    m_modifier.periodictime = pt;
    m_modifier.m_baseamount = a;
}

void Aura::UpdateAura(uint32 diff)
{
    switch(GetAuraClassType())
    {
        case AURA_CLASS_AREA_AURA:
            AreaAuraUpdate(diff);
            break;
        case AURA_CLASS_PERSISTENT_AREA_AURA:
        {
            PersistentAreaAuraUpdate(diff);
            break;
        }
        case AURA_CLASS_SINGLE_ENEMY_AURA:
        case AURA_CLASS_AURA:
        default:
            Update(diff);
            break;
    }
}

void Aura::Update(uint32 diff)
{
    if (m_isPeriodic)
    {
        m_periodicTimer -= diff;
        if (m_periodicTimer <= 0) // tick also at m_periodicTimer==0 to prevent lost last tick in case max m_duration == (max m_periodicTimer)*N
        {
            // update before applying (aura can be removed in TriggerSpell or PeriodicTick calls)
            m_periodicTimer += m_modifier.periodictime;
            ++m_periodicTick;                               // for some infinity auras in some cases can overflow and reset
            PeriodicTick();
        }
    }
    else
    {
        if (m_periodicTimer <= 0)
        {
            m_periodicTimer = NONPERIODIC_AURA_UPDATE_INTERVAL;
            PeriodicCheck();
        }
        else
            m_periodicTimer -= diff;
    }
}

void Aura::AreaAuraUpdate(uint32 diff)
{
    // update for the caster of the aura
    if (GetCasterGuid() == GetTarget()->GetObjectGuid())
    {
        Unit* caster = GetTarget();

        if (!caster || !caster->GetMap())
            return;

        if ( !caster->hasUnitState(UNIT_STAT_ISOLATED) )
        {
            Unit* owner = caster->GetCharmerOrOwner();
            if (!owner)
                owner = caster;

            GuidSet targets;
            Spell::UnitList _targets;

            switch(m_areaAuraType)
            {
                case AREA_AURA_PARTY:
                {
                    Group *pGroup = NULL;

                    if (owner->GetTypeId() == TYPEID_PLAYER)
                        pGroup = ((Player*)owner)->GetGroup();

                    if ( pGroup)
                    {
                        uint8 subgroup = ((Player*)owner)->GetSubGroup();
                        for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                        {
                            Player* Target = itr->getSource();
                            if (Target && Target->IsInWorld() && Target->isAlive() && Target->GetSubGroup()==subgroup && caster->IsInWorld() && caster->IsFriendlyTo(Target))
                            {
                                if (caster->IsWithinDistInMap(Target, m_radius))
                                    targets.insert(Target->GetObjectGuid());
                                if (Target->GetPet())
                                {
                                    GroupPetList m_groupPets = Target->GetPets();
                                    if (!m_groupPets.empty())
                                    {
                                        for (GroupPetList::const_iterator itr = m_groupPets.begin(); itr != m_groupPets.end(); ++itr)
                                            if (Pet* _pet = Target->GetMap()->GetPet(*itr))
                                                if (_pet && _pet->IsInWorld() && _pet->isAlive() && caster->IsWithinDistInMap(_pet, m_radius))
                                                    targets.insert(_pet->GetObjectGuid());
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        // add owner
                        if ( owner != caster && caster->IsWithinDistInMap(owner, m_radius) )
                            targets.insert(owner->GetObjectGuid());
                        // add caster's pet
                        if (caster->GetPet())
                        {
                            GroupPetList m_groupPets = caster->GetPets();
                            if (!m_groupPets.empty())
                            {
                                for (GroupPetList::const_iterator itr = m_groupPets.begin(); itr != m_groupPets.end(); ++itr)
                                    if (Pet* _pet = caster->GetMap()->GetPet(*itr))
                                        if (_pet && _pet->IsInWorld() && caster->IsWithinDistInMap(_pet, m_radius))
                                            targets.insert(_pet->GetObjectGuid());
                            }
                        }
                    }
                    break;
                }
                case AREA_AURA_RAID:
                {
                    Group *pGroup = NULL;

                    if (owner->GetTypeId() == TYPEID_PLAYER)
                        pGroup = ((Player*)owner)->GetGroup();

                    if ( pGroup)
                    {
                        for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                        {
                            Player* Target = itr->getSource();
                            if (Target && Target->IsInWorld() && Target->isAlive() && caster->IsInWorld() && caster->IsFriendlyTo(Target))
                            {
                                if (caster->IsWithinDistInMap(Target, m_radius))
                                    targets.insert(Target->GetObjectGuid());
                                if (Target->GetPet())
                                {
                                    GroupPetList m_groupPets = Target->GetPets();
                                    if (!m_groupPets.empty())
                                    {
                                        for (GroupPetList::const_iterator itr = m_groupPets.begin(); itr != m_groupPets.end(); ++itr)
                                            if (Pet* _pet = caster->GetMap()->GetPet(*itr))
                                                if (_pet && _pet->IsInWorld() && caster->IsWithinDistInMap(_pet, m_radius))
                                                    targets.insert(_pet->GetObjectGuid());
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        // add owner
                        if ( owner != caster && caster->IsWithinDistInMap(owner, m_radius) )
                            targets.insert(owner->GetObjectGuid());
                        // add caster's pet
                        if (caster->GetPet())
                        {
                            GroupPetList m_groupPets = caster->GetPets();
                            if (!m_groupPets.empty())
                            {
                                for (GroupPetList::const_iterator itr = m_groupPets.begin(); itr != m_groupPets.end(); ++itr)
                                    if (Pet* _pet = caster->GetMap()->GetPet(*itr))
                                        if (_pet && _pet->IsInWorld() && caster->IsWithinDistInMap(_pet, m_radius))
                                            targets.insert(_pet->GetObjectGuid());
                            }
                        }
                    }
                    break;
                }
                case AREA_AURA_FRIEND:
                {
                    MaNGOS::AnyFriendlyUnitInObjectRangeCheck u_check(caster, m_radius);
                    MaNGOS::UnitListSearcher<MaNGOS::AnyFriendlyUnitInObjectRangeCheck> searcher(_targets, u_check);
                    Cell::VisitAllObjects(caster, searcher, m_radius);
                    break;
                }
                case AREA_AURA_ENEMY:
                {
                    MaNGOS::AnyAoETargetUnitInObjectRangeCheck u_check(caster, m_radius); // No GetCharmer in searcher
                    MaNGOS::UnitListSearcher<MaNGOS::AnyAoETargetUnitInObjectRangeCheck> searcher(_targets, u_check);
                    Cell::VisitAllObjects(caster, searcher, m_radius);
                    break;
                }
                case AREA_AURA_OWNER:
                case AREA_AURA_PET:
                {
                    if (owner != caster && caster->IsInWorld() && caster->IsWithinDistInMap(owner, m_radius))
                        targets.insert(owner->GetObjectGuid());
                    break;
                }
                default:
                    break;
            }

            if (!_targets.empty())
                for (Spell::UnitList::iterator itr = _targets.begin(); itr != _targets.end(); ++itr)
                    if (*itr)
                        targets.insert((*itr)->GetObjectGuid());

            for (GuidSet::const_iterator tIter = targets.begin(); tIter != targets.end(); tIter++)
            {
                // flag for selection is need apply aura to current iteration target
                bool apply = true;

                // we need ignore present caster self applied are auras sometime
                // in cases if this only auras applied for spell effect
                Unit* i_target = caster->GetMap()->GetUnit(*tIter);
                if (!i_target)
                    continue;

                if (i_target->GetTypeId() == TYPEID_PLAYER && ((Player*)i_target)->IsBeingTeleportedFar())
                    continue;

                if (GetSpellProto()->HasAttribute(SPELL_ATTR_EX3_TARGET_ONLY_PLAYER) && i_target->GetTypeId() != TYPEID_PLAYER)
                    continue;

                if (i_target->IsImmuneToSpell(GetSpellProto(), GetAffectiveCaster() ? GetAffectiveCaster()->IsFriendlyTo(i_target) : true))
                    continue;
                else
                {
                    MAPLOCK_READ(i_target,MAP_LOCK_TYPE_AURAS);
                    Unit::SpellAuraHolderBounds spair = i_target->GetSpellAuraHolderBounds(GetId());
                    for(Unit::SpellAuraHolderMap::const_iterator i = spair.first; i != spair.second; ++i)
                    {
                        if (!i->second || i->second->IsDeleted())
                            continue;

                        Aura* aur = i->second->GetAuraByEffectIndex(m_effIndex);

                        if (!aur)
                            continue;

                        switch(m_areaAuraType)
                        {
                            case AREA_AURA_ENEMY:
                                // non caster self-casted auras (non stacked)
                                if (aur->GetModifier()->m_auraname != SPELL_AURA_NONE)
                                    apply = false;
                                break;
                            case AREA_AURA_RAID:
                                // non caster self-casted auras (stacked from diff. casters)
                                if (aur->GetModifier()->m_auraname != SPELL_AURA_NONE  || i->second->GetCasterGuid() == GetAffectiveCasterGuid())
                                    apply = false;
                                break;
                            default:
                                // in generic case not allow stacking area auras
                                apply = false;
                                break;
                        }

                        if(!apply)
                            break;
                    }
                }

                if(!apply)
                    continue;

                if (SpellEntry const *actualSpellInfo = sSpellMgr.SelectAuraRankForLevel(GetSpellProto(), i_target->getLevel()))
                {
                    int32 actualBasePoints = m_currentBasePoints;
                    // recalculate basepoints for lower rank (all AreaAura spell not use custom basepoints?)
                    if (actualSpellInfo != GetSpellProto())
                        actualBasePoints = actualSpellInfo->CalculateSimpleValue(m_effIndex);

                    SpellAuraHolderPtr holder = i_target->GetSpellAuraHolder(actualSpellInfo->Id, GetAffectiveCasterGuid());

                    if (!holder || holder->IsDeleted())
                    {
                        SpellAuraHolderPtr newholder = CreateSpellAuraHolder(actualSpellInfo, i_target,  GetAffectiveCaster());
                        newholder->SetAuraDuration(GetAuraDuration());
                        /*Aura* aura = */newholder->CreateAura(AURA_CLASS_AREA_AURA, m_effIndex, &actualBasePoints, newholder, i_target, GetAffectiveCaster(), NULL);
                        i_target->AddSpellAuraHolder(newholder);
                    }
                    else
                    {
                        holder->SetAuraDuration(GetAuraDuration());
                        Aura* aura = holder->GetAuraByEffectIndex(m_effIndex);
                        if (aura)
                        {
                            //holder->SetInUse(true);
                            aura->ApplyModifier(false,true);
                            aura->GetModifier()->m_amount = actualBasePoints;
                            aura->ApplyModifier(true,true);
                            //holder->SetInUse(false);
                        }
                        else
                        {
                            Aura* aura = holder->CreateAura(AURA_CLASS_AREA_AURA, m_effIndex, &actualBasePoints, holder, i_target, GetAffectiveCaster(), NULL);
                            i_target->AddAuraToModList(aura);
                            //holder->SetInUse(true);
                            aura->ApplyModifier(true,true);
                            //holder->SetInUse(false);
                        }
                    }
                }
            }
        }
        Update(diff);
    }
    else                                                    // aura at non-caster
    {
        Unit* realcaster = GetAffectiveCaster();
        Unit* caster = GetCaster();
        Unit* target = GetTarget();

        Update(diff);

        // remove aura if out-of-range from caster (after teleport for example)
        // or caster is isolated or caster no longer has the aura
        // or caster is (no longer) friendly
        bool needFriendly = (m_areaAuraType == AREA_AURA_ENEMY ? false : true);
        if ( !caster || caster->hasUnitState(UNIT_STAT_ISOLATED) ||
            !realcaster->IsInMap(target)      ||
            !caster->IsWithinDistInMap(target, m_radius)        ||
            !caster->HasAura(GetId(), GetEffIndex())            ||
            caster->IsFriendlyTo(target) != needFriendly
           )
        {
            target->RemoveSingleAuraFromSpellAuraHolder(GetId(), GetEffIndex(), GetAffectiveCasterGuid());
        }
        else if ( m_areaAuraType == AREA_AURA_PARTY)         // check if in same sub group
        {
            // not check group if target == owner or target == pet
            if (realcaster->GetCharmerOrOwnerGuid() != target->GetObjectGuid() && realcaster->GetObjectGuid() != target->GetCharmerOrOwnerGuid())
            {
                Player* check = realcaster->GetCharmerOrOwnerPlayerOrPlayerItself();

                Group *pGroup = check ? check->GetGroup() : NULL;
                if ( pGroup )
                {
                    Player* checkTarget = target->GetCharmerOrOwnerPlayerOrPlayerItself();
                    if(!checkTarget || !pGroup->SameSubGroup(check, checkTarget))
                        target->RemoveSingleAuraFromSpellAuraHolder(GetId(), GetEffIndex(), GetAffectiveCasterGuid());
                }
                else
                    target->RemoveSingleAuraFromSpellAuraHolder(GetId(), GetEffIndex(), GetAffectiveCasterGuid());
            }
        }
        else if ( m_areaAuraType == AREA_AURA_RAID)          // TODO: fix me!
        {
            // not check group if target == owner or target == pet
            if (realcaster->GetCharmerOrOwnerGuid() != target->GetObjectGuid() && realcaster->GetObjectGuid() != target->GetCharmerOrOwnerGuid())
            {
                Player* check = realcaster->GetCharmerOrOwnerPlayerOrPlayerItself();

                Group *pGroup = check ? check->GetGroup() : NULL;
                if ( pGroup )
                {
                    Player* checkTarget = target->GetCharmerOrOwnerPlayerOrPlayerItself();
                    if(!checkTarget)
                        target->RemoveSingleAuraFromSpellAuraHolder(GetId(), GetEffIndex(), GetAffectiveCasterGuid());
                }
                else
                    target->RemoveSingleAuraFromSpellAuraHolder(GetId(), GetEffIndex(), GetCasterGuid());
            }
        }
        else if (m_areaAuraType == AREA_AURA_PET || m_areaAuraType == AREA_AURA_OWNER)
        {
            if (target->GetObjectGuid() != realcaster->GetCharmerOrOwnerGuid())
                target->RemoveSingleAuraFromSpellAuraHolder(GetId(), GetEffIndex(), GetAffectiveCasterGuid());
        }
    }
}

void Aura::PersistentAreaAuraUpdate(uint32 diff)
{
    bool remove = false;

    // remove the aura if its caster or the dynamic object causing it was removed
    // or if the target moves too far from the dynamic object
    if (Unit *caster = GetCaster())
    {
        DynamicObject *dynObj = caster->GetDynObject(GetId(), GetEffIndex());
        if (dynObj)
        {
            if (!GetTarget()->IsWithinDistInMap(dynObj, dynObj->GetRadius()))
            {
                remove = true;
                dynObj->RemoveAffected(GetTarget());        // let later reapply if target return to range
            }
        }
        else
            remove = true;
    }
    else
        remove = true;

    Update(diff);

    if (remove)
        GetTarget()->RemoveAura(GetId(), GetEffIndex());
}

void Aura::ApplyModifier(bool apply, bool Real)
{
    AuraType aura = m_modifier.m_auraname;

    GetHolder()->SetInUse(true);
    if (aura < TOTAL_AURAS)
        (*this.*AuraHandler [aura])(apply, Real);
    GetHolder()->SetInUse(false);
}

bool Aura::isAffectedOnSpell(SpellEntry const *spell) const
{
    return spell->IsFitToFamily(SpellFamily(GetSpellProto()->SpellFamilyName), GetAuraSpellClassMask());
}

bool Aura::CanProcFrom(SpellEntry const *spell, uint32 procFlag, uint32 EventProcEx, uint32 procEx, bool active, bool useClassMask) const
{
    // Check EffectClassMask
    ClassFamilyMask const& mask  = GetAuraSpellClassMask();

    // allow proc for modifier auras with charges
    if (IsCastEndProcModifierAura(GetSpellProto(), GetEffIndex(), spell))
    {
        if (GetHolder()->GetAuraCharges() > 0)
        {
            if (procEx != PROC_EX_CAST_END && EventProcEx == PROC_EX_NONE)
                return false;
        }
    }
    else if (EventProcEx == PROC_EX_NONE && procEx == PROC_EX_CAST_END)
        return false;

    // if no class mask defined, or spell_proc_event has SpellFamilyName=0 - allow proc
    if (!useClassMask || !mask)
    {
        if (!(EventProcEx & PROC_EX_EX_TRIGGER_ALWAYS))
        {
            // Check for extra req (if none) and hit/crit
            if (EventProcEx == PROC_EX_NONE)
            {
                // No extra req, so can trigger only for active (damage/healing present) and hit/crit
                if(((procEx & (PROC_EX_NORMAL_HIT|PROC_EX_CRITICAL_HIT)) && active) || procEx == PROC_EX_CAST_END)
                    return true;
                else
                    return false;
            }
            // Passive spells hits here only if resist/reflect/immune/evade
            // Passive spells can`t trigger if need hit (exclude cases when procExtra include non-active flags)
            else if ((EventProcEx & (PROC_EX_NORMAL_HIT|PROC_EX_CRITICAL_HIT) & procEx) && !active)
                    return false;
            // Custom procs with aura apply/fade (must fit in aura mask, if exists, independent from SchoolMask)
            else if ((procFlag & PROC_FLAG_ON_AURA_APPLY) || (procFlag & PROC_FLAG_ON_AURA_FADE))
                    if (mask && !isAffectedOnSpell(spell))
                        return false;
        }
        return true;
    }
    else
    {
        // SpellFamilyName check is performed in SpellMgr::IsSpellProcEventCanTriggeredBy and it is done once for whole holder
        // note: SpellFamilyName is not checked if no spell_proc_event is defined
        return mask.IsFitToFamilyMask(spell->GetSpellFamilyFlags());
    }
}

void Aura::ReapplyAffectedPassiveAuras( Unit* target, bool owner_mode )
{
    if (!target)
        return;

    // we need store cast item guids for self casted spells
    // expected that not exist permanent auras from stackable auras from different items
    std::map<uint32, ObjectGuid> affectedSelf;

    Unit::SpellIdSet affectedAuraCaster;

    {
        MAPLOCK_READ(target,MAP_LOCK_TYPE_AURAS);
        Unit::SpellAuraHolderMap const& holderMap = target->GetSpellAuraHolderMap();
        for (Unit::SpellAuraHolderMap::const_iterator itr = holderMap.begin(); itr != holderMap.end(); ++itr)
        {
            if (!itr->second || itr->second->IsDeleted())
                continue;

            // permanent passive or permanent area aura
            // passive spells can be affected only by own or owner spell mods)
            if ((itr->second->IsPermanent() && ((owner_mode && itr->second->IsPassive()) || itr->second->IsAreaAura())) &&
                // non deleted and not same aura (any with same spell id)
                itr->second->GetId() != GetId() &&
                // and affected by aura
                isAffectedOnSpell(itr->second->GetSpellProto()))
            {
                // only applied by self or aura caster
                if (itr->second->GetCasterGuid() == target->GetObjectGuid())
                    affectedSelf[itr->second->GetId()] = itr->second->GetCastItemGuid();
                else if (itr->second->GetCasterGuid() == GetCasterGuid())
                    affectedAuraCaster.insert(itr->second->GetId());
            }
        }
    }

    if (!affectedSelf.empty())
    {
        Player* pTarget = target->GetTypeId() == TYPEID_PLAYER ? (Player*)target : NULL;

        for(std::map<uint32, ObjectGuid>::const_iterator map_itr = affectedSelf.begin(); map_itr != affectedSelf.end(); ++map_itr)
        {
            Item* item = pTarget && map_itr->second ? pTarget->GetItemByGuid(map_itr->second) : NULL;
            target->RemoveAurasDueToSpell(map_itr->first);
            target->CastSpell(target, map_itr->first, true, item);
        }
    }

    if (!affectedAuraCaster.empty())
    {
        Unit* caster = GetCaster();
        for(Unit::SpellIdSet::const_iterator set_itr = affectedAuraCaster.begin(); set_itr != affectedAuraCaster.end(); ++set_itr)
        {
            target->RemoveAurasDueToSpell(*set_itr);
            if (caster)
                caster->CastSpell(GetTarget(), *set_itr, true);
        }
    }
}

struct ReapplyAffectedPassiveAurasHelper
{
    explicit ReapplyAffectedPassiveAurasHelper(Aura* _aura) : aura(_aura) {}
    void operator()(Unit* unit) const { aura->ReapplyAffectedPassiveAuras(unit, true); }
    Aura* aura;
};

void Aura::ReapplyAffectedPassiveAuras()
{
    // not reapply spell mods with charges (use original value because processed and at remove)
    if (GetSpellProto()->procCharges)
        return;

    // not reapply some spell mods ops (mostly speedup case)
    switch (m_modifier.m_miscvalue)
    {
        case SPELLMOD_DURATION:
        case SPELLMOD_CHARGES:
        case SPELLMOD_NOT_LOSE_CASTING_TIME:
        case SPELLMOD_CASTING_TIME:
        case SPELLMOD_COOLDOWN:
        case SPELLMOD_COST:
        case SPELLMOD_ACTIVATION_TIME:
        case SPELLMOD_GLOBAL_COOLDOWN:
            return;
    }

    // reapply talents to own passive persistent auras
    ReapplyAffectedPassiveAuras(GetTarget(), true);

    // re-apply talents/passives/area auras applied to pet/totems (it affected by player spellmods)
    GetTarget()->CallForAllControlledUnits(ReapplyAffectedPassiveAurasHelper(this), CONTROLLED_PET|CONTROLLED_TOTEMS);

    // re-apply talents/passives/area auras applied to group members (it affected by player spellmods)
    if (Group* group = ((Player*)GetTarget())->GetGroup())
        for(GroupReference *itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
            if (Player* member = itr->getSource())
                if (member != GetTarget() && member->IsInMap(GetTarget()))
                    ReapplyAffectedPassiveAuras(member, false);
}

bool Aura::IsEffectStacking()
{
    SpellEntry const *spellProto = GetSpellProto();

    // generic check
    // <sid>
    // TODO rework this, need find another way or use as new field
    // if (spellProto->AttributesEx6 & (SPELL_ATTR_EX6_NO_STACK_DEBUFF_MAJOR | SPELL_ATTR_EX6_NO_STACK_BUFF))
    {
        // Mark/Gift of the Wild early exception check
        if (spellProto->IsFitToFamily<SPELLFAMILY_DRUID, CF_DRUID_MARK_OF_THE_WILD>())
        {
            // only mod resistance exclusive isn't stacking
            return (GetModifier()->m_auraname != SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE);
        }
        else
            return false;
    }

    // scrolls don't stack
    if (GetSpellSpecific(spellProto->Id) == SPELL_SCROLL)
        return false;

    // some hardcoded checks are needed (given attrEx6 not present)
    switch(GetModifier()->m_auraname)
    {
        // these effects never stack
        case SPELL_AURA_MOD_MELEE_HASTE:
            if (spellProto->SpellFamilyName == SPELLFAMILY_GENERIC)
                return true;

            break;
        case SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE:
        case SPELL_AURA_MOD_HEALING_PCT:                                                       // Mortal Strike / Wound Poison / Aimed Shot / Furious Attacks
        case SPELL_AURA_MOD_STAT:                                                              // various stat buffs
            return (spellProto->SpellFamilyName == SPELLFAMILY_GENERIC);
        case SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK:                                           // Wrath of Air Totem / Mind-Numbing Poison and many more
            return (spellProto->CalculateSimpleValue(m_effIndex) > 0);
        case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE:                                               // Ferocious Inspiration / Sanctified Retribution
            if (spellProto->IsFitToFamily<SPELLFAMILY_PALADIN, CF_PALADIN_RETRIBUTION_AURA, CF_PALADIN_HEART_OF_THE_CRUSADER>()) // Sanctified Retribution / HoC
            {
                return false;
            }
            break;
        case SPELL_AURA_MOD_RESISTANCE_PCT:                                                    // Sunder Armor / Sting
        case SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN:                                              // Ebon Plague (spell not implemented) / Earth and Moon
            if (spellProto->IsFitToFamily<SPELLFAMILY_WARRIOR, CF_WARRIOR_SUNDER_ARMOR>() ||   // Sunder Armor (only spell triggering this aura has the flag)
                spellProto->IsFitToFamily<SPELLFAMILY_HUNTER,  CF_HUNTER_PET_SPELLS>() ||      // Sting (Hunter Pet)
                (spellProto->SpellFamilyName == SPELLFAMILY_DRUID && spellProto->SpellIconID == 2991) || // Earth and Moon
                spellProto->IsFitToFamily<SPELLFAMILY_ROGUE,  CF_ROGUE_MIND_NUMBING_POISON>() ||  // Mind-Numbing Poison
                spellProto->IsFitToFamily<SPELLFAMILY_PRIEST, CF_PRIEST_MISC_TALENTS>())       // Inspiration
            {
                return false;
            }
            break;
        case SPELL_AURA_MOD_CRIT_PERCENT:                                                      // Rampage
            if (spellProto->SpellFamilyName == SPELLFAMILY_WARRIOR && spellProto->SpellIconID == 2006)
                return false;
            break;
        case SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE:                                        // Winter's Chill / Improved Scorch
            if (spellProto->SpellFamilyName == SPELLFAMILY_MAGE)
                return false;
            break;
        case SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE:                  // Misery / Imp. Faerie Fire (must find triggered aura / spell not implemented?)
            if (spellProto->SpellFamilyName == SPELLFAMILY_PRIEST &&
                spellProto->SpellIconID == 2211)                        // Misery
            {
                return false;
            }
            break;

        default:
            break;
    }

    return true;
}

/*********************************************************/
/***               BASIC AURA FUNCTION                 ***/
/*********************************************************/
void Aura::HandleAddModifier(bool apply, bool Real)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER || !Real)
        return;

    if (m_modifier.m_miscvalue >= MAX_SPELLMOD)
        return;

    if (apply)
    {
        SpellEntry const* spellProto = GetSpellProto();

        // Add custom charges for some mod aura
        switch (spellProto->Id)
        {
            case 17941:                                     // Shadow Trance
            case 22008:                                     // Netherwind Focus
            case 31834:                                     // Light's Grace
            case 34754:                                     // Clearcasting
            case 34936:                                     // Backlash
            case 44401:                                     // Missile Barrage
            case 48108:                                     // Hot Streak
            case 51124:                                     // Killing Machine
            case 54741:                                     // Firestarter
            case 57761:                                     // Fireball!
            case 64823:                                     // Elune's Wrath (Balance druid t8 set
                GetHolder()->SetAuraCharges(1);
                break;
            default:
                break;
        }
    }

    ((Player*)GetTarget())->AddSpellMod(this, apply);

    ReapplyAffectedPassiveAuras();
}

void Aura::TriggerSpell()
{
    ObjectGuid casterGUID = GetCasterGuid();
    Unit* triggerTarget = GetTriggerTarget();

    if (!casterGUID || !triggerTarget)
        return;

    // generic casting code with custom spells and target/caster customs
    uint32 trigger_spell_id = GetSpellProto()->EffectTriggerSpell[m_effIndex];

    SpellEntry const* triggeredSpellInfo = sSpellStore.LookupEntry(trigger_spell_id);
    SpellEntry const* auraSpellInfo = GetSpellProto();
    uint32 auraId = auraSpellInfo->Id;
    Unit* target = GetTarget();
    Unit* triggerCaster = triggerTarget;
    WorldObject* triggerTargetObject = NULL;

    // specific code for cases with no trigger spell provided in field
    if (triggeredSpellInfo == NULL)
    {
        switch(auraSpellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
            {
                switch(auraId)
                {
                    case 812:                               // Periodic Mana Burn
                    {
                        trigger_spell_id = 25779;           // Mana Burn

                        if (GetTarget()->GetTypeId() != TYPEID_UNIT)
                            return;

                        triggerTarget = ((Creature*)GetTarget())->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, trigger_spell_id, SELECT_FLAG_POWER_MANA);
                        if (!triggerTarget)
                            return;

                        break;
                    }
//                    // Polymorphic Ray
//                    case 6965: break;
                    case 9712:                              // Thaumaturgy Channel
                        trigger_spell_id = 21029;
                        break;
//                    // Egan's Blaster
//                    case 17368: break;
//                    // Haunted
//                    case 18347: break;
//                    // Ranshalla Waiting
//                    case 18953: break;
//                    // Inferno
//                    case 19695: break;
//                    // Frostwolf Muzzle DND
//                    case 21794: break;
//                    // Alterac Ram Collar DND
//                    case 21866: break;
//                    // Celebras Waiting
//                    case 21916: break;
                    case 23170:                             // Brood Affliction: Bronze
                    {
                        target->CastSpell(target, 23171, true, NULL, this);
                        return;
                    }
                    case 23184:                             // Mark of Frost
                    case 25041:                             // Mark of Nature
                    case 37125:                             // Mark of Death
                    {
                        std::list<Player*> targets;

                        // spells existed in 1.x.x; 23183 - mark of frost; 25042 - mark of nature; both had radius of 100.0 yards in 1.x.x DBC
                        // spells are used by Azuregos and the Emerald dragons in order to put a stun debuff on the players which resurrect during the encounter
                        // in order to implement the missing spells we need to make a grid search for hostile players and check their auras; if they are marked apply debuff
                        // spell 37127 used for the Mark of Death, is used server side, so it needs to be implemented here

                        uint32 markSpellId = 0;
                        uint32 debuffSpellId = 0;

                        switch (auraId)
                        {
                            case 23184:
                                markSpellId = 23182;
                                debuffSpellId = 23186;
                                break;
                            case 25041:
                                markSpellId = 25040;
                                debuffSpellId = 25043;
                                break;
                            case 37125:
                                markSpellId = 37128;
                                debuffSpellId = 37131;
                                break;
                        }

                        MaNGOS::AnyPlayerInObjectRangeWithAuraCheck u_check(GetTarget(), 100.0f, markSpellId);
                        MaNGOS::PlayerListSearcher<MaNGOS::AnyPlayerInObjectRangeWithAuraCheck > checker(targets, u_check);
                        Cell::VisitWorldObjects(GetTarget(), checker, 100.0f);

                        for (std::list<Player*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
                            (*itr)->CastSpell((*itr), debuffSpellId, true, NULL, NULL, casterGUID);

                        return;
                    }
                    case 23493:                             // Restoration
                    {
                        uint32 heal = triggerTarget->GetMaxHealth() / 10;
                        uint32 absorb = 0;
                        triggerTarget->CalculateHealAbsorb(heal, &absorb);
                        triggerTarget->DealHeal(triggerTarget, heal - absorb, auraSpellInfo, false, absorb);

                        if (int32 mana = triggerTarget->GetMaxPower(POWER_MANA))
                        {
                            mana /= 10;
                            triggerTarget->EnergizeBySpell(triggerTarget, 23493, mana, POWER_MANA);
                        }
                        return;
                    }
//                    // Stoneclaw Totem Passive TEST
//                    case 23792: break;
//                    // Axe Flurry
//                    case 24018: break;
                    case 24210:                             // Mark of Arlokk
                    {
                        // Replacement for (classic) spell 24211 (doesn't exist anymore)
                        std::list<Creature*> lList;

                        // Search for all Zulian Prowler in range
                        MaNGOS::AllCreaturesOfEntryInRangeCheck check(triggerTarget, 15101, 15.0f);
                        MaNGOS::CreatureListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(lList, check);
                        Cell::VisitGridObjects(triggerTarget, searcher, 15.0f);

                        for (std::list<Creature*>::const_iterator itr = lList.begin(); itr != lList.end(); ++itr)
                            if ((*itr)->isAlive())
                                (*itr)->AddThreat(triggerTarget, float(5000));

                        return;
                    }
//                    // Restoration
//                    case 24379: break;
//                    // Happy Pet
//                    case 24716: break;
                    case 24780:                             // Dream Fog
                    {
                        // Note: In 1.12 triggered spell 24781 still exists, need to script dummy effect for this spell then
                        // Select an unfriendly enemy in 100y range and attack it
                        if (target->GetTypeId() != TYPEID_UNIT)
                            return;

                        ThreatList const& tList = target->getThreatManager().getThreatList();
                        for (ThreatList::const_iterator itr = tList.begin();itr != tList.end(); ++itr)
                        {
                            Unit* pUnit = target->GetMap()->GetUnit((*itr)->getUnitGuid());

                            if (pUnit && target->getThreatManager().getThreat(pUnit))
                                target->getThreatManager().modifyThreatPercent(pUnit, -100);
                        }

                        if (Unit* pEnemy = target->SelectRandomUnfriendlyTarget(target->getVictim(), 100.0f))
                            ((Creature*)target)->AI()->AttackStart(pEnemy);

                        return;
                    }
//                    // Cannon Prep
//                    case 24832: break;
                    case 24834:                             // Shadow Bolt Whirl
                    {
                        uint32 spellForTick[8] = { 24820, 24821, 24822, 24823, 24835, 24836, 24837, 24838 };
                        uint32 tick = GetAuraTicks();
                        if (tick < 8)
                        {
                            trigger_spell_id = spellForTick[tick];

                            // casted in left/right (but triggered spell have wide forward cone)
                            float forward = target->GetOrientation();
                            float angle = target->GetOrientation() + ( tick % 2 == 0 ? M_PI_F / 2 : - M_PI_F / 2);
                            target->SetOrientation(angle);
                            triggerTarget->CastSpell(triggerTarget, trigger_spell_id, true, NULL, this, casterGUID);
                            target->SetOrientation(forward);
                        }
                        return;
                    }
//                    // Stink Trap
//                    case 24918: break;
//                    // Agro Drones
//                    case 25152: break;
                    case 25371:                             // Consume
                    {
                        int32 bpDamage = triggerTarget->GetMaxHealth()*10/100;
                        triggerTarget->CastCustomSpell(triggerTarget, 25373, &bpDamage, NULL, NULL, true, NULL, this, casterGUID);
                        return;
                    }
//                    // Pain Spike
//                    case 25572: break;
                    case 26009:                             // Rotate 360
                    case 26136:                             // Rotate -360
                    {
                        float newAngle = target->GetOrientation();

                        if (auraId == 26009)
                            newAngle += M_PI_F/40;
                        else
                            newAngle -= M_PI_F/40;

                        newAngle = MapManager::NormalizeOrientation(newAngle);

                        target->SetFacingTo(newAngle);

                        target->CastSpell(target, 26029, true);
                        return;
                    }
//                    // Consume
//                    case 26196: break;
//                    // Berserk
//                    case 26615: break;
//                    // Defile
//                    case 27177: break;
//                    // Teleport: IF/UC
//                    case 27601: break;
//                    // Five Fat Finger Exploding Heart Technique
//                    case 27673: break;
//                    // Nitrous Boost
//                    case 27746: break;
//                    // Steam Tank Passive
//                    case 27747: break;
                    case 27808:                             // Frost Blast
                    {
                        int32 bpDamage = triggerTarget->GetMaxHealth()*26/100;
                        triggerTarget->CastCustomSpell(triggerTarget, 29879, &bpDamage, NULL, NULL, true, NULL, this, casterGUID);
                        return;
                    }
                    // Detonate Mana
                    case 27819:
                    {
                        // 33% Mana Burn on normal mode, 50% on heroic mode
                        int32 bpDamage = (int32)triggerTarget->GetPower(POWER_MANA) / 3);
                        triggerTarget->ModifyPower(POWER_MANA, -bpDamage);
                        triggerTarget->CastCustomSpell(triggerTarget, 27820, &bpDamage, NULL, NULL, true, NULL, this, triggerTarget->GetObjectGuid());
                        return;
                    }
//                    // Controller Timer
//                    case 28095: break;
                    // Stalagg Chain and Feugen Chain
                    case 28096:
                    case 28111:
                    {
                        // X-Chain is casted by Tesla to X, so: caster == Tesla, target = X
                        Unit* pCaster = GetCaster();
                        if (pCaster && pCaster->GetTypeId() == TYPEID_UNIT && !pCaster->IsWithinDistInMap(target, 60.0f))
                        {
                            pCaster->InterruptNonMeleeSpells(true);
                            ((Creature*)pCaster)->SetInCombatWithZone();
                            // Stalagg Tesla Passive or Feugen Tesla Passive
                            pCaster->CastSpell(pCaster, auraId == 28096 ? 28097 : 28109, true, NULL, NULL, target->GetObjectGuid());
                        }
                        return;
                    }
                    // Stalagg Tesla Passive and Feugen Tesla Passive
                    case 28097:
                    case 28109:
                    {
                        // X-Tesla-Passive is casted by Tesla on Tesla with original caster X, so: caster = X, target = Tesla
                        Unit* pCaster = GetCaster();
                        if (pCaster && pCaster->GetTypeId() == TYPEID_UNIT)
                        {
                            if (pCaster->getVictim() && !pCaster->IsWithinDistInMap(target, 60.0f))
                            {
                                if (Unit* pTarget = ((Creature*)pCaster)->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                                    target->CastSpell(pTarget, 28099, false);// Shock
                            }
                            else
                            {
                                // "Evade"
                                target->RemoveAurasDueToSpell(auraId);
                                target->DeleteThreatList();
                                target->CombatStop(true);
                                // Recast chain (Stalagg Chain or Feugen Chain
                                target->CastSpell(pCaster, auraId == 28097 ? 28096 : 28111, false);
                            }
                        }
                        return;
                    }
//                    // Mark of Didier
//                    case 28114: break;
//                    // Communique Timer, camp
//                    case 28346: break;
                    case 28522:                             // Icebolt (Sapphiron - Naxxramas)
                        // dunno if triggered spell id is correct
                        if (!target->HasAura(45776))
                            trigger_spell_id = 45776;
                        break;
//                    // Silithyst
//                    case 29519: break;
                    case 29528:                             // Inoculate Nestlewood Owlkin
                        // prevent error reports in case ignored player target
                        if (triggerTarget->GetTypeId() != TYPEID_UNIT)
                            return;
                        break;
//                    // Overload
//                    case 29768: break;
//                    // Return Fire
//                    case 29788: break;
//                    // Return Fire
//                    case 29793: break;
//                    // Return Fire
//                    case 29794: break;
//                    // Guardian of Icecrown Passive
//                    case 29897: break;
                    case 29917:                             // Feed Captured Animal
                        trigger_spell_id = 29916;
                        break;
//                    // Flame Wreath
//                    case 29946: break;
//                    // Flame Wreath
//                    case 29947: break;
//                    // Mind Exhaustion Passive
//                    case 30025: break;
//                    // Nether Beam - Serenity
//                    case 30401: break;
                    case 30576:                             // Quake
                        trigger_spell_id = 30571;
                        break;
//                    // Burning Maul
//                    case 30598: break;
//                    // Regeneration
//                    case 30799:
//                    case 30800:
//                    case 30801:
//                        break;
//                    // Despawn Self - Smoke cloud
//                    case 31269: break;
//                    // Time Rift Periodic
//                    case 31320: break;
//                    // Corrupt Medivh
//                    case 31326: break;
                    case 31347:                             // Doom
                    {
                        target->CastSpell(target,31350,true);
                        target->DealDamage(target, target->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                        return;
                    }
                    case 31373:                             // Spellcloth
                    {
                        // Summon Elemental after create item
                        triggerTarget->SummonCreature(17870, 0.0f, 0.0f, 0.0f, triggerTarget->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
                        return;
                    }
//                    // Bloodmyst Tesla
//                    case 31611: break;
                    case 31944:                             // Doomfire
                    {
                        int32 damage = m_modifier.m_amount * ((GetAuraDuration() + m_modifier.periodictime) / GetAuraMaxDuration());
                        triggerTarget->CastCustomSpell(triggerTarget, 31969, &damage, NULL, NULL, true, NULL, this, casterGUID);
                        return;
                    }
//                    // Teleport Test
//                    case 32236: break;
//                    // Earthquake
//                    case 32686: break;
//                    // Possess
//                    case 33401: break;
//                    // Draw Shadows
//                    case 33563: break;
//                    // Murmur's Touch
//                    case 33711: break;
                    case 34229:                             // Flame Quills
                    {
                        // cast 24 spells 34269-34289, 34314-34316
                        for(uint32 spell_id = 34269; spell_id != 34290; ++spell_id)
                            triggerTarget->CastSpell(triggerTarget, spell_id, true, NULL, this, casterGUID);
                        for(uint32 spell_id = 34314; spell_id != 34317; ++spell_id)
                            triggerTarget->CastSpell(triggerTarget, spell_id, true, NULL, this, casterGUID);
                        return;
                    }
//                    // Gravity Lapse
//                    case 34480: break;
//                    // Tornado
//                    case 34683: break;
//                    // Frostbite Rotate
//                    case 34748: break;
//                    // Arcane Flurry
//                    case 34821: break;
//                    // Interrupt Shutdown
//                    case 35016: break;
//                    // Interrupt Shutdown
//                    case 35176: break;
//                    // Inferno
//                    case 35268: break;
//                    // Salaadin's Tesla
//                    case 35515: break;
//                    // Ethereal Channel (Red)
//                    case 35518: break;
//                    // Nether Vapor
//                    case 35879: break;
//                    // Dark Portal Storm
//                    case 36018: break;
//                    // Burning Maul
//                    case 36056: break;
//                    // Living Grove Defender Lifespan
//                    case 36061: break;
//                    // Professor Dabiri Talks
//                    case 36064: break;
//                    // Kael Gaining Power
//                    case 36091: break;
//                    // They Must Burn Bomb Aura
//                    case 36344: break;
//                    // They Must Burn Bomb Aura (self)
//                    case 36350: break;
//                    // Stolen Ravenous Ravager Egg
//                    case 36401: break;
//                    // Activated Cannon
//                    case 36410: break;
//                    // Stolen Ravenous Ravager Egg
//                    case 36418: break;
//                    // Enchanted Weapons
//                    case 36510: break;
//                    // Cursed Scarab Periodic
//                    case 36556: break;
//                    // Cursed Scarab Despawn Periodic
//                    case 36561: break;
//                    // Vision Guide
//                    case 36573: break;
//                    // Cannon Charging (platform)
//                    case 36785: break;
//                    // Cannon Charging (self)
//                    case 36860: break;
                    case 37027:                             // Remote Toy
                        trigger_spell_id = 37029;
                        break;
//                    // Mark of Death
//                    case 37125: break;
//                    // Arcane Flurry
//                    case 37268: break;
                    case 37429:                             // Spout (left)
                    case 37430:                             // Spout (right)
                    {
                        float newAngle = target->GetOrientation();

                        if (auraId == 37429)
                            newAngle += 2*M_PI_F/100;
                        else
                            newAngle -= 2*M_PI_F/100;

                        newAngle = MapManager::NormalizeOrientation(newAngle);

                        target->SetFacingTo(newAngle);

                        target->CastSpell(target, 37433, true);
                        return;
                    }
//                    // Karazhan - Chess NPC AI, Snapshot timer
//                    case 37440: break;
//                    // Karazhan - Chess NPC AI, action timer
//                    case 37504: break;
//                    // Karazhan - Chess: Is Square OCCUPIED aura (DND)
//                    case 39400: break;
//                    // Banish
//                    case 37546: break;
//                    // Shriveling Gaze
//                    case 37589: break;
//                    // Fake Aggro Radius (2 yd)
//                    case 37815: break;
//                    // Corrupt Medivh
//                    case 37853: break;
                    case 38495:                             // Eye of Grillok
                    {
                        target->CastSpell(target, 38530, true);
                        return;
                    }
                    case 38554:                             // Absorb Eye of Grillok (Zezzak's Shard)
                    {
                        if (target->GetTypeId() != TYPEID_UNIT)
                            return;

                        if (Unit* caster = GetCaster())
                            caster->CastSpell(caster, 38495, true, NULL, this);
                        else
                            return;

                        Creature* creatureTarget = (Creature*)target;

                        creatureTarget->ForcedDespawn();
                        return;
                    }
//                    // Magic Sucker Device timer
//                    case 38672: break;
//                    // Tomb Guarding Charging
//                    case 38751: break;
//                    // Murmur's Touch
//                    case 38794: break;
                    case 39105:                             // Activate Nether-wraith Beacon (31742 Nether-wraith Beacon item)
                    {
                        float fX, fY, fZ;
                        triggerTarget->GetClosePoint(fX, fY, fZ, triggerTarget->GetObjectBoundingRadius(), 20.0f);
                        triggerTarget->SummonCreature(22408, fX, fY, fZ, triggerTarget->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
                        return;
                    }
//                    // Drain World Tree Visual
//                    case 39140: break;
//                    // Quest - Dustin's Undead Dragon Visual aura
//                    case 39259: break;
//                    // Hellfire - The Exorcism, Jules releases darkness, aura
//                    case 39306: break;
//                    // Inferno
//                    case 39346: break;
//                    // Enchanted Weapons
//                    case 39489: break;
//                    // Shadow Bolt Whirl
//                    case 39630: break;
//                    // Shadow Bolt Whirl
//                    case 39634: break;
//                    // Shadow Inferno
//                    case 39645: break;
                    case 39857:                             // Tear of Azzinoth Summon Channel - it's not really supposed to do anything,and this only prevents the console spam
                        trigger_spell_id = 39856;
                        break;
//                    // Soulgrinder Ritual Visual (Smashed)
//                    case 39974: break;
//                    // Simon Game Pre-game timer
//                    case 40041: break;
//                    // Knockdown Fel Cannon: The Aggro Check Aura
//                    case 40113: break;
//                    // Spirit Lance
//                    case 40157: break;
//                    // Demon Transform 2
//                    case 40398: break;
//                    // Demon Transform 1
//                    case 40511: break;
//                    // Ancient Flames
//                    case 40657: break;
//                    // Ethereal Ring Cannon: Cannon Aura
//                    case 40734: break;
//                    // Cage Trap
//                    case 40760: break;
//                    // Random Periodic
//                    case 40867: break;
//                    // Prismatic Shield
//                    case 40879: break;
//                    // Aura of Desire
//                    case 41350: break;
//                    // Dementia
//                    case 41404: break;
//                    // Chaos Form
//                    case 41629: break;
//                    // Alert Drums
//                    case 42177: break;
//                    // Spout
//                    case 42581: break;
//                    // Spout
//                    case 42582: break;
//                    // Return to the Spirit Realm
//                    case 44035: break;
//                    // Curse of Boundless Agony
//                    case 45050: break;
//                    // Earthquake
//                    case 46240: break;
                    case 46736:                             // Personalized Weather
                        trigger_spell_id = 46737;
                        break;
//                    // Stay Submerged
//                    case 46981: break;
//                    // Dragonblight Ram
//                    case 47015: break;
                    case 51121:                             // Time Bomb
                    case 59376:
                    {
                        if (target)
                        {
                            int32 healthMissing = target->GetMaxHealth() - target->GetHealth();
                            target->CastCustomSpell(target, 51132, &healthMissing, NULL, NULL, true);
                        }
                        return;
                    }
//                    // Party G.R.E.N.A.D.E.
//                    case 51510: break;
//                    // Horseman Abilities
//                    case 52347: break;
//                    // GPS (Greater drake Positioning System)
//                    case 53389: break;
//                    // WotLK Prologue Frozen Shade Summon Aura
//                    case 53459: break;
//                    // WotLK Prologue Frozen Shade Speech
//                    case 53460: break;
//                    // WotLK Prologue Dual-plagued Brain Summon Aura
//                    case 54295: break;
//                    // WotLK Prologue Dual-plagued Brain Speech
//                    case 54299: break;
//                    // Rotate 360 (Fast)
//                    case 55861: break;
//                    // Shadow Sickle
//                    case 56702: break;
//                    // Portal Periodic
//                    case 58008: break;
//                    // Destroy Door Seal
//                    case 58040: break;
//                    // Draw Magic
//                    case 58185: break;
                    case 58886:                             // Food
                    {
                        if (GetAuraTicks() != 1)
                            return;

                        uint32 randomBuff[5] = {57288, 57139, 57111, 57286, 57291};

                        trigger_spell_id = urand(0, 1) ? 58891 : randomBuff[urand(0, 4)];

                        break;
                    }
//                    // Shadow Sickle
//                    case 59103: break;
//                    // Time Bomb
//                    case 59376: break;
//                    // Whirlwind Visual
//                    case 59551: break;
//                    // Hearstrike
//                    case 59783: break;
//                    // Z Check
//                    case 61678: break;
//                    // isDead Check
//                    case 61976: break;
//                    // Start the Engine
//                    case 62432: break;
//                    // Enchanted Broom
//                    case 62571: break;
//                    // Mulgore Hatchling
//                    case 62586: break;
                    case 62679:                             // Durotar Scorpion
                        trigger_spell_id = auraSpellInfo->CalculateSimpleValue(m_effIndex);
                        break;
//                    // Fighting Fish
//                    case 62833: break;
//                    // Shield Level 1
//                    case 63130: break;
//                    // Shield Level 2
//                    case 63131: break;
//                    // Shield Level 3
//                    case 63132: break;
//                    // Food
//                    case 64345: break;
//                    // Remove Player from Phase
//                    case 64445: break;
//                    // Food
//                    case 65418: break;
//                    // Food
//                    case 65419: break;
//                    // Food
//                    case 65420: break;
//                    // Food
//                    case 65421: break;
//                    // Food
//                    case 65422: break;
//                    // Rolling Throw
//                    case 67546: break;
                    case 69012:                             // Explosive Barrage - Krick and Ick
                    {
                        if (triggerTarget->GetTypeId() == TYPEID_UNIT)
                        {
                            if (Unit* pTarget = ((Creature*)triggerTarget)->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                            {
                                triggerTarget->CastSpell(pTarget, 69015, true);
                            }
                        }
                        return;
                    }
                    case 70017:                             // Gunship Cannon Fire
                        trigger_spell_id = 70021;
                        break;
//                    // Ice Tomb
//                    case 70157: break;
                    case 70842:                             // Mana Barrier
                    {
                        if (!triggerTarget || triggerTarget->getPowerType() != POWER_MANA)
                            return;

                        int32 damage = triggerTarget->GetHealth() - triggerTarget->GetMaxHealth();
                        if (damage >= 0)
                            return;

                        if (int32(triggerTarget->GetPower(POWER_MANA)) < abs(damage))
                        {
                            damage = -int32(triggerTarget->GetPower(POWER_MANA));
                            triggerTarget->RemoveAurasDueToSpell(auraId);
                        }

                        triggerTarget->DealHeal(triggerTarget, -damage, auraSpellInfo);
                        triggerTarget->ModifyPower(POWER_MANA, damage);
                        break;
                    }
//                    // Summon Timer: Suppresser
//                    case 70912: break;
//                    // Aura of Darkness
//                    case 71110: break;
//                    // Aura of Darkness
//                    case 71111: break;
//                    // Ball of Flames Visual
//                    case 71706: break;
//                    // Summon Broken Frostmourne
//                    case 74081: break;
                    default:
                        break;
                }
                break;
            }
            case SPELLFAMILY_MAGE:
            {
                switch(auraId)
                {
                    case 66:                                // Invisibility
                        // Here need periodic trigger reducing threat spell (or do it manually)
                        return;
                    default:
                        break;
                }
                break;
            }
//            case SPELLFAMILY_WARRIOR:
//            {
//                switch(auraId)
//                {
//                    // Wild Magic
//                    case 23410: break;
//                    // Corrupted Totems
//                    case 23425: break;
//                    default:
//                        break;
//                }
//                break;
//            }
//            case SPELLFAMILY_PRIEST:
//            {
//                switch(auraId)
//                {
//                    // Blue Beam
//                    case 32930: break;
//                    // Fury of the Dreghood Elders
//                    case 35460: break;
//                    default:
//                        break;
//                }
//                break;
//            }
            case SPELLFAMILY_HUNTER:
            {
                switch (auraId)
                {
                    case 53302:                             // Sniper training
                    case 53303:
                    case 53304:
                        if (triggerTarget->GetTypeId() != TYPEID_PLAYER)
                            return;

                        // Reset reapply counter at move
                        if (((Player*)triggerTarget)->isMoving())
                        {
                            m_modifier.m_amount = 6;
                            return;
                        }

                        // We are standing at the moment
                        if (m_modifier.m_amount > 0)
                        {
                            --m_modifier.m_amount;
                            return;
                        }

                        // select rank of buff
                        switch(auraId)
                        {
                            case 53302: trigger_spell_id = 64418; break;
                            case 53303: trigger_spell_id = 64419; break;
                            case 53304: trigger_spell_id = 64420; break;
                        }

                        // If aura is active - no need to continue
                        if (triggerTarget->HasAura(trigger_spell_id))
                            return;

                        break;
                    default:
                        break;
                }
                break;
            }
            case SPELLFAMILY_DRUID:
            {
                switch(auraId)
                {
                    case 768:                               // Cat Form
                        // trigger_spell_id not set and unknown effect triggered in this case, ignoring for while
                        return;
                    case 22842:                             // Frenzied Regeneration
                    case 22895:
                    case 22896:
                    case 26999:
                    {
                        int32 LifePerRage = GetModifier()->m_amount;

                        int32 lRage = target->GetPower(POWER_RAGE);
                        if (lRage > 100)                    // rage stored as rage*10
                            lRage = 100;
                        target->ModifyPower(POWER_RAGE, -lRage);
                        int32 FRTriggerBasePoints = int32(lRage*LifePerRage/10);
                        target->CastCustomSpell(target, 22845, &FRTriggerBasePoints, NULL, NULL, true, NULL, this);
                        return;
                    }
                    default:
                        break;
                }
                break;
            }

//            case SPELLFAMILY_HUNTER:
//            {
//                switch(auraId)
//                {
//                    //Frost Trap Aura
//                    case 13810:
//                        return;
//                    //Rizzle's Frost Trap
//                    case 39900:
//                        return;
//                    // Tame spells
//                    case 19597:         // Tame Ice Claw Bear
//                    case 19676:         // Tame Snow Leopard
//                    case 19677:         // Tame Large Crag Boar
//                    case 19678:         // Tame Adult Plainstrider
//                    case 19679:         // Tame Prairie Stalker
//                    case 19680:         // Tame Swoop
//                    case 19681:         // Tame Dire Mottled Boar
//                    case 19682:         // Tame Surf Crawler
//                    case 19683:         // Tame Armored Scorpid
//                    case 19684:         // Tame Webwood Lurker
//                    case 19685:         // Tame Nightsaber Stalker
//                    case 19686:         // Tame Strigid Screecher
//                    case 30100:         // Tame Crazed Dragonhawk
//                    case 30103:         // Tame Elder Springpaw
//                    case 30104:         // Tame Mistbat
//                    case 30647:         // Tame Barbed Crawler
//                    case 30648:         // Tame Greater Timberstrider
//                    case 30652:         // Tame Nightstalker
//                        return;
//                    default:
//                        break;
//                }
//                break;
//            }
            case SPELLFAMILY_SHAMAN:
            {
                switch(auraId)
                {
                    case 28820:                             // Lightning Shield (The Earthshatterer set trigger after cast Lighting Shield)
                    {
                        // Need remove self if Lightning Shield not active
                        Unit::SpellAuraHolderMap const& auras = triggerTarget->GetSpellAuraHolderMap();
                        for(Unit::SpellAuraHolderMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                        {
                            SpellEntry const* spell = itr->second->GetSpellProto();
                            if (spell->SpellFamilyName == SPELLFAMILY_SHAMAN &&
                                spell->GetSpellFamilyFlags().test<CF_SHAMAN_LIGHTNING_SHIELD>())
                                return;
                        }
                        triggerTarget->RemoveAurasDueToSpell(28820);
                        return;
                    }
                    case 38443:                             // Totemic Mastery (Skyshatter Regalia (Shaman Tier 6) - bonus)
                    {
                        if (triggerTarget->IsAllTotemSlotsUsed())
                            triggerTarget->CastSpell(triggerTarget, 38437, true, NULL, this);
                        else
                            triggerTarget->RemoveAurasDueToSpell(38437);
                        return;
                    }
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }

        // Reget trigger spell proto
        triggeredSpellInfo = sSpellStore.LookupEntry(trigger_spell_id);
    }
    else                                                    // initial triggeredSpellInfo != NULL
    {
        // for channeled spell cast applied from aura owner to channel target (persistent aura affects already applied to true target)
        // come periodic casts applied to targets, so need seelct proper caster (ex. 15790)
        if (IsChanneledSpell(GetSpellProto()) && GetSpellProto()->Effect[GetEffIndex()] != SPELL_EFFECT_PERSISTENT_AREA_AURA)
        {
            // interesting 2 cases: periodic aura at caster of channeled spell
            if (target->GetObjectGuid() == casterGUID)
            {
                triggerCaster = target;

                if (WorldObject* channelTarget = target->GetMap()->GetWorldObject(target->GetChannelObjectGuid()))
                {
                    if (channelTarget->isType(TYPEMASK_UNIT))
                        triggerTarget = (Unit*)channelTarget;
                    else
                        triggerTargetObject = channelTarget;
                }
            }
            // or periodic aura at caster channel target
            else if (Unit* caster = GetCaster())
            {
                if (target->GetObjectGuid() == caster->GetChannelObjectGuid())
                {
                    triggerCaster = caster;
                    triggerTarget = target;
                }
            }
        }

        // Spell exist but require custom code
        switch(auraId)
        {
            case 9347:                                      // Mortal Strike
            {
                if (target->GetTypeId() != TYPEID_UNIT)
                    return;
                // expected selection current fight target
                triggerTarget = ((Creature*)target)->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, triggeredSpellInfo);
                if (!triggerTarget)
                    return;

                break;
            }
            case 1010:                                      // Curse of Idiocy
            {
                // TODO: spell casted by result in correct way mostly
                // BUT:
                // 1) target show casting at each triggered cast: target don't must show casting animation for any triggered spell
                //      but must show affect apply like item casting
                // 2) maybe aura must be replace by new with accumulative stat mods instead stacking

                // prevent cast by triggered auras
                if (casterGUID == triggerTarget->GetObjectGuid())
                    return;

                // stop triggering after each affected stats lost > 90
                int32 intelectLoss = 0;
                int32 spiritLoss = 0;

                Unit::AuraList const& mModStat = triggerTarget->GetAurasByType(SPELL_AURA_MOD_STAT);
                for(Unit::AuraList::const_iterator i = mModStat.begin(); i != mModStat.end(); ++i)
                {
                    if ((*i)->GetId() == 1010)
                    {
                        switch((*i)->GetModifier()->m_miscvalue)
                        {
                            case STAT_INTELLECT: intelectLoss += (*i)->GetModifier()->m_amount; break;
                            case STAT_SPIRIT:    spiritLoss   += (*i)->GetModifier()->m_amount; break;
                            default: break;
                        }
                    }
                }

                if (intelectLoss <= -90 && spiritLoss <= -90)
                    return;

                break;
            }
            // Earthen Power (from Earthbind Totem Passive)
            case 6474:
            {
                Unit *owner = target->GetOwner();

                if (!owner)
                    break;

                Unit::AuraList const& dummyAuras = owner->GetAurasByType(SPELL_AURA_DUMMY);
                for(Unit::AuraList::const_iterator itr = dummyAuras.begin(); itr != dummyAuras.end(); ++itr)
                {
                    if ((*itr)->GetSpellProto()->SpellIconID == 2289 && (*itr)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_SHAMAN)
                    {
                        if (!roll_chance_i((*itr)->GetModifier()->m_amount))
                            break;

                        target->CastSpell(target, 59566, true, NULL, this);
                        break;
                    }
                 }
                break;
            }
            case 16191:                                     // Mana Tide
            {
                triggerTarget->CastCustomSpell(triggerTarget, trigger_spell_id, &m_modifier.m_amount, NULL, NULL, true, NULL, this);
                return;
            }
            case 28084:                                     // Negative Charge
            {
                if (triggerTarget->HasAura(29660))
                    triggerTarget->RemoveAurasDueToSpell(29660);
                break;
            }
            case 28059:                                     // Positive Charge
            {
                if (triggerTarget->HasAura(29659))
                    triggerTarget->RemoveAurasDueToSpell(29659);
                break;
            }
            case 33525:                                     // Ground Slam
                triggerTarget->CastSpell(triggerTarget, trigger_spell_id, true, NULL, this, casterGUID);
                return;
            case 38280:                                     // Static Charge (Lady Vashj in Serpentshrine Cavern)
            case 53563:                                     // Beacon of Light
            case 52658:                                     // Static Overload (normal&heroic) (Ionar in Halls of Lightning)
            case 59795:
            case 63018:                                     // Searing Light (normal&heroic) (XT-002 in Ulduar)
            case 65121:
            case 63024:                                     // Gravity Bomb (normal&heroic) (XT-002 in Ulduar)
            case 64234:
                // original caster must be target
                target->CastSpell(target, trigger_spell_id, true, NULL, this, target->GetObjectGuid());
                return;
            case 38736:                                     // Rod of Purification - for quest 10839 (Veil Skith: Darkstone of Terokk)
            {
                if (Unit* caster = GetCaster())
                    caster->CastSpell(triggerTarget, trigger_spell_id, true, NULL, this);
                return;
            }
            case 44883:                                     // Encapsulate
            {
                // Self cast spell, hence overwrite caster (only channeled spell where the triggered spell deals dmg to SELF)
                triggerCaster = triggerTarget;
                break;
            }
            case 48094:                                      // Intense Cold
                triggerTarget->CastSpell(triggerTarget, trigger_spell_id, true, NULL, this);
                return;
            case 58678:                                     // Rock Shards (Vault of Archavon, Archavon)
            {
                if (GetAuraTicks() != 1 && GetAuraTicks()%7)
                    return;
                break;
            }
            case 56654:                                     // Rapid Recuperation (triggered energize have baspioints == 0)
            case 58882:
            {
                int32 mana = target->GetMaxPower(POWER_MANA) * m_modifier.m_amount / 100;
                triggerTarget->CastCustomSpell(triggerTarget, trigger_spell_id, &mana, NULL, NULL, true, NULL, this);
                return;
            }
        }
    }

    // All ok cast by default case
    if (triggeredSpellInfo)
    {
        if (triggerTargetObject)
            triggerCaster->CastSpell(triggerTargetObject->GetPositionX(), triggerTargetObject->GetPositionY(), triggerTargetObject->GetPositionZ(),
                triggeredSpellInfo, true, NULL, this, casterGUID);
        else
            triggerCaster->CastSpell(triggerTarget, triggeredSpellInfo, true, NULL, this, casterGUID);
    }
    else
    {
        if (Unit* caster = GetCaster())
        {
            if (triggerTarget->GetTypeId() != TYPEID_UNIT || !sScriptMgr.OnEffectDummy(caster, GetId(), GetEffIndex(), (Creature*)triggerTarget))
                sLog.outError("Aura::TriggerSpell: Spell %u have 0 in EffectTriggered[%d], not handled custom case?",GetId(),GetEffIndex());
        }
    }
}

void Aura::TriggerSpellWithValue()
{
    ObjectGuid casterGuid = GetCasterGuid();
    Unit* target = GetTriggerTarget();

    if (!casterGuid || !target)
        return;

    // generic casting code with custom spells and target/caster customs
    uint32 trigger_spell_id = GetSpellProto()->EffectTriggerSpell[m_effIndex];
    int32  basepoints0 = GetModifier()->m_amount;

    target->CastCustomSpell(target, trigger_spell_id, &basepoints0, NULL, NULL, true, NULL, this, casterGuid);
}

/*********************************************************/
/***                  AURA EFFECTS                     ***/
/*********************************************************/

void Aura::HandleAuraDummy(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    Unit *target = GetTarget();

    // AT APPLY
    if (apply)
    {
        switch(GetSpellProto()->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
            {
                switch(GetId())
                {
                    case 1515:                              // Tame beast
                        // FIX_ME: this is 2.0.12 threat effect replaced in 2.1.x by dummy aura, must be checked for correctness
                        if (target->CanHaveThreatList())
                            if (Unit* caster = GetCaster())
                                target->AddThreat(caster, 10.0f, false, GetSpellSchoolMask(GetSpellProto()), GetSpellProto());
                        return;
                    case 7057:                              // Haunting Spirits
                        // expected to tick with 30 sec period (tick part see in Aura::PeriodicTick)
                        m_isPeriodic = true;
                        m_modifier.periodictime = 30*IN_MILLISECONDS;
                        m_periodicTimer = m_modifier.periodictime;
                        return;
                    case 10255:                             // Stoned
                    {
                        if (Unit* caster = GetCaster())
                        {
                            if (caster->GetTypeId() != TYPEID_UNIT)
                                return;

                            caster->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            caster->addUnitState(UNIT_STAT_ROOT);
                        }
                        return;
                    }
                    case 13139:                             // net-o-matic
                        // root to self part of (root_target->charge->root_self sequence
                        if (Unit* caster = GetCaster())
                            caster->CastSpell(caster, 13138, true, NULL, this);
                        return;
                    case 28832:                             // Mark of Korth'azz
                    case 28833:                             // Mark of Blaumeux
                    case 28834:                             // Mark of Rivendare
                    case 28835:                             // Mark of Zeliek
                    {
                        int32 damage = 0;

                        switch (GetStackAmount())
                        {
                            case 1:
                                return;
                            case 2: damage =   500; break;
                            case 3: damage =  1500; break;
                            case 4: damage =  4000; break;
                            case 5: damage = 12500; break;
                            default:
                                damage = 14000 + 1000 * GetStackAmount();
                                break;
                        }

                        if (Unit* caster = GetCaster())
                            caster->CastCustomSpell(target, 28836, &damage, NULL, NULL, true, NULL, this);
                        return;
                    }
                    case 31606:                             // Stormcrow Amulet
                    {
                        CreatureInfo const * cInfo = ObjectMgr::GetCreatureTemplate(17970);

                        // we must assume db or script set display id to native at ending flight (if not, target is stuck with this model)
                        if (cInfo)
                            target->SetDisplayId(Creature::ChooseDisplayId(cInfo));

                        return;
                    }
                    case 32045:                             // Soul Charge
                    case 32051:
                    case 32052:
                    {
                        // max duration is 2 minutes, but expected to be random duration
                        // real time randomness is unclear, using max 30 seconds here
                        // see further down for expire of this aura
                        GetHolder()->SetAuraDuration(urand(1, 30)*IN_MILLISECONDS);
                        return;
                    }
                    case 33326:                             // Stolen Soul Dispel
                    {
                        target->RemoveAurasDueToSpell(32346);
                        return;
                    }
                }
                break;
            }
            case SPELLFAMILY_WARRIOR:
            {
                switch(GetId())
                {
                    case 41099:                             // Battle Stance
                    {
                        if (target->GetTypeId() != TYPEID_UNIT)
                            return;

                        // Stance Cooldown
                        target->CastSpell(target, 41102, true, NULL, this);

                        // Battle Aura
                        target->CastSpell(target, 41106, true, NULL, this);

                        // equipment
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_0, 32614);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_1, 0);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_2, 0);
                        return;
                    }
                    case 41100:                             // Berserker Stance
                    {
                        if (target->GetTypeId() != TYPEID_UNIT)
                            return;

                        // Stance Cooldown
                        target->CastSpell(target, 41102, true, NULL, this);

                        // Berserker Aura
                        target->CastSpell(target, 41107, true, NULL, this);

                        // equipment
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_0, 32614);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_1, 0);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_2, 0);
                        return;
                    }
                    case 41101:                             // Defensive Stance
                    {
                        if (target->GetTypeId() != TYPEID_UNIT)
                            return;

                        // Stance Cooldown
                        target->CastSpell(target, 41102, true, NULL, this);

                        // Defensive Aura
                        target->CastSpell(target, 41105, true, NULL, this);

                        // equipment
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_0, 32604);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_1, 31467);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_2, 0);
                        return;
                    }
                    case 53790:                             // Defensive Stance
                    {
                        if (target->GetTypeId() != TYPEID_UNIT)
                            return;

                        // Stance Cooldown
                        target->CastSpell(target, 59526, true, NULL, this);

                        // Defensive Aura
                        target->CastSpell(target, 41105, true, NULL, this);

                        // equipment
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_0, 43625);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_1, 39384);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_2, 0);
                        return;
                    }
                    case 53791:                             // Berserker Stance
                    {
                        if (target->GetTypeId() != TYPEID_UNIT)
                            return;

                        // Stance Cooldown
                        target->CastSpell(target, 59526, true, NULL, this);

                        // Berserker Aura
                        target->CastSpell(target, 41107, true, NULL, this);

                        // equipment
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_0, 43625);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_1, 43625);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_2, 0);
                        return;
                    }
                    case 53792:                             // Battle Stance
                    {
                        if (target->GetTypeId() != TYPEID_UNIT)
                            return;

                        // Stance Cooldown
                        target->CastSpell(target, 59526, true, NULL, this);

                        // Battle Aura
                        target->CastSpell(target, 41106, true, NULL, this);

                        // equipment
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_0, 43623);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_1, 0);
                        ((Creature*)target)->SetVirtualItem(VIRTUAL_ITEM_SLOT_2, 0);
                        return;
                    }
                }

                // Overpower
                if (GetSpellProto()->GetSpellFamilyFlags().test<CF_WARRIOR_OVERPOWER>())
                {
                    // Must be casting target
                    if (!target->IsNonMeleeSpellCasted(false) || !target->GetObjectGuid().IsPlayerOrPet()) //unrelenting assault don't must affect on pve.
                        return;

                    Unit* caster = GetCaster();
                    if (!caster)
                        return;

                    Unit::AuraList const& modifierAuras = caster->GetAurasByType(SPELL_AURA_ADD_FLAT_MODIFIER);
                    for(Unit::AuraList::const_iterator itr = modifierAuras.begin(); itr != modifierAuras.end(); ++itr)
                    {
                        // Unrelenting Assault
                        if ((*itr)->GetSpellProto()->SpellFamilyName==SPELLFAMILY_WARRIOR && (*itr)->GetSpellProto()->SpellIconID == 2775)
                        {
                            switch ((*itr)->GetSpellProto()->Id)
                            {
                                case 46859:                 // Unrelenting Assault, rank 1
                                    target->CastSpell(target,64849,true,NULL,(*itr)());
                                    break;
                                case 46860:                 // Unrelenting Assault, rank 2
                                    target->CastSpell(target,64850,true,NULL,(*itr)());
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                    }
                    return;
                }
                break;
            }
            case SPELLFAMILY_ROGUE:
            {
                // Honor Among Thieves
                if (GetId() == 52916)
                {
                    // Get Honor Among Thieves party aura
                    Unit::AuraList const &procTriggerSpellAuras = target->GetAurasByType(SPELL_AURA_PROC_TRIGGER_SPELL);
                    for (Unit::AuraList::const_iterator i = procTriggerSpellAuras.begin(); i != procTriggerSpellAuras.end(); ++i)
                    {
                        SpellEntry const *spellInfo = (*i)->GetSpellProto();

                        if (!spellInfo)
                            continue;

                        if (spellInfo->EffectTriggerSpell[0] == 52916)
                        {
                            // Get caster of aura
                            if(!(*i)->GetCaster() || (*i)->GetCaster()->GetTypeId() != TYPEID_PLAYER)
                                continue;

                            Player *pCaster = (Player*)((*i)->GetCaster());

                            // do not proc if player has CD, or if player has no target, or if player's target is not valid
                            if (pCaster->HasAura(51699, EFFECT_INDEX_1) || !pCaster->getVictim() || pCaster->IsFriendlyTo(pCaster->getVictim()))
                                continue;
                            // give combo point and aura for cooldown on success
                            else if (roll_chance_i(spellInfo->CalculateSimpleValue(EFFECT_INDEX_0)))
                                pCaster->CastSpell(pCaster->getVictim(), 51699, true);
                        }
                    }

                    // return after loop to make sure all rogues with Honor Among Thieves get the benefit of this proc rather than only first
                    return;
                }
                break;
            }
            case SPELLFAMILY_HUNTER:
            {
                switch(GetId())
                {
                    case 34026:                             // Kill Command
                        target->CastSpell(target, 34027, true, NULL, this);
                        return;
                }
                break;
            }
            case SPELLFAMILY_SHAMAN:
            {
                switch(GetId())
                {
                    case 55198:                             // Tidal Force
                        target->CastSpell(target, 55166, true, NULL, this);
                        return;
                }

                // Earth Shield
                if (GetSpellProto()->GetSpellFamilyFlags().test<CF_SHAMAN_EARTH_SHIELD>())
                {
                    // prevent double apply bonuses
                    if (target->GetTypeId() != TYPEID_PLAYER || !((Player*)target)->GetSession()->PlayerLoading())
                    {
                        if (Unit* caster = GetCaster())
                        {
                            m_modifier.m_amount = caster->SpellHealingBonusDone(target, GetSpellProto(), m_modifier.m_amount, SPELL_DIRECT_DAMAGE);
                            m_modifier.m_amount = target->SpellHealingBonusTaken(caster, GetSpellProto(), m_modifier.m_amount, SPELL_DIRECT_DAMAGE);
                        }
                    }
                    return;
                }
                break;
            }
        }
    }
    // AT REMOVE
    else
    {
        if (IsQuestTameSpell(GetId()) && target->isAlive())
        {
            Unit* caster = GetCaster();
            if (!caster || !caster->isAlive())
                return;

            uint32 finalSpellId = 0;
            switch(GetId())
            {
                case 19548: finalSpellId = 19597; break;
                case 19674: finalSpellId = 19677; break;
                case 19687: finalSpellId = 19676; break;
                case 19688: finalSpellId = 19678; break;
                case 19689: finalSpellId = 19679; break;
                case 19692: finalSpellId = 19680; break;
                case 19693: finalSpellId = 19684; break;
                case 19694: finalSpellId = 19681; break;
                case 19696: finalSpellId = 19682; break;
                case 19697: finalSpellId = 19683; break;
                case 19699: finalSpellId = 19685; break;
                case 19700: finalSpellId = 19686; break;
                case 30646: finalSpellId = 30647; break;
                case 30653: finalSpellId = 30648; break;
                case 30654: finalSpellId = 30652; break;
                case 30099: finalSpellId = 30100; break;
                case 30102: finalSpellId = 30103; break;
                case 30105: finalSpellId = 30104; break;
            }

            if (finalSpellId)
                caster->CastSpell(target, finalSpellId, true, NULL, this);
            return;
        }

        switch(GetId())
        {
            case 10255:                                     // Stoned
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->GetTypeId() != TYPEID_UNIT)
                        return;

                    // see dummy effect of spell 10254 for removal of flags etc
                    caster->CastSpell(caster, 10254, true);
                }
                return;
            }
            case 12479:                                     // Hex of Jammal'an
                target->CastSpell(target, 12480, true, NULL, this);
                return;
            case 12774:                                     // (DND) Belnistrasz Idol Shutdown Visual
            {
                if (m_removeMode == AURA_REMOVE_BY_DEATH)
                    return;

                // Idom Rool Camera Shake <- wtf, don't drink while making spellnames?
                if (Unit* caster = GetCaster())
                    caster->CastSpell(caster, 12816, true);

                return;
            }
            case 28059:                                     // Positive Charge (Thaddius)
            {
                if (target->HasAura(29659))
                    target->RemoveAurasDueToSpell(29659);
                return;
            }
            case 28084:                                     // Negative Charge (Thaddius)
            {
                if (target->HasAura(29660))
                    target->RemoveAurasDueToSpell(29660);
                return;
            }
            case 28169:                                     // Mutating Injection
            {
                // Mutagen Explosion
                target->CastSpell(target, 28206, true, NULL, this);
                // Poison Cloud
                target->CastSpell(target, 28240, true, NULL, this);
                return;
            }
            case 32045:                                     // Soul Charge
            {
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE)
                    target->CastSpell(target, 32054, true, NULL, this);
                return;
            }
            case 32051:                                     // Soul Charge
            {
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE)
                    target->CastSpell(target, 32057, true, NULL, this);
                return;
            }
            case 32052:                                     // Soul Charge
            {
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE)
                    target->CastSpell(target, 32053, true, NULL, this);
                return;
            }
            case 32286:                                     // Focus Target Visual
            {
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE)
                    target->CastSpell(target, 32301, true, NULL, this);
                return;
            }
        }

        // Living Bomb
        if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_MAGE && GetSpellProto()->GetSpellFamilyFlags().test<CF_MAGE_LIVING_BOMB>())
        {
            if (m_removeMode == AURA_REMOVE_BY_EXPIRE || m_removeMode == AURA_REMOVE_BY_DISPEL)
            {
                if (this->GetCaster())
                    GetCaster()->CastSpell(target,m_modifier.m_amount,true,NULL,this);
                else
                    target->CastSpell(target,m_modifier.m_amount,true,NULL,this);
            }
            return;
        }
    }

    // AT APPLY & REMOVE

    switch(GetSpellProto()->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch(GetId())
            {
                case 6606:                                  // Self Visual - Sleep Until Cancelled (DND)
                {
                    if (apply)
                    {
                        target->SetStandState(UNIT_STAND_STATE_SLEEP);
                        target->addUnitState(UNIT_STAT_ROOT);
                    }
                    else
                    {
                        target->clearUnitState(UNIT_STAT_ROOT);
                        target->SetStandState(UNIT_STAND_STATE_STAND);
                    }

                    return;
                }
                case 11196:                                 // Recently Bandaged
                    target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, GetMiscValue(), apply);
                    return;
                case 24658:                                 // Unstable Power
                {
                    if (apply)
                    {
                        Unit* caster = GetCaster();
                        if (!caster)
                            return;

                        caster->CastSpell(target, 24659, true, NULL, NULL, GetCasterGuid());
                    }
                    else
                        target->RemoveAurasDueToSpell(24659);
                    return;
                }
                case 24661:                                 // Restless Strength
                {
                    if (apply)
                    {
                        Unit* caster = GetCaster();
                        if (!caster)
                            return;

                        caster->CastSpell(target, 24662, true, NULL, NULL, GetCasterGuid());
                    }
                    else
                        target->RemoveAurasDueToSpell(24662);
                    return;
                }
                case 29266:                                 // Permanent Feign Death
                case 31261:                                 // Permanent Feign Death (Root)
                case 37493:                                 // Feign Death
                case 52593:                                 // Bloated Abomination Feign Death
                case 55795:                                 // Falling Dragon Feign Death
                case 57626:                                 // Feign Death
                case 57685:                                 // Permanent Feign Death
                case 58768:                                 // Permanent Feign Death (Freeze Jumpend)
                case 58806:                                 // Permanent Feign Death (Drowned Anim)
                case 58951:                                 // Permanent Feign Death
                case 64461:                                 // Permanent Feign Death (No Anim) (Root)
                case 65985:                                 // Permanent Feign Death (Root Silence Pacify)
                case 70592:                                 // Permanent Feign Death
                case 70628:                                 // Permanent Feign Death
                case 70630:                                 // Frozen Aftermath - Feign Death
                case 71598:                                 // Feign Death
                {
                    // Unclear what the difference really is between them.
                    // Some has effect1 that makes the difference, however not all.
                    // Some appear to be used depending on creature location, in water, at solid ground, in air/suspended, etc
                    // For now, just handle all the same way
                    if (target->GetTypeId() == TYPEID_UNIT)
                        target->SetFeignDeath(apply);

                    return;
                }
                case 27978:
                case 40131:
                    if (apply)
                        target->m_AuraFlags |= UNIT_AURAFLAG_ALIVE_INVISIBLE;
                    else
                        target->m_AuraFlags |= ~UNIT_AURAFLAG_ALIVE_INVISIBLE;
                    return;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
            break;
        case SPELLFAMILY_WARLOCK:
        {
            // Haunt
            if (GetSpellProto()->SpellIconID == 3172 && GetSpellProto()->GetSpellFamilyFlags().test<CF_WARLOCK_HAUNT>())
            {
                // NOTE: for avoid use additional field damage stored in dummy value (replace unused 100%
                if (apply)
                    m_modifier.m_amount = 0;                // use value as damage counter instead redundant 100% percent
                else
                {
                    int32 bp0 = m_modifier.m_amount;

                    if (Unit* caster = GetCaster())
                        target->CastCustomSpell(caster, 48210, &bp0, NULL, NULL, true, NULL, this);
                }
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            switch(GetId())
            {
                case 52610:                                 // Savage Roar
                {
                    if (apply)
                    {
                        if (target->GetShapeshiftForm() != FORM_CAT)
                            return;

                        target->CastSpell(target, 62071, true);
                    }
                    else
                        target->RemoveAurasDueToSpell(62071);
                    return;
                }
                case 61336:                                 // Survival Instincts
                {
                    if (apply)
                    {
                        if (!target->IsInFeralForm())
                            return;

                        int32 bp0 = int32(target->GetMaxHealth() * m_modifier.m_amount / 100);
                        target->CastCustomSpell(target, 50322, &bp0, NULL, NULL, true);
                    }
                    else
                        target->RemoveAurasDueToSpell(50322);
                    return;
                }
            }

            // Lifebloom
            if (GetSpellProto()->GetSpellFamilyFlags().test<CF_DRUID_LIFEBLOOM>())
            {
                if (apply)
                {
                    if (Unit* caster = GetCaster())
                    {
                        // prevent double apply bonuses
                        if (target->GetTypeId() != TYPEID_PLAYER || !((Player*)target)->GetSession()->PlayerLoading())
                        {
                            m_modifier.m_amount = caster->SpellHealingBonusDone(target, GetSpellProto(), m_modifier.m_amount, SPELL_DIRECT_DAMAGE, GetStackAmount());
                            m_modifier.m_amount = target->SpellHealingBonusTaken(caster, GetSpellProto(), m_modifier.m_amount, SPELL_DIRECT_DAMAGE, GetStackAmount());
                        }
                    }
                }
                else
                {
                    // Final heal on duration end
                    if (m_removeMode != AURA_REMOVE_BY_EXPIRE)
                        return;

                    // final heal
                    if (target->IsInWorld() && GetStackAmount() > 0)
                    {
                        int32 amount = m_modifier.m_amount;
                        target->CastCustomSpell(target, 33778, &amount, NULL, NULL, true, NULL, this, GetCasterGuid());

                        if (Unit* caster = GetCaster())
                        {
                            int32 returnmana = (GetSpellProto()->ManaCostPercentage * caster->GetCreateMana() / 100) * GetStackAmount() / 2;
                            caster->CastCustomSpell(caster, 64372, &returnmana, NULL, NULL, true, NULL, this, GetCasterGuid());
                        }
                    }
                }
                return;
            }

            // Predatory Strikes
            if (target->GetTypeId()==TYPEID_PLAYER && GetSpellProto()->SpellIconID == 1563)
            {
                ((Player*)target)->UpdateAttackPowerAndDamage();
                return;
            }

            // Improved Moonkin Form
            if (GetSpellProto()->SpellIconID == 2855)
            {
                uint32 spell_id;
                switch(GetId())
                {
                    case 48384: spell_id = 50170; break;    //Rank 1
                    case 48395: spell_id = 50171; break;    //Rank 2
                    case 48396: spell_id = 50172; break;    //Rank 3
                    default:
                        sLog.outError("HandleAuraDummy: Not handled rank of IMF (Spell: %u)",GetId());
                        return;
                }

                if (apply)
                {
                    if (target->GetShapeshiftForm() != FORM_MOONKIN)
                        return;

                    target->CastSpell(target, spell_id, true);
                }
                else
                    target->RemoveAurasDueToSpell(spell_id);
                return;
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
            switch(GetId())
            {
                case 57934:                                 // Tricks of the Trade, main spell
                {
                    if (apply)
                        GetHolder()->SetAuraCharges(1);     // not have proper charges set in spell data
                    else
                    {
                        // used for direct in code aura removes and spell proc event charges expire
                        if (m_removeMode != AURA_REMOVE_BY_DEFAULT)
                            target->getHostileRefManager().ResetThreatRedirection();
                    }
                    return;
                }
            }
            break;
        case SPELLFAMILY_HUNTER:
        {
            switch(GetId())
            {
                case 34477:                                 // Misdirection, main spell
                {
                    if (apply)
                        GetHolder()->SetAuraCharges(1);     // not have proper charges set in spell data
                    else
                    {
                        // used for direct in code aura removes and spell proc event charges expire
                        if (m_removeMode != AURA_REMOVE_BY_DEFAULT)
                            target->getHostileRefManager().ResetThreatRedirection();
                    }
                    return;
                }
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
            switch(GetId())
            {
                case 20911:                                 // Blessing of Sanctuary
                case 25899:                                 // Greater Blessing of Sanctuary
                {
                    if (apply)
                        target->CastSpell(target, 67480, true, NULL, this);
                    else
                        target->RemoveAurasDueToSpell(67480);
                    return;
                }
            }
            break;
        case SPELLFAMILY_SHAMAN:
        {
            switch(GetId())
            {
                case 6495:                                  // Sentry Totem
                {
                    if (target->GetTypeId() != TYPEID_PLAYER)
                        return;

                    Totem* totem = target->GetTotem(TOTEM_SLOT_AIR);

                    if (totem && apply)
                        ((Player*)target)->SetViewPoint(totem);
                    else
                        ((Player*)target)->SetViewPoint(NULL);

                    return;
                }
            }
            break;
        }
    }

    if (GetEffIndex() == EFFECT_INDEX_0 && target->GetTypeId() == TYPEID_PLAYER)
    {
        SpellAreaForAreaMapBounds saBounds = sSpellMgr.GetSpellAreaForAuraMapBounds(GetId());
        if (saBounds.first != saBounds.second)
        {
            uint32 zone, area;
            target->GetZoneAndAreaId(zone, area);

            for(SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
            {
                // some auras remove at aura remove
                if (!itr->second->IsFitToRequirements((Player*)target, zone, area))
                    target->RemoveAurasDueToSpell(itr->second->spellId);
                // some auras applied at aura apply
                else if (itr->second->autocast)
                {
                    if (!target->HasAura(itr->second->spellId, EFFECT_INDEX_0))
                        target->CastSpell(target, itr->second->spellId, true);
                }
            }
        }
    }

    // script has to "handle with care", only use where data are not ok to use in the above code.
    if (target->GetTypeId() == TYPEID_UNIT)
        sScriptMgr.OnAuraDummy(this, apply);
}

void Aura::HandleAuraMounted(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    Unit *target = GetTarget();

    if (apply)
    {
        CreatureInfo const* ci = ObjectMgr::GetCreatureTemplate(m_modifier.m_miscvalue);
        if(!ci)
        {
            sLog.outErrorDb("AuraMounted: `creature_template`='%u' not found in database (only need it modelid)", m_modifier.m_miscvalue);
            return;
        }

        uint32 display_id = Creature::ChooseDisplayId(ci);
        CreatureModelInfo const *minfo = sObjectMgr.GetCreatureModelRandomGender(display_id);
        if (minfo)
            display_id = minfo->modelid;

        target->Mount(display_id, GetId(), GetMiscValue());
    }
    else
    {
        target->Unmount(true);
    }
}

void Aura::HandleAuraWaterWalk(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
    if (!apply && GetTarget()->HasAuraType(SPELL_AURA_WATER_WALK))
        return;

    GetTarget()->SetWaterWalk(apply);
}

void Aura::HandleAuraFeatherFall(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    Unit *target = GetTarget();

    // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
    if (!apply && target->HasAuraType(SPELL_AURA_FEATHER_FALL))
        return;

    WorldPacket data;
    if (apply)
        data.Initialize(SMSG_MOVE_FEATHER_FALL, 8+4);
    else
        data.Initialize(SMSG_MOVE_NORMAL_FALL, 8+4);
    data << target->GetPackGUID();
    data << uint32(0);
    target->SendMessageToSet(&data, true);

    // start fall from current height
    if(!apply && target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)target)->SetFallInformation(0, target->GetPositionZ());

    // additional custom cases
    if(!apply)
    {
        switch(GetId())
        {
            // Soaring - Test Flight chain
            case 36812:
            case 37910:
            case 37940:
            case 37962:
            case 37968:
            {
                if (Unit* pCaster = GetCaster())
                    pCaster->CastSpell(pCaster, 37108, true);
                return;
            }
        }
    }
}

void Aura::HandleAuraHover(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
    if (!apply && GetTarget()->HasAuraType(SPELL_AURA_HOVER))
        return;

    WorldPacket data;
    if (apply)
    {
        GetTarget()->m_movementInfo.AddMovementFlag(MOVEFLAG_HOVER);
        data.Initialize(GetTarget()->GetTypeId() == TYPEID_PLAYER ? SMSG_MOVE_SET_HOVER : SMSG_SPLINE_MOVE_SET_HOVER, 8+4);
        data << GetTarget()->GetPackGUID();
        if (GetTarget()->GetTypeId() == TYPEID_PLAYER)
        {
            data << uint32(0);
        }
        else
            GetTarget()->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
    }
    else
    {
        GetTarget()->m_movementInfo.RemoveMovementFlag(MOVEFLAG_HOVER);
        data.Initialize(GetTarget()->GetTypeId() == TYPEID_PLAYER ? SMSG_MOVE_UNSET_HOVER : SMSG_SPLINE_MOVE_UNSET_HOVER, 8+4);
        data << GetTarget()->GetPackGUID();
        if (GetTarget()->GetTypeId() == TYPEID_PLAYER)
        {
            data << uint32(0);
        }
        else
            GetTarget()->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
    }
    GetTarget()->SendMessageToSet(&data, true);
}

void Aura::HandleWaterBreathing(bool /*apply*/, bool /*Real*/)
{
    // update timers in client
    if (GetTarget()->GetTypeId()==TYPEID_PLAYER)
        ((Player*)GetTarget())->UpdateMirrorTimers();
}

void Aura::HandleAuraModShapeshift(bool apply, bool Real)
{
    if (!Real)
        return;

    ShapeshiftForm form = ShapeshiftForm(m_modifier.m_miscvalue);

    SpellShapeshiftFormEntry const* ssEntry = sSpellShapeshiftFormStore.LookupEntry(form);
    if (!ssEntry)
    {
        sLog.outError("Unknown shapeshift form %u in spell %u", form, GetId());
        return;
    }

    uint32 modelid = 0;
    Powers PowerType = POWER_MANA;
    Unit* target = GetTarget();

    switch(form)
    {
        case FORM_CAT:
            if(Player::TeamForRace(target->getRace()) == ALLIANCE)
                modelid = 892;
            else
                modelid = 8571;
            PowerType = POWER_ENERGY;
            break;
        case FORM_TRAVEL:
            modelid = 632;
            break;
        case FORM_AQUA:
            if(Player::TeamForRace(target->getRace()) == ALLIANCE)
                modelid = 2428;
            else
                modelid = 2428;
            break;
        case FORM_BEAR:
            if(Player::TeamForRace(target->getRace()) == ALLIANCE)
                modelid = 2281;
            else
                modelid = 2289;
            PowerType = POWER_RAGE;
            break;
        case FORM_GHOUL:
            if(Player::TeamForRace(target->getRace()) == ALLIANCE)
                modelid = 10045;
            break;
        case FORM_DIREBEAR:
            if(Player::TeamForRace(target->getRace()) == ALLIANCE)
                modelid = 2281;
            else
                modelid = 2289;
            PowerType = POWER_RAGE;
            break;
        case FORM_CREATUREBEAR:
            modelid = 902;
            break;
        case FORM_GHOSTWOLF:
            modelid = 4613;
            break;
        case FORM_MOONKIN:
            if(Player::TeamForRace(target->getRace()) == ALLIANCE)
                modelid = 15374;
            else
                modelid = 15375;
            break;
        case FORM_AMBIENT:
        case FORM_SHADOW:
        case FORM_STEALTH:
            break;
        case FORM_TREE:
            modelid = 864;
            break;
        case FORM_BATTLESTANCE:
        case FORM_BERSERKERSTANCE:
        case FORM_DEFENSIVESTANCE:
            PowerType = POWER_RAGE;
            break;
        case FORM_SPIRITOFREDEMPTION:
            modelid = 16031;
            break;
        default:
            break;
    }

    // remove polymorph before changing display id to keep new display id
    switch (form)
    {
        case FORM_CAT:
        case FORM_TREE:
        case FORM_TRAVEL:
        case FORM_AQUA:
        case FORM_BEAR:
        case FORM_DIREBEAR:
        case FORM_MOONKIN:
        {
            // remove movement affects
            target->RemoveSpellsCausingAura(SPELL_AURA_MOD_ROOT, GetHolder());
            std::set<uint32> toRemoveSpellList;
            Unit::AuraList const& slowingAuras = target->GetAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
            for (Unit::AuraList::const_iterator iter = slowingAuras.begin(); iter != slowingAuras.end(); ++iter)
            {
                if (iter->IsEmpty())
                    continue;

                SpellAuraHolderPtr holder = iter->GetHolder();
                if (!holder || holder->IsDeleted())
                    continue;

                SpellEntry const* aurSpellInfo = holder->GetSpellProto();

                uint32 aurMechMask = GetAllSpellMechanicMask(aurSpellInfo);

                // If spell that caused this aura has Croud Control or Daze effect
                if (((aurMechMask & MECHANIC_NOT_REMOVED_BY_SHAPESHIFT) &&
                    // some non-Daze spells that have MECHANIC_DAZE
                    aurSpellInfo->Id != 18118 &&    // Aftermath
                    !aurSpellInfo->IsFitToFamily<SPELLFAMILY_PALADIN, CF_PALADIN_AVENGERS_SHIELD>()) ||
                    // some Daze spells have these parameters instead of MECHANIC_DAZE (skip snare spells)
                    (aurSpellInfo->SpellIconID == 15 && aurSpellInfo->Dispel == 0 &&
                    (aurMechMask & (1 << (MECHANIC_SNARE-1))) == 0))
                {
                    continue;
                }

                // All OK, remove aura now
                toRemoveSpellList.insert(aurSpellInfo->Id);
            }

            for (std::set<uint32>::iterator i = toRemoveSpellList.begin(); i != toRemoveSpellList.end(); ++i)
                target->RemoveAurasDueToSpellByCancel(*i);

            // and polymorphic affects
            if (target->IsPolymorphed())
                target->RemoveAurasDueToSpell(target->getTransForm());

            break;
        }
        default:
           break;
    }

    if (apply)
    {
        // remove other shapeshift before applying a new one
        target->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT, GetHolder());

        if (modelid > 0)
            target->SetDisplayId(modelid);

        // now only powertype must be set
        switch (form)
        {
            case FORM_CAT:
                PowerType = POWER_ENERGY;
                break;
            case FORM_BEAR:
            case FORM_DIREBEAR:
            case FORM_BATTLESTANCE:
            case FORM_BERSERKERSTANCE:
            case FORM_DEFENSIVESTANCE:
                PowerType = POWER_RAGE;
                break;
            default:
                break;
        }

        if (PowerType != POWER_MANA)
        {
            // reset power to default values only at power change
            if (target->getPowerType() != PowerType)
                target->setPowerType(PowerType);

            switch (form)
            {
                case FORM_CAT:
                case FORM_BEAR:
                case FORM_DIREBEAR:
                {
                    // get furor proc chance
                    int32 furorChance = 0;
                    Unit::AuraList const& mDummy = target->GetAurasByType(SPELL_AURA_DUMMY);
                    for (Unit::AuraList::const_iterator i = mDummy.begin(); i != mDummy.end(); ++i)
                    {
                        if ((*i)->GetSpellProto()->SpellIconID == 238)
                        {
                            furorChance = (*i)->GetModifier()->m_amount;
                            break;
                        }
                    }

                    if (m_modifier.m_miscvalue == FORM_CAT)
                    {
                        // Furor chance is now amount allowed to save energy for cat form
                        // without talent it reset to 0
                        if ((int32)target->GetPower(POWER_ENERGY) > furorChance)
                        {
                            target->SetPower(POWER_ENERGY, 0);
                            target->CastCustomSpell(target, 17099, &furorChance, NULL, NULL, true, NULL, this);
                        }
                    }
                    else if (furorChance)                   // only if talent known
                    {
                        target->SetPower(POWER_RAGE, 0);
                        if (irand(1, 100) <= furorChance)
                            target->CastSpell(target, 17057, true, NULL, this);
                    }
                    break;
                }
                case FORM_BATTLESTANCE:
                case FORM_DEFENSIVESTANCE:
                case FORM_BERSERKERSTANCE:
                {
                    uint32 Rage_val = 0;
                    // Stance mastery + Tactical mastery (both passive, and last have aura only in defense stance, but need apply at any stance switch)
                    if (target->GetTypeId() == TYPEID_PLAYER)
                    {
                        PlayerSpellMap const& sp_list = ((Player *)target)->GetSpellMap();
                        for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                        {
                            if (itr->second.state == PLAYERSPELL_REMOVED)
                                continue;

                            SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);
                            if (spellInfo && spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR && spellInfo->SpellIconID == 139)
                                Rage_val += target->CalculateSpellDamage(target, spellInfo, EFFECT_INDEX_0) * 10;
                        }
                    }

                    if (target->GetPower(POWER_RAGE) > Rage_val)
                        target->SetPower(POWER_RAGE, Rage_val);
                    break;
                }
                default:
                    break;
            }
        }

        target->SetShapeshiftForm(form);
    }
    else
    {
        if (modelid > 0)
            target->SetDisplayId(target->GetNativeDisplayId());

        if (target->getClass() == CLASS_DRUID)
            target->setPowerType(POWER_MANA);

        target->SetShapeshiftForm(FORM_NONE);

        // re-apply transform display with preference negative cases
        Unit::AuraList& otherTransforms = target->GetAurasByType(SPELL_AURA_TRANSFORM);
        if (!otherTransforms.empty())
        {
            // look for other transform auras
            Aura* handledAura = (*otherTransforms.begin())();
            for (Unit::AuraList::iterator itr = otherTransforms.begin(); itr != otherTransforms.end(); ++itr)
            {
                if (itr->IsEmpty())
                    continue;

                // negative auras are preferred
                if (!IsPositiveSpell((*itr)->GetId()))
                {
                    handledAura = (*itr)();
                    break;
                }
            }
            handledAura->ApplyModifier(true);
        }
    }

    // adding/removing linked auras
    // add/remove the shapeshift aura's boosts
    HandleShapeshiftBoosts(apply);

    target->UpdateSpeed(MOVE_RUN, true);

    if (target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)target)->InitDataForForm();

}

void Aura::HandleAuraTransform(bool apply, bool Real)
{
    Unit *target = GetTarget();
    if (apply)
    {
        do { // (to avoid lots of indentation changes)

        // update active transform spell only when transform or shapeshift not set or not overwriting negative by positive case
        if (target->GetModelForForm() && IsPositiveSpell(GetId()))
            break;

        // special case (spell specific functionality)
        if (m_modifier.m_miscvalue == 0)
        {
            switch (GetId())
            {
                case 16739:                                 // Orb of Deception
                {
                    uint32 orb_model = target->GetNativeDisplayId();
                    switch(orb_model)
                    {
                        // Troll Female
                        case 1479: target->SetDisplayId(10134); break;
                        // Troll Male
                        case 1478: target->SetDisplayId(10135); break;
                        // Tauren Male
                        case 59:   target->SetDisplayId(10136); break;
                        // Human Male
                        case 49:   target->SetDisplayId(10137); break;
                        // Human Female
                        case 50:   target->SetDisplayId(10138); break;
                        // Orc Male
                        case 51:   target->SetDisplayId(10139); break;
                        // Orc Female
                        case 52:   target->SetDisplayId(10140); break;
                        // Dwarf Male
                        case 53:   target->SetDisplayId(10141); break;
                        // Dwarf Female
                        case 54:   target->SetDisplayId(10142); break;
                        // NightElf Male
                        case 55:   target->SetDisplayId(10143); break;
                        // NightElf Female
                        case 56:   target->SetDisplayId(10144); break;
                        // Undead Female
                        case 58:   target->SetDisplayId(10145); break;
                        // Undead Male
                        case 57:   target->SetDisplayId(10146); break;
                        // Tauren Female
                        case 60:   target->SetDisplayId(10147); break;
                        // Gnome Male
                        case 1563: target->SetDisplayId(10148); break;
                        // Gnome Female
                        case 1564: target->SetDisplayId(10149); break;
                        // BloodElf Female
                        case 15475: target->SetDisplayId(17830); break;
                        // BloodElf Male
                        case 15476: target->SetDisplayId(17829); break;
                        // Dranei Female
                        case 16126: target->SetDisplayId(17828); break;
                        // Dranei Male
                        case 16125: target->SetDisplayId(17827); break;
                        default: break;
                    }
                    break;
                }
                case 42365:                                 // Murloc costume
                    target->SetDisplayId(21723);
                    break;
                //case 44186:                               // Gossip NPC Appearance - All, Brewfest
                    //break;
                //case 48305:                               // Gossip NPC Appearance - All, Spirit of Competition
                    //break;
                case 50517:                                 // Dread Corsair
                case 51926:                                 // Corsair Costume
                {
                    // expected for players
                    uint32 race = target->getRace();

                    switch(race)
                    {
                        case RACE_HUMAN:
                            target->SetDisplayId(target->getGender() == GENDER_MALE ? 25037 : 25048);
                            break;
                        case RACE_ORC:
                            target->SetDisplayId(target->getGender() == GENDER_MALE ? 25039 : 25050);
                            break;
                        case RACE_DWARF:
                            target->SetDisplayId(target->getGender() == GENDER_MALE ? 25034 : 25045);
                            break;
                        case RACE_NIGHTELF:
                            target->SetDisplayId(target->getGender() == GENDER_MALE ? 25038 : 25049);
                            break;
                        case RACE_UNDEAD:
                            target->SetDisplayId(target->getGender() == GENDER_MALE ? 25042 : 25053);
                            break;
                        case RACE_TAUREN:
                            target->SetDisplayId(target->getGender() == GENDER_MALE ? 25040 : 25051);
                            break;
                        case RACE_GNOME:
                            target->SetDisplayId(target->getGender() == GENDER_MALE ? 25035 : 25046);
                            break;
                        case RACE_TROLL:
                            target->SetDisplayId(target->getGender() == GENDER_MALE ? 25041 : 25052);
                            break;
                        case RACE_GOBLIN:                   // not really player race (3.x), but model exist
                            target->SetDisplayId(target->getGender() == GENDER_MALE ? 25036 : 25047);
                            break;
                    }

                    break;
                }
                //case 50531:                               // Gossip NPC Appearance - All, Pirate Day
                    //break;
                //case 51010:                               // Dire Brew
                    //break;
                //case 53806:                               // Pygmy Oil
                    //break;
                case 62847:                                 // NPC Appearance - Valiant 02
                    target->SetDisplayId(target->getGender() == GENDER_MALE ? 26185 : 26186);
                    break;
                //case 62852:                               // NPC Appearance - Champion 01
                    //break;
                //case 63965:                               // NPC Appearance - Champion 02
                    //break;
                //case 63966:                               // NPC Appearance - Valiant 03
                    //break;
                case 65386:                                 // Honor the Dead
                case 65495:
                {
                    switch(target->getGender())
                    {
                        case GENDER_MALE:
                            target->SetDisplayId(29203);    // Chapman
                            break;
                        case GENDER_FEMALE:
                        case GENDER_NONE:
                            target->SetDisplayId(29204);    // Catrina
                            break;
                    }
                    break;
                }
                //case 65511:                               // Gossip NPC Appearance - Brewfest
                    //break;
                //case 65522:                               // Gossip NPC Appearance - Winter Veil
                    //break;
                //case 65523:                               // Gossip NPC Appearance - Default
                    //break;
                //case 65524:                               // Gossip NPC Appearance - Lunar Festival
                    //break;
                //case 65525:                               // Gossip NPC Appearance - Hallow's End
                    //break;
                //case 65526:                               // Gossip NPC Appearance - Midsummer
                    //break;
                //case 65527:                               // Gossip NPC Appearance - Spirit of Competition
                    //break;
                case 65528:                                 // Gossip NPC Appearance - Pirates' Day
                {
                    // expecting npc's using this spell to have models with race info.
                    // random gender, regardless of current gender
                    switch (target->getRace())
                    {
                        case RACE_HUMAN:
                            target->SetDisplayId(roll_chance_i(50) ? 25037 : 25048);
                            break;
                        case RACE_ORC:
                            target->SetDisplayId(roll_chance_i(50) ? 25039 : 25050);
                            break;
                        case RACE_DWARF:
                            target->SetDisplayId(roll_chance_i(50) ? 25034 : 25045);
                            break;
                        case RACE_NIGHTELF:
                            target->SetDisplayId(roll_chance_i(50) ? 25038 : 25049);
                            break;
                        case RACE_UNDEAD:
                            target->SetDisplayId(roll_chance_i(50) ? 25042 : 25053);
                            break;
                        case RACE_TAUREN:
                            target->SetDisplayId(roll_chance_i(50) ? 25040 : 25051);
                            break;
                        case RACE_GNOME:
                            target->SetDisplayId(roll_chance_i(50) ? 25035 : 25046);
                            break;
                        case RACE_TROLL:
                            target->SetDisplayId(roll_chance_i(50) ? 25041 : 25052);
                            break;
                        case RACE_GOBLIN:
                            target->SetDisplayId(roll_chance_i(50) ? 25036 : 25047);
                            break;
                    }

                    break;
                }
                case 65529:                                 // Gossip NPC Appearance - Day of the Dead (DotD)
                    // random, regardless of current gender
                    target->SetDisplayId(roll_chance_i(50) ? 29203 : 29204);
                    break;
                //case 66236:                               // Incinerate Flesh
                    //break;
                //case 69999:                               // [DND] Swap IDs
                    //break;
                //case 70764:                               // Citizen Costume (note: many spells w/same name)
                    //break;
                //case 71309:                               // [DND] Spawn Portal
                    //break;
                case 71450:                                 // Crown Parcel Service Uniform
                    target->SetDisplayId(target->getGender() == GENDER_MALE ? 31002 : 31003);
                    break;
                //case 75531:                               // Gnomeregan Pride
                    //break;
                //case 75532:                               // Darkspear Pride
                    //break;
                default:
                    sLog.outError("Aura::HandleAuraTransform, spell %u does not have creature entry defined, need custom defined model.", GetId());
                    break;
            }
        }
        else                                                // m_modifier.m_miscvalue != 0
        {
            uint32 model_id;

            CreatureInfo const * ci = ObjectMgr::GetCreatureTemplate(m_modifier.m_miscvalue);
            if (!ci)
            {
                model_id = 16358;                           // pig pink ^_^
                sLog.outError("Auras: unknown creature id = %d (only need its modelid) Form Spell Aura Transform in Spell ID = %d", m_modifier.m_miscvalue, GetId());
            }
            else
                model_id = Creature::ChooseDisplayId(ci);   // Will use the default model here

            target->SetDisplayId(model_id);

            // creature case, need to update equipment if additional provided
            if (ci && target->GetTypeId() == TYPEID_UNIT)
                ((Creature*)target)->LoadEquipment(ci->equipmentId, false);
        }

        } while (0);

        // update active transform spell only not set or not overwriting negative by positive case
        if (!target->getTransForm() || !IsPositiveSpell(GetId()) || IsPositiveSpell(target->getTransForm()))
            target->setTransForm(GetId());

        // polymorph case
        if (Real && target->GetTypeId() == TYPEID_PLAYER && target->IsPolymorphed())
        {
            // for players, start regeneration after 1s (in polymorph fast regeneration case)
            // only if caster is Player (after patch 2.4.2)
            if (GetCasterGuid().IsPlayer())
                ((Player*)target)->setRegenTimer(1*IN_MILLISECONDS);

            //dismount polymorphed target (after patch 2.4.2)
            if (target->IsMounted())
                target->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED, GetHolder());
        }
    }
    else                                                    // !apply
    {
        // ApplyModifier(true) will reapply it if need
        target->setTransForm(0);
        target->SetDisplayId(target->GetNativeDisplayId());

        // apply default equipment for creature case
        if (target->GetTypeId() == TYPEID_UNIT)
            ((Creature*)target)->LoadEquipment(((Creature*)target)->GetCreatureInfo()->equipmentId, true);

        // re-apply some from still active with preference negative cases
        Unit::AuraList& otherTransforms = target->GetAurasByType(SPELL_AURA_TRANSFORM);
        if (!otherTransforms.empty())
        {
            // look for other transform auras
            Aura* handledAura = (*otherTransforms.begin())();
            for(Unit::AuraList::iterator i = otherTransforms.begin();i != otherTransforms.end(); ++i)
            {
                // negative auras are preferred
                if (!IsPositiveSpell((*i)->GetSpellProto()->Id))
                {
                    handledAura = (*i)();
                    break;
                }
            }
            handledAura->ApplyModifier(true);
        }
        // re-apply shapeshift display if no transform auras remaining
        else if (target->GetShapeshiftForm())
        {
            if (uint32 modelid = target->GetModelForForm())
                target->SetDisplayId(modelid);
        }

        // Dragonmaw Illusion (restore mount model)
        if (GetId() == 42016 && target->GetMountID() == 16314)
        {
            if (!target->GetAurasByType(SPELL_AURA_MOUNTED).empty())
            {
                uint32 cr_id = target->GetAurasByType(SPELL_AURA_MOUNTED).front()->GetModifier()->m_miscvalue;
                if (CreatureInfo const* ci = ObjectMgr::GetCreatureTemplate(cr_id))
                {
                    uint32 display_id = Creature::ChooseDisplayId(ci);
                    CreatureModelInfo const *minfo = sObjectMgr.GetCreatureModelRandomGender(display_id);
                    if (minfo)
                        display_id = minfo->modelid;

                    target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, display_id);
                }
            }
        }
    }
}

void Aura::HandleForceReaction(bool apply, bool Real)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    if(!Real)
        return;

    Player* player = (Player*)GetTarget();

    uint32 faction_id = m_modifier.m_miscvalue;
    ReputationRank faction_rank = ReputationRank(m_modifier.m_amount);

    player->GetReputationMgr().ApplyForceReaction(faction_id, faction_rank, apply);
    player->GetReputationMgr().SendForceReactions();

    // stop fighting if at apply forced rank friendly or at remove real rank friendly
    if ((apply && faction_rank >= REP_FRIENDLY) || (!apply && player->GetReputationRank(faction_id) >= REP_FRIENDLY))
        player->StopAttackFaction(faction_id);

    // drop BG flag if player is carrying
    if (SpellEntry const *spellInfo = GetSpellProto())
    {
        switch(spellInfo->Id)
        {
            case 1953:                          // Blink
            case 48020:                         // Demonic Circle
            case 54861:                         // Nitro Boosts
                if (player->InBattleGround() && (player->HasAura(23335) || player->HasAura(23333) || player->HasAura(34976)))
                    if (BattleGround *bg = player->GetBattleGround())
                        bg->EventPlayerDroppedFlag(player);
                break;
            default:
                break;
        }
    }
}

void Aura::HandleAuraModSkill(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 prot=GetSpellProto()->EffectMiscValue[m_effIndex];
    int32 points = GetModifier()->m_amount;

    ((Player*)GetTarget())->ModifySkillBonus(prot, (apply ? points: -points), m_modifier.m_auraname == SPELL_AURA_MOD_SKILL_TALENT);
    if (prot == SKILL_DEFENSE)
        ((Player*)GetTarget())->UpdateDefenseBonusesMod();
}

void Aura::HandleChannelDeathItem(bool apply, bool Real)
{
    if (Real && !apply)
    {
        if (m_removeMode != AURA_REMOVE_BY_DEATH)
            return;
        // Item amount
        if (m_modifier.m_amount <= 0)
            return;

        SpellEntry const *spellInfo = GetSpellProto();
        if (spellInfo->EffectItemType[m_effIndex] == 0)
            return;

        Unit* victim = GetTarget();
        Unit* caster = GetCaster();
        if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
            return;

        // Soul Shard (target req.)
        if (spellInfo->EffectItemType[m_effIndex] == 6265)
        {
            // Only from non-grey units
            if (!((Player*)caster)->isHonorOrXPTarget(victim) ||
                (victim->GetTypeId() == TYPEID_UNIT && !((Player*)caster)->isAllowedToLoot((Creature*)victim)))
                return;
        }

        //Adding items
        uint32 noSpaceForCount = 0;
        uint32 count = m_modifier.m_amount;

        ItemPosCountVec dest;
        InventoryResult msg = ((Player*)caster)->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, spellInfo->EffectItemType[m_effIndex], count, &noSpaceForCount);
        if ( msg != EQUIP_ERR_OK )
        {
            count-=noSpaceForCount;
            ((Player*)caster)->SendEquipError( msg, NULL, NULL, spellInfo->EffectItemType[m_effIndex] );
            if (count==0)
                return;
        }

        Item* newitem = ((Player*)caster)->StoreNewItem(dest, spellInfo->EffectItemType[m_effIndex], true);
        ((Player*)caster)->SendNewItem(newitem, count, true, true);
    }
}

void Aura::HandleBindSight(bool apply, bool /*Real*/)
{
    Unit* caster = GetCaster();

    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if (apply)
        ((Player*)caster)->SetViewPoint(GetTarget());
    else
        ((Player*)caster)->SetViewPoint(NULL);
}

void Aura::HandleFarSight(bool apply, bool /*Real*/)
{
    Unit* caster = GetCaster();
    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if (apply)
        ((Player*)caster)->SetViewPoint(GetTarget());
    else
        ((Player*)caster)->SetViewPoint(NULL);
}

void Aura::HandleAuraTrackCreatures(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId()!=TYPEID_PLAYER)
        return;

    if (apply)
        GetTarget()->RemoveNoStackAurasDueToAuraHolder(GetHolder());

    if (apply)
        GetTarget()->SetFlag(PLAYER_TRACK_CREATURES, uint32(1) << (m_modifier.m_miscvalue-1));
    else
        GetTarget()->RemoveFlag(PLAYER_TRACK_CREATURES, uint32(1) << (m_modifier.m_miscvalue-1));
}

void Aura::HandleAuraTrackResources(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId()!=TYPEID_PLAYER)
        return;

    if (apply)
        GetTarget()->RemoveNoStackAurasDueToAuraHolder(GetHolder());

    if (apply)
        GetTarget()->SetFlag(PLAYER_TRACK_RESOURCES, uint32(1) << (m_modifier.m_miscvalue-1));
    else
        GetTarget()->RemoveFlag(PLAYER_TRACK_RESOURCES, uint32(1) << (m_modifier.m_miscvalue-1));
}

void Aura::HandleAuraTrackStealthed(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId()!=TYPEID_PLAYER)
        return;

    if (apply)
        GetTarget()->RemoveNoStackAurasDueToAuraHolder(GetHolder());

    GetTarget()->ApplyModByteFlag(PLAYER_FIELD_BYTES, 0, PLAYER_FIELD_BYTE_TRACK_STEALTHED, apply);
}

void Aura::HandleAuraModScale(bool apply, bool /*Real*/)
{
    GetTarget()->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE_X, float(m_modifier.m_amount), apply);
    GetTarget()->UpdateModelData();
}

void Aura::HandleModPossess(bool apply, bool Real)
{
    if(!Real)
        return;

    Unit *target = GetTarget();

    // not possess yourself
    if (GetCasterGuid() == target->GetObjectGuid())
        return;

    Unit* caster = GetCaster();
    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)caster;

    if (apply)
    {
        target->addUnitState(UNIT_STAT_CONTROLLED);

        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
        target->SetCharmerGuid(p_caster->GetObjectGuid());
        target->setFaction(p_caster->getFaction());

        // target should became visible at SetView call(if not visible before):
        // otherwise client\p_caster will ignore packets from the target(SetClientControl for example)
        p_caster->SetViewPoint(target);

        p_caster->SetCharm(target);
        p_caster->SetClientControl(target, 1);
        p_caster->SetMover(target);

        target->CombatStop(true);
        target->DeleteThreatList();
        target->getHostileRefManager().deleteReferences();

        if (CharmInfo *charmInfo = target->InitCharmInfo(target))
        {
            charmInfo->SetState(CHARM_STATE_REACT,REACT_PASSIVE);
            charmInfo->SetState(CHARM_STATE_COMMAND,COMMAND_STAY);
            charmInfo->InitPossessCreateSpells();
        }

        p_caster->PossessSpellInitialize();

        if (target->GetTypeId() == TYPEID_UNIT)
        {
            ((Creature*)target)->AIM_Initialize();
        }
        else if (target->GetTypeId() == TYPEID_PLAYER)
        {
            ((Player*)target)->SetClientControl(target, 0);
        }

    }
    else
    {
        p_caster->SetCharm(NULL);

        p_caster->SetClientControl(target, 0);
        p_caster->SetMover(p_caster);

        // there is a possibility that target became invisible for client\p_caster at ResetView call:
        // it must be called after movement control unapplying, not before! the reason is same as at aura applying
        p_caster->SetViewPoint(NULL);

        p_caster->RemovePetActionBar();

        // on delete only do caster related effects
        if (m_removeMode == AURA_REMOVE_BY_DELETE)
            return;

        target->clearUnitState(UNIT_STAT_CONTROLLED);

        target->CombatStop(true);
        target->DeleteThreatList();
        target->getHostileRefManager().deleteReferences();

        target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

        target->SetCharmerGuid(ObjectGuid());

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            ((Player*)target)->setFactionForRace(target->getRace());
            ((Player*)target)->SetClientControl(target, 1);
        }
        else if (target->GetTypeId() == TYPEID_UNIT)
        {
            CreatureInfo const *cinfo = ((Creature*)target)->GetCreatureInfo();
            target->setFaction(cinfo->faction_A);
        }

        if (target->GetTypeId() == TYPEID_UNIT)
        {
            ((Creature*)target)->AIM_Initialize();
            target->AttackedBy(caster);
        }
    }
}

void Aura::HandleModPossessPet(bool apply, bool Real)
{
    if(!Real)
        return;

    Unit* caster = GetCaster();
    if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Unit* target = GetTarget();
    if (target->GetTypeId() != TYPEID_UNIT || !((Creature*)target)->IsPet())
        return;

    Pet* pet = (Pet*)target;
    Player* p_caster = (Player*)caster;

    if (apply)
    {
        pet->addUnitState(UNIT_STAT_CONTROLLED);

        // target should became visible at SetView call(if not visible before):
        // otherwise client\p_caster will ignore packets from the target(SetClientControl for example)
        p_caster->SetViewPoint(pet);

        p_caster->SetCharm(pet);
        p_caster->SetClientControl(pet, 1);
        p_caster->SetMover(pet);

        pet->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

        pet->StopMoving();
        pet->GetMotionMaster()->MoveIdle();
    }
    else
    {
        p_caster->SetCharm(NULL);
        p_caster->SetClientControl(pet, 0);
        p_caster->SetMover(p_caster);

        // there is a possibility that target became invisible for client\p_caster at ResetView call:
        // it must be called after movement control unapplying, not before! the reason is same as at aura applying
        p_caster->SetViewPoint(NULL);

        // on delete only do caster related effects
        if (m_removeMode == AURA_REMOVE_BY_DELETE)
            return;

        pet->clearUnitState(UNIT_STAT_CONTROLLED);

        pet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

        pet->AttackStop();

        // out of range pet dismissed
        if (!pet->IsWithinDistInMap(p_caster, pet->GetMap()->GetVisibilityDistance()))
        {
            p_caster->RemovePet(PET_SAVE_REAGENTS);
        }
        else
        {
            pet->GetMotionMaster()->MoveTargetedHome();
        }
    }
}

void Aura::HandleModCharm(bool apply, bool Real)
{
    if(!Real)
        return;

    Unit *target = GetTarget();

    // not charm yourself
    if (GetCasterGuid() == target->GetObjectGuid())
        return;

    Unit* caster = GetCaster();
    if(!caster)
        return;

    if ( apply )
    {
        // is it really need after spell check checks?
        target->RemoveSpellsCausingAura(SPELL_AURA_MOD_CHARM, GetHolder());
        target->RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS, GetHolder());

        target->SetCharmerGuid(GetCasterGuid());
        target->setFaction(caster->getFaction());
        target->CastStop(target == caster ? GetId() : 0);
        caster->SetCharm(target);

        target->CombatStop(true);
        target->DeleteThreatList();
        target->getHostileRefManager().deleteReferences();

        if (target->GetTypeId() == TYPEID_UNIT)
        {
            ((Creature*)target)->AIM_Initialize();
            CharmInfo *charmInfo = target->InitCharmInfo(target);
            charmInfo->SetState(CHARM_STATE_REACT,REACT_DEFENSIVE);
            charmInfo->InitCharmCreateSpells();

            if (caster->GetTypeId() == TYPEID_PLAYER && caster->getClass() == CLASS_WARLOCK)
            {
                CreatureInfo const *cinfo = ((Creature*)target)->GetCreatureInfo();
                if (cinfo && cinfo->type == CREATURE_TYPE_DEMON)
                {
                    // creature with pet number expected have class set
                    if (target->GetByteValue(UNIT_FIELD_BYTES_0, 1)==0)
                    {
                        if (cinfo->unit_class==0)
                            sLog.outErrorDb("Creature (Entry: %u) have unit_class = 0 but used in charmed spell, that will be result client crash.",cinfo->Entry);
                        else
                            sLog.outError("Creature (Entry: %u) have unit_class = %u but at charming have class 0!!! that will be result client crash.",cinfo->Entry,cinfo->unit_class);

                        target->SetByteValue(UNIT_FIELD_BYTES_0, 1, CLASS_MAGE);
                    }

                    //just to enable stat window
                    charmInfo->SetPetNumber(sObjectMgr.GeneratePetNumber(), true);
                    //if charmed two demons the same session, the 2nd gets the 1st one's name
                    target->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(NULL)));
                }
            }
        }

        if (caster->GetTypeId() == TYPEID_PLAYER)
            ((Player*)caster)->CharmSpellInitialize();
    }
    else
    {
        target->SetCharmerGuid(ObjectGuid());

        if (target->GetTypeId() == TYPEID_PLAYER)
            ((Player*)target)->setFactionForRace(target->getRace());
        else
        {
            CreatureInfo const *cinfo = ((Creature*)target)->GetCreatureInfo();

            // restore faction
            if(((Creature*)target)->IsPet())
            {
                if (Unit* owner = ((Pet*)target)->GetOwner())
                    target->setFaction(owner->getFaction());
                else if (cinfo)
                    target->setFaction(cinfo->faction_A);
            }
            else if (cinfo)                              // normal creature
                target->setFaction(cinfo->faction_A);

            // restore UNIT_FIELD_BYTES_0
            if (cinfo && caster->GetTypeId() == TYPEID_PLAYER && caster->getClass() == CLASS_WARLOCK && cinfo->type == CREATURE_TYPE_DEMON)
            {
                // DB must have proper class set in field at loading, not req. restore, including workaround case at apply
                // m_target->SetByteValue(UNIT_FIELD_BYTES_0, 1, cinfo->unit_class);

                if (target->GetCharmInfo())
                    target->GetCharmInfo()->SetPetNumber(0, true);
                else
                    sLog.outError("Aura::HandleModCharm: target (GUID: %u TypeId: %u) has a charm aura but no charm info!", target->GetGUIDLow(), target->GetTypeId());
            }
        }

        caster->SetCharm(NULL);

        if (caster->GetTypeId() == TYPEID_PLAYER)
            ((Player*)caster)->RemovePetActionBar();

        target->CombatStop(true);
        target->DeleteThreatList();
        target->getHostileRefManager().deleteReferences();

        if (target->GetTypeId() == TYPEID_UNIT)
        {
            ((Creature*)target)->AIM_Initialize();
            target->AttackedBy(caster);
        }
    }
}

void Aura::HandleModConfuse(bool apply, bool Real)
{
    if(!Real)
        return;

    if(!apply && GetTarget()->HasAuraType(GetModifier()->m_auraname))
        return;

    GetTarget()->SetConfused(apply, GetCasterGuid(), GetId());
}

void Aura::HandleModFear(bool apply, bool Real)
{
    if (!Real)
        return;

    if(!apply && GetTarget()->HasAuraType(GetModifier()->m_auraname))
        return;

    GetTarget()->SetFeared(apply, GetCasterGuid(), GetId());
}

void Aura::HandleFeignDeath(bool apply, bool Real)
{
    if(!Real)
        return;

    if(!apply && GetTarget()->HasAuraType(GetModifier()->m_auraname))
        return;

    GetTarget()->SetFeignDeath(apply, GetCasterGuid(), GetId());
}

void Aura::HandleAuraModDisarm(bool apply, bool Real)
{
    if(!Real)
        return;

    Unit *target = GetTarget();

    if(!apply && target->HasAuraType(GetModifier()->m_auraname))
        return;

    uint32 flags;
    uint32 field;
    WeaponAttackType attack_type;

    switch (GetModifier()->m_auraname)
    {
        default:
        case SPELL_AURA_MOD_DISARM:
        {
            field = UNIT_FIELD_FLAGS;
            flags = UNIT_FLAG_DISARMED;
            attack_type = BASE_ATTACK;
            break;
        }
    }

    target->ApplyModFlag(field, flags, apply);

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    // main-hand attack speed already set to special value for feral form already and don't must change and reset at remove.
    if (target->IsInFeralForm())
        return;

    if (apply)
    {
        target->SetAttackTime(BASE_ATTACK,BASE_ATTACK_TIME);
    }
    else
        ((Player *)target)->SetRegularAttackTime();

    target->UpdateDamagePhysical(attack_type);
}

void Aura::HandleAuraModStun(bool apply, bool Real)
{
    if(!Real)
        return;

    Unit *target = GetTarget();

    if (apply)
    {
        // Frost stun aura -> freeze/unfreeze target
        if (GetSpellSchoolMask(GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
            target->ModifyAuraState(AURA_STATE_FROZEN, apply);

        target->CastStop(target->GetObjectGuid() == GetCasterGuid() ? GetId() : 0);
        target->GetUnitStateMgr().PushAction(UNIT_ACTION_STUN);

        // Deep Freeze damage part
        if (GetId() == 44572 && !target->IsCharmerOrOwnerPlayerOrPlayerItself() && target->IsImmuneToSpellEffect(GetSpellProto(), EFFECT_INDEX_0))
        {
            Unit* caster = GetCaster();
            if(!caster)
                return;
            caster->CastSpell(target, 71757, true);
        }

        // Summon the Naj'entus Spine GameObject on target if spell is Impaling Spine
        switch(GetId())
        {
            case 39837: // Impaling Spine
            {
                GameObject* pObj = new GameObject;
                if (pObj->Create(target->GetMap()->GenerateLocalLowGuid(HIGHGUID_GAMEOBJECT), 185584, target->GetMap(), target->GetPhaseMask(),
                    target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation()))
                {
                    pObj->SetRespawnTime(GetAuraDuration()/IN_MILLISECONDS);
                    pObj->SetSpellId(GetId());
                    target->AddGameObject(pObj);
                    target->GetMap()->Add(pObj);
                }
                else
                    delete pObj;

                break;
            }
            case 6358: // Seduction
            {
                if (Unit* caster = GetCaster())
                {
                    if (!apply)
                        caster->InterruptSpell(CURRENT_CHANNELED_SPELL, false);
                }
                break;
            }
        }
    }
    else
    {
        // Frost stun aura -> freeze/unfreeze target
        if (GetSpellSchoolMask(GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
        {
            bool found_another = false;
            for(AuraType const* itr = &frozenAuraTypes[0]; *itr != SPELL_AURA_NONE; ++itr)
            {
                Unit::AuraList const& auras = target->GetAurasByType(*itr);
                for(Unit::AuraList::const_iterator i = auras.begin(); i != auras.end(); ++i)
                {
                    if ( GetSpellSchoolMask((*i)->GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
                    {
                        found_another = true;
                        break;
                    }
                }
                if (found_another)
                    break;
            }

            if(!found_another)
                target->ModifyAuraState(AURA_STATE_FROZEN, apply);
        }

        // Real remove called after current aura remove from lists, check if other similar auras active
        if (target->HasAuraType(SPELL_AURA_MOD_STUN))
            return;

        target->GetUnitStateMgr().DropAction(UNIT_ACTION_STUN);

        if(!target->hasUnitState(UNIT_STAT_ROOT))       // prevent allow move if have also root effect
        {
            if (target->getVictim() && target->isAlive())
                target->SetTargetGuid(target->getVictim()->GetObjectGuid());

            target->SetRoot(false);
        }

        // Wyvern Sting
        if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_HUNTER && GetSpellProto()->GetSpellFamilyFlags().test<CF_HUNTER_WYVERN_STING2>())
        {
            Unit* caster = GetCaster();
            if ( !caster || caster->GetTypeId()!=TYPEID_PLAYER )
                return;

            uint32 spell_id = 0;

            switch(GetId())
            {
                case 19386: spell_id = 24131; break;
                case 24132: spell_id = 24134; break;
                case 24133: spell_id = 24135; break;
                case 27068: spell_id = 27069; break;
                case 49011: spell_id = 49009; break;
                case 49012: spell_id = 49010; break;
                default:
                    sLog.outError("Spell selection called for unexpected original spell %u, new spell for this spell family?",GetId());
                    return;
            }

            SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell_id);

            if(!spellInfo)
                return;

            caster->CastSpell(target,spellInfo,true,NULL,this);
            return;
        }
    }
}

void Aura::HandleModStealth(bool apply, bool Real)
{
    Unit *target = GetTarget();

    if (apply)
    {
        // drop flag at stealth in bg
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

        // only at real aura add
        if (Real)
        {
            target->SetStandFlags(UNIT_STAND_FLAGS_CREEP);

            if (target->GetTypeId()==TYPEID_PLAYER)
                target->SetByteFlag(PLAYER_FIELD_BYTES2, 3, PLAYER_FIELD_BYTE2_STEALTH);

            // apply only if not in GM invisibility (and overwrite invisibility state)
            if (target->GetVisibility()!=VISIBILITY_OFF)
            {
                target->SetVisibility(VISIBILITY_GROUP_NO_DETECT);
                target->SetVisibility(VISIBILITY_GROUP_STEALTH);
            }

            // apply full stealth period bonuses only at first stealth aura in stack
            if (target->GetAurasByType(SPELL_AURA_MOD_STEALTH).size()<=2) // Vanish also triggering Stealth, but all ok, no double auras here possible
            {
                Unit::AuraList const& mDummyAuras = target->GetAurasByType(SPELL_AURA_DUMMY);
                for(Unit::AuraList::const_iterator i = mDummyAuras.begin();i != mDummyAuras.end(); ++i)
                {
                    // Master of Subtlety
                    if ((*i)->GetSpellProto()->SpellIconID == 2114 && GetSpellProto()->SpellFamilyName == SPELLFAMILY_ROGUE)
                    {
                        target->RemoveAurasDueToSpell(31666);
                        int32 bp = (*i)->GetModifier()->m_amount;
                        target->CastCustomSpell(target,31665,&bp,NULL,NULL,true);
                    }
                    // Overkill
                    else if ((*i)->GetId() == 58426 && GetSpellProto()->GetSpellFamilyFlags().test<CF_ROGUE_STEALTH>())
                    {
                        target->CastSpell(target, 58427, true);
                    }
                }
            }
        }
    }
    else
    {
        // only at real aura remove of _last_ SPELL_AURA_MOD_STEALTH
        if (Real && !target->HasAuraType(SPELL_AURA_MOD_STEALTH))
        {
            // if no GM invisibility
            if (target->GetVisibility()!=VISIBILITY_OFF)
            {
                target->RemoveStandFlags(UNIT_STAND_FLAGS_CREEP);

                if (target->GetTypeId()==TYPEID_PLAYER)
                    target->RemoveByteFlag(PLAYER_FIELD_BYTES2, 3, PLAYER_FIELD_BYTE2_STEALTH);

                // restore invisibility if any
                if (target->HasAuraType(SPELL_AURA_MOD_INVISIBILITY))
                {
                    target->SetVisibility(VISIBILITY_GROUP_NO_DETECT);
                    target->SetVisibility(VISIBILITY_GROUP_INVISIBILITY);
                }
                else
                    target->SetVisibility(VISIBILITY_ON);
            }

            // apply delayed talent bonus remover at last stealth aura remove
            Unit::AuraList const& mDummyAuras = target->GetAurasByType(SPELL_AURA_DUMMY);
            for(Unit::AuraList::const_iterator i = mDummyAuras.begin();i != mDummyAuras.end(); ++i)
            {
                // Master of Subtlety
                if ((*i)->GetSpellProto()->SpellIconID == 2114)
                    target->CastSpell(target, 31666, true);
                // Overkill
                else if ((*i)->GetId() == 58426)//Overkill we should remove anyway
                {
                    if (SpellAuraHolderPtr holder = target->GetSpellAuraHolder(58427))
                    {
                        holder->SetAuraMaxDuration(20*IN_MILLISECONDS);
                        holder->RefreshHolder();
                    }
                }
            }
        }
    }
}

void Aura::HandleInvisibility(bool apply, bool Real)
{
    Unit *target = GetTarget();

    if (apply)
    {
        target->m_invisibilityMask |= (1 << m_modifier.m_miscvalue);

        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

        if (Real && target->GetTypeId()==TYPEID_PLAYER)
        {
            // apply glow vision
            target->SetByteFlag(PLAYER_FIELD_BYTES2, 3, PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);

        }

        // apply only if not in GM invisibility and not stealth
        if (target->GetVisibility() == VISIBILITY_ON)
        {
            // Aura not added yet but visibility code expect temporary add aura
            target->SetVisibility(VISIBILITY_GROUP_NO_DETECT);
            target->SetVisibility(VISIBILITY_GROUP_INVISIBILITY);
        }
    }
    else
    {
        // recalculate value at modifier remove (current aura already removed)
        target->m_invisibilityMask = 0;
        Unit::AuraList const& auras = target->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY);
        for(Unit::AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
            target->m_invisibilityMask |= (1 << (*itr)->GetModifier()->m_miscvalue);

        // only at real aura remove and if not have different invisibility auras.
        if (Real && target->m_invisibilityMask == 0)
        {
            // remove glow vision
            if (target->GetTypeId() == TYPEID_PLAYER)
                target->RemoveByteFlag(PLAYER_FIELD_BYTES2, 3, PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);

            // apply only if not in GM invisibility & not stealthed while invisible
            if (target->GetVisibility() != VISIBILITY_OFF)
            {
                // if have stealth aura then already have stealth visibility
                if (!target->HasAuraType(SPELL_AURA_MOD_STEALTH))
                    target->SetVisibility(VISIBILITY_ON);
            }
        }

        if (GetId() == 48809)                               // Binding Life
            target->CastSpell(target, GetSpellProto()->CalculateSimpleValue(m_effIndex), true);
    }
}

void Aura::HandleInvisibilityDetect(bool apply, bool Real)
{
    Unit *target = GetTarget();

    if (apply)
    {
        target->m_detectInvisibilityMask |= (1 << m_modifier.m_miscvalue);
    }
    else
    {
        // recalculate value at modifier remove (current aura already removed)
        target->m_detectInvisibilityMask = 0;
        Unit::AuraList const& auras = target->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY_DETECTION);
        for(Unit::AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
            target->m_detectInvisibilityMask |= (1 << (*itr)->GetModifier()->m_miscvalue);
    }
    if (Real && target->GetTypeId()==TYPEID_PLAYER)
        ((Player*)target)->GetCamera()->UpdateVisibilityForOwner();
}

void Aura::HandleAuraModRoot(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    Unit *target = GetTarget();

    if (apply)
    {
        // Frost root aura -> freeze/unfreeze target
        if (GetSpellSchoolMask(GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
            target->ModifyAuraState(AURA_STATE_FROZEN, apply);

        target->GetUnitStateMgr().PushAction(UNIT_ACTION_ROOT);

    }
    else
    {
        // Frost root aura -> freeze/unfreeze target
        if (GetSpellSchoolMask(GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
        {
            bool found_another = false;
            for(AuraType const* itr = &frozenAuraTypes[0]; *itr != SPELL_AURA_NONE; ++itr)
            {
                Unit::AuraList const& auras = target->GetAurasByType(*itr);
                for(Unit::AuraList::const_iterator i = auras.begin(); i != auras.end(); ++i)
                {
                    if ( GetSpellSchoolMask((*i)->GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
                    {
                        found_another = true;
                        break;
                    }
                }
                if (found_another)
                    break;
            }

            if(!found_another)
                target->ModifyAuraState(AURA_STATE_FROZEN, apply);
        }

        // Real remove called after current aura remove from lists, check if other similar auras active
        if (target->HasAuraType(SPELL_AURA_MOD_ROOT))
            return;

        target->GetUnitStateMgr().DropAction(UNIT_ACTION_ROOT);

        if (GetSpellProto()->Id == 70980)                   // Web Wrap (Icecrown Citadel, trash mob Nerub'ar Broodkeeper)
            target->CastSpell(target, 71010, true);
    }
}

void Aura::HandleAuraModSilence(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    Unit *target = GetTarget();

    if (apply)
    {
        target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
        // Stop cast only spells vs PreventionType == SPELL_PREVENTION_TYPE_SILENCE
        for (uint32 i = CURRENT_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
            if (Spell* spell = target->GetCurrentSpell(CurrentSpellTypes(i)))
                if (spell->m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
                    // Stop spells on prepare or casting state
                    target->InterruptSpell(CurrentSpellTypes(i), false);
    }
    else
    {
        // Real remove called after current aura remove from lists, check if other similar auras active
        if (!target->HasAuraType(SPELL_AURA_MOD_SILENCE) &&
            !target->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
            target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
    }
}

void Aura::HandleModThreat(bool apply, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    Unit *target = GetTarget();

    if (!target->isAlive())
        return;

    int level_diff = 0;
    int multiplier = 0;
    switch (GetId())
    {
        // Arcane Shroud
        case 26400:
            level_diff = target->getLevel() - 60;
            multiplier = 2;
            break;
        // The Eye of Diminution
        case 28862:
            level_diff = target->getLevel() - 60;
            multiplier = 1;
            break;
    }

    if (level_diff > 0)
        m_modifier.m_amount += multiplier * level_diff;

    if (target->GetTypeId() == TYPEID_PLAYER)
        for(int8 x=0;x < MAX_SPELL_SCHOOL;x++)
            if (m_modifier.m_miscvalue & int32(1<<x))
                ApplyPercentModFloatVar(target->m_threatModifier[x], float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModTotalThreat(bool apply, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    Unit *target = GetTarget();

    if (!target->isAlive() || target->GetTypeId() != TYPEID_PLAYER)
        return;

    Unit* caster = GetCaster();

    if (!caster || !caster->isAlive())
        return;

    float threatMod = apply ? float(m_modifier.m_amount) : float(-m_modifier.m_amount);

    target->getHostileRefManager().threatAssist(caster, threatMod, GetSpellProto());
}

void Aura::HandleModTaunt(bool apply, bool Real)
{
    // only at real add/remove aura
    if (!Real)
        return;

    Unit *target = GetTarget();

    if (!target->isAlive() || !target->CanHaveThreatList())
        return;

    Unit* caster = GetCaster();

    if (!caster || !caster->isAlive())
        return;

    if (apply)
        target->TauntApply(caster);
    else
    {
        // When taunt aura fades out, mob will switch to previous target if current has less than 1.1 * secondthreat
        target->TauntFadeOut(caster);
    }
}

/*********************************************************/
/***                  MODIFY SPEED                     ***/
/*********************************************************/
void Aura::HandleAuraModIncreaseSpeed(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    Unit *target = GetTarget();

    GetTarget()->UpdateSpeed(MOVE_RUN, true);

    if (apply && GetSpellProto()->Id == 58875)
        target->CastSpell(target, 58876, true);
}

void Aura::HandleAuraModIncreaseMountedSpeed(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    Unit *target = GetTarget();

    target->UpdateSpeed(MOVE_RUN, true);

    // Festive Holiday Mount
    if (apply && GetSpellProto()->SpellIconID != 1794 && target->HasAura(62061))
        // Reindeer Transformation
        target->CastSpell(target, 25860, true, NULL, this);
}

void Aura::HandleAuraModIncreaseSwimSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    GetTarget()->UpdateSpeed(MOVE_SWIM, true);
}

void Aura::HandleAuraModDecreaseSpeed(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    Unit* target = GetTarget();

    if (apply)
    {
        // Gronn Lord's Grasp, becomes stoned
        if (GetId() == 33572)
        {
            if (GetStackAmount() >= 5 && !target->HasAura(33652))
                target->CastSpell(target, 33652, true);
        }
    }

    target->UpdateSpeed(MOVE_RUN, true);
    target->UpdateSpeed(MOVE_SWIM, true);
    target->UpdateSpeed(MOVE_FLIGHT, true);
}

void Aura::HandleAuraModUseNormalSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    Unit *target = GetTarget();

    target->UpdateSpeed(MOVE_RUN, true);
    target->UpdateSpeed(MOVE_SWIM, true);
    target->UpdateSpeed(MOVE_FLIGHT, true);
}

/*********************************************************/
/***                     IMMUNITY                      ***/
/*********************************************************/

void Aura::HandleModMechanicImmunity(bool apply, bool /*Real*/)
{
    uint32 misc  = m_modifier.m_miscvalue;

    Unit *target = GetTarget();

    if (apply && GetSpellProto()->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY))
    {
        uint32 mechanic = 1 << (misc-1);

        // immune movement impairment and loss of control (spell data have special structure for mark this case)
        if (IsSpellRemoveAllMovementAndControlLossEffects(GetSpellProto()))
            mechanic=IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK;

        target->RemoveAurasAtMechanicImmunity(mechanic, GetId());
    }

    target->ApplySpellImmune(GetId(),IMMUNITY_MECHANIC,misc,apply);

    // Demonic Circle
    if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_WARLOCK && GetSpellProto()->SpellIconID == 3221)
    {
        if (target->GetTypeId() != TYPEID_PLAYER)
            return;

        if (apply)
            if (GameObject* obj = target->GetGameObject(48018))
                ((Player*)target)->TeleportTo(obj->GetMapId(),obj->GetPositionX(),obj->GetPositionY(),obj->GetPositionZ(),obj->GetOrientation(), TELE_TO_NOT_LEAVE_COMBAT);
    }

    // Bestial Wrath
    if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_HUNTER && GetSpellProto()->SpellIconID == 1680)
    {
        // The Beast Within cast on owner if talent present
        if (Unit* owner = target->GetOwner())
        {
            // Search talent The Beast Within
            Unit::AuraList const& dummyAuras = owner->GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
            for(Unit::AuraList::const_iterator i = dummyAuras.begin(); i != dummyAuras.end(); ++i)
            {
                if ((*i)->GetSpellProto()->SpellIconID == 2229)
                {
                    if (apply)
                        owner->CastSpell(owner, 34471, true, NULL, this);
                    else
                        owner->RemoveAurasDueToSpell(34471);
                    break;
                }
            }
        }
    }
    // Heroic Fury (Intercept cooldown remove)
    else if (apply && GetSpellProto()->Id == 60970 && target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)target)->RemoveSpellCooldown(20252, true);
}

void Aura::HandleModMechanicImmunityMask(bool apply, bool /*Real*/)
{
    uint32 mechanic  = m_modifier.m_miscvalue;

    if (apply && GetSpellProto()->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY))
        GetTarget()->RemoveAurasAtMechanicImmunity(mechanic,GetId());

    // check implemented in Unit::IsImmuneToSpell and Unit::IsImmuneToSpellEffect
}

//this method is called whenever we add / remove aura which gives m_target some imunity to some spell effect
void Aura::HandleAuraModEffectImmunity(bool apply, bool /*Real*/)
{
    Unit *target = GetTarget();

    // when removing flag aura, handle flag drop
    if ( !apply && target->GetTypeId() == TYPEID_PLAYER
        && (GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION) )
    {
        Player* player = (Player*)target;
        if (BattleGround* bg = player->GetBattleGround())
            bg->EventPlayerDroppedFlag(player);
        else if (OutdoorPvP* outdoorPvP = sOutdoorPvPMgr.GetScript(player->GetCachedZoneId()))
            outdoorPvP->HandleDropFlag(player, GetSpellProto()->Id);
        else if (InstanceData* mapInstance = player->GetInstanceData())
            mapInstance->OnPlayerDroppedFlag(player, GetId());
    }

    target->ApplySpellImmune(GetId(), IMMUNITY_EFFECT, m_modifier.m_miscvalue, apply);
}

void Aura::HandleAuraModStateImmunity(bool apply, bool Real)
{
    if (apply && Real && GetSpellProto()->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY))
    {
        Unit::SpellIdSet toRemoveSpellList;
        Unit::AuraList const& auraList = GetTarget()->GetAurasByType(AuraType(m_modifier.m_miscvalue));
        for (Unit::AuraList::const_iterator itr = auraList.begin(); itr != auraList.end(); ++itr)
        {
            if (itr->IsEmpty() || (*itr)() == this)                   // skip itself aura (it already added)
                continue;
            toRemoveSpellList.insert(auraList.front()->GetId());
        }

        for (Unit::SpellIdSet::const_iterator i = toRemoveSpellList.begin(); i != toRemoveSpellList.end(); ++i)
            GetTarget()->RemoveAurasDueToSpell(*i);
    }

    GetTarget()->ApplySpellImmune(GetId(), IMMUNITY_STATE, m_modifier.m_miscvalue, apply);
}

void Aura::HandleAuraModSchoolImmunity(bool apply, bool Real)
{
    Unit* target = GetTarget();
    target->ApplySpellImmune(GetId(), IMMUNITY_SCHOOL, m_modifier.m_miscvalue, apply);

    // remove all flag auras (they are positive, but they must be removed when you are immune)
    if (GetSpellProto()->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY)
        && GetSpellProto()->HasAttribute(SPELL_ATTR_EX2_DAMAGE_REDUCED_SHIELD))
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

    // TODO: optimalize this cycle - use RemoveAurasWithInterruptFlags call or something else
    if (Real && apply &&
        GetSpellProto()->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY) &&
        IsPositiveSpell(GetId()))                       //Only positive immunity removes auras
    {
        std::set<uint32> toRemoveSpellList;
        uint32 school_mask = m_modifier.m_miscvalue;
        Unit::SpellAuraHolderMap& Auras = target->GetSpellAuraHolderMap();
        for (Unit::SpellAuraHolderMap::iterator iter = Auras.begin(); iter != Auras.end(); ++iter)
        {
            if (!iter->second || iter->second->IsDeleted())
                continue;

            SpellEntry const* spell = iter->second->GetSpellProto();

            if ((GetSpellSchoolMask(spell) & school_mask)//Check for school mask
                && !spell->HasAttribute(SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY)   //Spells unaffected by invulnerability
                && !iter->second->IsPositive()                //Don't remove positive spells
                && spell->Id != GetId())                //Don't remove self
            {
                toRemoveSpellList.insert(iter->second->GetId());
            }
        }
        for (std::set<uint32>::iterator i = toRemoveSpellList.begin(); i != toRemoveSpellList.end(); ++i)
            target->RemoveAurasDueToSpell(*i);
    }

    if ( Real && GetSpellProto()->Mechanic == MECHANIC_BANISH )
    {
        if ( apply )
            target->addUnitState(UNIT_STAT_ISOLATED);
        else
            target->clearUnitState(UNIT_STAT_ISOLATED);
    }
}

void Aura::HandleAuraModDmgImmunity(bool apply, bool /*Real*/)
{
    GetTarget()->ApplySpellImmune(GetId(), IMMUNITY_DAMAGE, m_modifier.m_miscvalue, apply);
}

void Aura::HandleAuraModDispelImmunity(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    GetTarget()->ApplySpellDispelImmunity(GetSpellProto(), DispelType(m_modifier.m_miscvalue), apply);
}

void Aura::HandleAuraProcTriggerSpell(bool apply, bool Real)
{
    if(!Real)
        return;

    Unit *target = GetTarget();

    switch (GetId())
    {
        // some spell have charges by functionality not have its in spell data
        case 28200:                                         // Ascendance (Talisman of Ascendance trinket)
            if (apply)
                GetHolder()->SetAuraCharges(6);
            break;
        case 50720:                                         // Vigilance (threat transfering)
            if (apply)
            {
                if (Unit* caster = GetCaster())
                    target->CastSpell(caster, 59665, true);
            }
            else
                target->getHostileRefManager().ResetThreatRedirection();
            break;
        case 72059:                                         // Unstable (Kinetic Bomb - Blood Council encounter)
            if (!apply)
            {
                if (target->GetTypeId() == TYPEID_UNIT)
                    ((Creature*)target)->ForcedDespawn();
            }
            break;
        case 72451:                                         // Mutated Plague (Putricide)
        case 72463:
        case 72671:
        case 72672:
            if (!apply)
            {
                if (Unit *pCaster = GetCaster())
                {
                    if (pCaster->isAlive())
                        target->CastSpell(pCaster, GetModifier()->m_amount, true); // cast healing spell
                }
            }
            break;
        default:
            break;
    }
}

void Aura::HandleAuraModStalked(bool apply, bool /*Real*/)
{
    // used by spells: Hunter's Mark, Mind Vision, Syndicate Tracker (MURP) DND
    if (apply)
        GetTarget()->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
    else
        GetTarget()->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
}

/*********************************************************/
/***                   PERIODIC                        ***/
/*********************************************************/

void Aura::HandlePeriodicTriggerSpell(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;

    Unit *target = GetTarget();

    if (!apply)
    {
        switch(GetId())
        {
            case 66:                                        // Invisibility
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE)
                    target->CastSpell(target, 32612, true, NULL, this);

                return;
            case 28522:                                     // Icebolt (Naxxramas: Sapphiron)
                if (target->HasAura(45776))                 // Should trigger/remove some kind of iceblock
                    // not sure about ice block spell id
                    target->RemoveAurasDueToSpell(45776);

                return;
            case 42783:                                     // Wrath of the Astrom...
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE && GetEffIndex() + 1 < MAX_EFFECT_INDEX)
                    target->CastSpell(target, GetSpellProto()->CalculateSimpleValue(SpellEffectIndex(GetEffIndex()+1)), true);

                return;
            case 46221:                                     // Animal Blood
                if (target->GetTypeId() == TYPEID_PLAYER && m_removeMode == AURA_REMOVE_BY_DEFAULT && target->IsInWater())
                {
                    float position_z = target->GetTerrain()->GetWaterLevel(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                    // Spawn Blood Pool
                    target->CastSpell(target->GetPositionX(), target->GetPositionY(), position_z, 63471, true);
                }

                return;
            case 51121:                                     // Urom Clocking Bomb
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE)
                    target->CastSpell(target, 51132, true);
                return;
            case 51912:                                     // Ultra-Advanced Proto-Typical Shortening Blaster
            case 53102:                                     // Scepter of Domination
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE)
                {
                    if (Unit* pCaster = GetCaster())
                        pCaster->CastSpell(target, GetSpellProto()->EffectTriggerSpell[GetEffIndex()], true, NULL, this);
                }

                return;
            case 52658:                                     // Ionar Static Overload Explode (N)
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE)
                    target->CastSpell(target, 53337, true);

                return;
            case 59795:                                     // Ionar Static Overload Explode (H)
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE)
                    target->CastSpell(target, 59798, true);

                return;
            case 63018:                                     // Searing Light (Ulduar: XT-002)
            case 65121:                                     // Searing Light (h) (Ulduar: XT-002)
                if (Unit *pCaster = GetCaster())
                {
                    if (pCaster->HasAura(GetModifier()->m_amount))
                        pCaster->CastSpell(target, 64210, true);
                }

                return;
            case 63024:                                     // Gravity Bomb (Ulduar: XT-002)
            case 64234:                                     // Gravity Bomb (h) (Ulduar: XT-002)
                if (Unit *pCaster = GetCaster())
                {
                    uint32 spellId = GetId() == 63024 ? 64203 : 64235;
                    if (pCaster->HasAura(GetModifier()->m_amount))
                        pCaster->CastSpell(target, spellId, true);
                }

                return;
            case 66083:                                     // Lightning Arrows (Trial of the Champion encounter)
                if (m_removeMode == AURA_REMOVE_BY_EXPIRE)
                {
                    if (Unit* pCaster = GetCaster())
                        pCaster->CastSpell(pCaster, 66085, true, NULL, this);
                }

                return;
            case 70405:                                     // Mutated Transformation (Putricide)
            case 72508:
            case 72509:
            case 72510:
                if (target->GetTypeId() == TYPEID_UNIT)
                    ((Creature*)target)->ForcedDespawn();
                return;
            case 71441:                                     // Unstable Ooze Explosion (Rotface)
                target->CastSpell(target, 67375, true);
                return;
            default:
                break;
        }
    }

    switch (GetId())
    {
        case 70157:                                     // Ice Tomb (Sindragosa)
        {
            if (apply)
            {
                if (GameObject *pGO = target->SummonGameobject(201722, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, 180))
                {
                    pGO->SetSpellId(GetId());
                    target->AddGameObject(pGO);
                }
                if (Creature *pCreature = target->SummonCreature(36980, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 180000))
                {
                    pCreature->SetCreatorGuid(target->GetObjectGuid());
                }
            }
            else
            {
                if (GameObject *pGo = target->GetGameObject(GetId()))
                    pGo->Delete();
            }

            return;
        }
        case 71530:                                     // Essence of the Blood Queen (Queen Lana'thel)
        case 71531:
        case 71532:
        case 71533:
        case 71525:
        case 71473:
        case 70867:
        case 70879:
        case 71265:                                     // Swarming Shadows (Queen Lana'thel)
        {
            if (apply)
            {
                target->CastSpell(target, 70871, true, 0, this, target->GetObjectGuid()); // add the buff for healing

                if (Unit *pCaster = GetCaster())
                {
                    // if we were bitten then we remove Frenzied Bloodthirst aura
                    SpellAuraHolderPtr holder = pCaster->GetSpellAuraHolder(70877);
                    if (!holder)
                        holder = pCaster->GetSpellAuraHolder(71474);

                    if (holder)
                    {
                        pCaster->RemoveAurasDueToSpell(70877);
                        pCaster->CastSpell(pCaster, GetId(), true, 0, 0, holder->GetCasterGuid());
                    }
                }
            }
            else
            {
                target->RemoveAurasDueToSpell(70871); // remove the buff
            }
            break;
        }
    }
}

void Aura::HandlePeriodicEnergize(bool apply, bool Real)
{
    if (!Real)
        return;

    Unit *target = GetTarget();

    // For prevent double apply bonuses
    bool loading = (target->GetTypeId() == TYPEID_PLAYER && ((Player*)target)->GetSession()->PlayerLoading());

    if (apply && !loading)
    {
        switch (GetId())
        {
            case 29166:                                     // Innervate (value% of casters base mana)
            {
                if (Unit* caster = GetCaster())
                {
                    m_modifier.m_amount = int32(caster->GetCreateMana() * GetBasePoints() / (100 * GetAuraMaxTicks()));
                }
                break;
            }
            case 48391:                                     // Owlkin Frenzy 2% base mana
                m_modifier.m_amount = target->GetCreateMana() * 2 / 100;
                break;
            case 57669:                                     // Replenishment (0.2% from max)
                m_modifier.m_amount = target->GetMaxPower(POWER_MANA) * 2 / 1000;
                break;
            case 61782:                                     // Infinite Replenishment (0.25% from max)
                m_modifier.m_amount = target->GetMaxPower(POWER_MANA) * 25 / 10000;
                break;
            default:
                break;
        }
    }
    if (!apply && !loading)
    {
        switch (GetId())
        {
            case 5229:                                      // Druid Bear Enrage
                if (target->HasAura(51185))               // King of the Jungle self Enrage bonus with infinity duration
                    target->RemoveAurasDueToSpell(51185);
                break;
            default:
                break;
        }
    }

    m_isPeriodic = apply;
}

void Aura::HandleAuraPowerBurn(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;
}

void Aura::HandlePeriodicHeal(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;

    Unit *target = GetTarget();

    // For prevent double apply bonuses
    bool loading = (target->GetTypeId() == TYPEID_PLAYER && ((Player*)target)->GetSession()->PlayerLoading());

    // Custom damage calculation after
    if (apply)
    {
        if (loading)
            return;

        Unit *caster = GetCaster();
        if (!caster)
            return;

        // Gift of the Naaru (have multiple spellfamilies)
        if (GetSpellProto()->GetSpellFamilyFlags().test<CF_ALL_GIFT_OF_THE_NAARU>())
        {
            float add = 0.0f;
            switch (GetSpellProto()->SpellFamilyName)
            {
                case SPELLFAMILY_MAGE:
                case SPELLFAMILY_WARLOCK:
                case SPELLFAMILY_PRIEST:
                    add = 1.885f * (float)caster->SpellBaseDamageBonusDone(GetSpellSchoolMask(GetSpellProto()));
                    break;

                case SPELLFAMILY_PALADIN:
                case SPELLFAMILY_SHAMAN:
                    add = std::max(1.885f * (float)caster->SpellBaseDamageBonusDone(GetSpellSchoolMask(GetSpellProto())), 1.1f * (float)caster->GetTotalAttackPowerValue(BASE_ATTACK));
                    break;

                case SPELLFAMILY_WARRIOR:
                case SPELLFAMILY_HUNTER:
                    add = 1.1f * (float)std::max(caster->GetTotalAttackPowerValue(BASE_ATTACK), caster->GetTotalAttackPowerValue(RANGED_ATTACK));
                    break;

                case SPELLFAMILY_GENERIC:
                default:
                    sLog.outError("Aura::HandlePeriodicHeal unknown type of aura %u",GetId());
                    break;
            }

            int32 add_per_tick = floor(add / GetAuraMaxTicks());
            m_modifier.m_amount += (add_per_tick > 0 ? add_per_tick : 0);
        }
        // Lifeblood
        else if (GetSpellProto()->SpellIconID == 3088 && GetSpellProto()->SpellVisual == 8145)
        {
            int32 healthBonus = int32 (0.0032f * caster->GetMaxHealth());
            m_modifier.m_amount += healthBonus;
        }
        else
        {
            m_modifier.m_amount = caster->SpellHealingBonusDone(target, GetSpellProto(), m_modifier.m_amount, DOT, GetStackAmount());
        }

        // Rejuvenation
        if (GetSpellProto()->IsFitToFamily<SPELLFAMILY_DRUID, CF_DRUID_REJUVENATION>())
            if (caster->HasAura(64760))                     // Item - Druid T8 Restoration 4P Bonus
                caster->CastCustomSpell(target, 64801, &m_modifier.m_amount, NULL, NULL, true, NULL);
    }
}

void Aura::HandlePeriodicDamage(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    m_isPeriodic = apply;

    Unit *target = GetTarget();
    SpellEntry const* spellProto = GetSpellProto();

    // For prevent double apply bonuses
    bool loading = (target->GetTypeId() == TYPEID_PLAYER && ((Player*)target)->GetSession()->PlayerLoading());

    // Custom damage calculation after
    if (apply)
    {
        if (loading)
            return;

        Unit *caster = GetCaster();
        if (!caster)
            return;

        switch (spellProto->SpellFamilyName)
        {
            case SPELLFAMILY_WARRIOR:
            {
                // Rend
                if (spellProto->GetSpellFamilyFlags().test<CF_WARRIOR_REND>())
                {
                    // $0.2*(($MWB+$mwb)/2+$AP/14*$MWS) bonus per tick
                    float ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);
                    int32 mws = caster->GetAttackTime(BASE_ATTACK);
                    float mwb_min = caster->GetWeaponDamageRange(BASE_ATTACK,MINDAMAGE);
                    float mwb_max = caster->GetWeaponDamageRange(BASE_ATTACK,MAXDAMAGE);
                    m_modifier.m_amount+=int32(((mwb_min+mwb_max)/2+ap*mws/14000)*0.2f);
                    // If used while target is above 75% health, Rend does 35% more damage
                    if (spellProto->CalculateSimpleValue(EFFECT_INDEX_1) !=0 &&
                        target->GetHealth() > target->GetMaxHealth() * spellProto->CalculateSimpleValue(EFFECT_INDEX_1) / 100)
                        m_modifier.m_amount += m_modifier.m_amount * spellProto->CalculateSimpleValue(EFFECT_INDEX_2) / 100;

                    // Improved Rend - Rank 1
                    if (caster->HasAura(12286))
                        m_modifier.m_amount += int32(m_modifier.m_amount * 0.1f);
                    // Improved Rend - Rank 2
                    if (caster->HasAura(12658))
                        m_modifier.m_amount += int32(m_modifier.m_amount * 0.2f);
                }
                break;
            }
            case SPELLFAMILY_DRUID:
            {
                // Rip
                if (spellProto->GetSpellFamilyFlags().test<CF_DRUID_RIP_BITE>())
                {
                    if (caster->GetTypeId() != TYPEID_PLAYER)
                        break;

                    // 0.01*$AP*cp
                    uint8 cp = caster->GetComboPoints();

                    // Idol of Feral Shadows. Cant be handled as SpellMod in SpellAura:Dummy due its dependency from CPs
                    Unit::AuraList const& dummyAuras = caster->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraList::const_iterator itr = dummyAuras.begin(); itr != dummyAuras.end(); ++itr)
                    {
                        if((*itr)->GetId()==34241)
                        {
                            m_modifier.m_amount += cp * (*itr)->GetModifier()->m_amount;
                            break;
                        }
                    }
                    m_modifier.m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * cp / 100);
                }
                // Insect Swarm
                else if (spellProto->GetSpellFamilyFlags().test<CF_DRUID_INSECT_SWARM>())
                {
                    // Idol of the Crying Wind
                    Unit::AuraList const& dummyAuras = caster->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraList::const_iterator itr = dummyAuras.begin(); itr != dummyAuras.end(); ++itr)
                    {
                        if((*itr)->GetId()==64950)
                        {
                            m_modifier.m_amount += (*itr)->GetModifier()->m_amount;
                            break;
                        }
                    }
                }
                break;
            }
            case SPELLFAMILY_ROGUE:
            {
                // Rupture
                if (spellProto->GetSpellFamilyFlags().test<CF_ROGUE_RUPTURE>())
                {
                    if (caster->GetTypeId() != TYPEID_PLAYER)
                        break;
                    //1 point : ${($m1+$b1*1+0.015*$AP)*4} damage over 8 secs
                    //2 points: ${($m1+$b1*2+0.024*$AP)*5} damage over 10 secs
                    //3 points: ${($m1+$b1*3+0.03*$AP)*6} damage over 12 secs
                    //4 points: ${($m1+$b1*4+0.03428571*$AP)*7} damage over 14 secs
                    //5 points: ${($m1+$b1*5+0.0375*$AP)*8} damage over 16 secs
                    float AP_per_combo[6] = {0.0f, 0.015f, 0.024f, 0.03f, 0.03428571f, 0.0375f};
                    uint8 cp = caster->GetComboPoints();
                    if (cp > 5) cp = 5;
                    m_modifier.m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * AP_per_combo[cp]);
                }
                break;
            }
            case SPELLFAMILY_PALADIN:
            {
                // Holy Vengeance / Blood Corruption
                if (spellProto->GetSpellFamilyFlags().test<CF_PALADIN_SEAL_OF_CORRUPT_VENGE>() && spellProto->SpellVisual == 7902)
                {
                    // AP * 0.025 + SPH * 0.013 bonus per tick
                    float ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);
                    int32 holy = caster->SpellBaseDamageBonusDone(GetSpellSchoolMask(spellProto));
                    if (holy < 0)
                        holy = 0;
                    m_modifier.m_amount += int32(GetStackAmount()) * (int32(ap * 0.025f) + int32(holy * 13 / 1000));
                }
                break;
            }
            default:
                break;
        }

        if (m_modifier.m_auraname == SPELL_AURA_PERIODIC_DAMAGE)
        {
            DamageInfo damageInfo =  DamageInfo(caster, target, GetSpellProto(), m_modifier.m_amount);
            damageInfo.damageType = DOT;
            // SpellDamageBonusDone for magic spells
            if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE || spellProto->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)
            {
                caster->SpellDamageBonusDone(&damageInfo, GetStackAmount());
            }
            // MeleeDamagebonusDone for weapon based spells
            else
            {
                caster->MeleeDamageBonusDone(&damageInfo, GetStackAmount());
            }
            m_modifier.m_amount = damageInfo.damage;
        }
    }
    // remove time effects
    else
    {
        // Parasitic Shadowfiend - handle summoning of two Shadowfiends on DoT expire
        if (spellProto->Id == 41917)
            target->CastSpell(target, 41915, true);
        else if (spellProto->Id == 74562) // SPELL_FIERY_COMBUSTION - Ruby sanctum boss Halion
            target->CastSpell(target, 74607, true, NULL, NULL, GetCasterGuid());
        else if (spellProto->Id == 74792) // SPELL_SOUL_CONSUMPTION - Ruby sanctum boss Halion
            target->CastSpell(target, 74799, true, NULL, NULL, GetCasterGuid());
        // Void Shifted
        else if (spellProto->Id == 54361 || spellProto->Id == 59743)
            target->CastSpell(target, 54343, true, NULL, NULL, GetCasterGuid());
    }

    switch (GetId())
    {
        case 68980:                             // Harvest Soul(s) (Lich King)
        case 74295:
        case 74296:
        case 74297:
        case 74325:
        case 74326:
        case 74327:
        case 73654:
        {
            if (!apply)
            {
                // if died - cast Harvested Soul on Lich King
                if (m_removeMode == AURA_REMOVE_BY_DEATH)
                    target->CastSpell(target, 72679, true);
            }

            break;
        }
        case 70911:                             // Unbound Plague (Putricide)
        case 72854:
        case 72855:
        case 72856:
        {
            if (apply)
            {
                target->CastSpell(target, 70955, true); // Bounce Protection
                if (Unit *pCaster = GetCaster())
                {
                    if (SpellAuraHolderPtr holder = pCaster->GetSpellAuraHolder(GetId()))
                    {
                        GetHolder()->SetAuraDuration(holder->GetAuraDuration());
                        GetHolder()->SendAuraUpdate(false);
                    }
                }
            }
            else
            {
                target->RemoveAurasDueToSpell(70917); // remove Search Periodic
                target->CastSpell(target, 70953, true); // Plague Sickness
            }

            break;
        }
    }
}

void Aura::HandlePeriodicDamagePCT(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;
}

void Aura::HandlePeriodicLeech(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;

    // For prevent double apply bonuses
    bool loading = (GetTarget()->GetTypeId() == TYPEID_PLAYER && ((Player*)GetTarget())->GetSession()->PlayerLoading());

    // Custom damage calculation after
    if (apply)
    {
        if (loading)
            return;

        Unit *caster = GetCaster();
        if (!caster)
            return;

        DamageInfo damageInfo =  DamageInfo(caster, GetTarget(), GetSpellProto(), m_modifier.m_amount);
        damageInfo.damageType = DOT;
        caster->SpellDamageBonusDone(&damageInfo, GetStackAmount());
        m_modifier.m_amount = damageInfo.damage;
    }
}

void Aura::HandlePeriodicManaLeech(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;
}

void Aura::HandlePeriodicHealthFunnel(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;

    // For prevent double apply bonuses
    bool loading = (GetTarget()->GetTypeId() == TYPEID_PLAYER && ((Player*)GetTarget())->GetSession()->PlayerLoading());

    // Custom damage calculation after
    if (apply)
    {
        if (loading)
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;
        DamageInfo damageInfo =  DamageInfo(caster, GetTarget(), GetSpellProto(), m_modifier.m_amount);
        damageInfo.damageType = DOT;
        caster->SpellDamageBonusDone(&damageInfo, GetStackAmount());
        m_modifier.m_amount = damageInfo.damage;
    }
}

/*********************************************************/
/***                  MODIFY STATS                     ***/
/*********************************************************/

/********************************/
/***        RESISTANCE        ***/
/********************************/

void Aura::HandleAuraModResistanceExclusive(bool apply, bool /*Real*/)
{
    Unit *target = GetTarget();

    for(int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL;x++)
    {
        //int32 oldMaxValue = 0;
        if (m_modifier.m_miscvalue & int32(1<<x))
        {
            float change = target->CheckAuraStackingAndApply(this, UnitMods(UNIT_MOD_RESISTANCE_START + x), TOTAL_VALUE, float(m_modifier.m_amount), apply, int32(1<<x));
            if (change != 0)
                target->ApplyResistanceBuffModsMod(SpellSchools(x), m_modifier.m_amount > 0, change, true);
        }
    }
}

void Aura::HandleAuraModResistance(bool apply, bool /*Real*/)
{
    Unit *target = GetTarget();

    for(int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL;x++)
    {
        if (m_modifier.m_miscvalue & int32(1<<x))
        {
            float change = target->CheckAuraStackingAndApply(this, UnitMods(UNIT_MOD_RESISTANCE_START + x), TOTAL_VALUE, float(m_modifier.m_amount), apply);
            if (change != 0)
                target->ApplyResistanceBuffModsMod(SpellSchools(x), m_modifier.m_amount > 0, float(m_modifier.m_amount), apply);
        }
    }
}

void Aura::HandleAuraModBaseResistancePCT(bool apply, bool /*Real*/)
{
    // only players and pets have base stats
    if (GetTarget()->GetTypeId() == TYPEID_PLAYER || ((Creature*)GetTarget())->IsPet())
    {
        for(int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL;x++)
        {
            if (m_modifier.m_miscvalue & int32(1<<x))
                GetTarget()->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_PCT, float(m_modifier.m_amount), apply);
        }
    }
}

void Aura::HandleModResistancePercent(bool apply, bool /*Real*/)
{
    Unit *target = GetTarget();

    for(int8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
    {
        if (m_modifier.m_miscvalue & int32(1<<i))
        {
            float change = target->CheckAuraStackingAndApply(this, UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_PCT, float(m_modifier.m_amount), apply, int32(1<<i));
            if (change)
                target->ApplyResistanceBuffModsPercentMod(SpellSchools(i), m_modifier.m_amount > 0, change, apply);
        }
    }
}

void Aura::HandleModBaseResistance(bool apply, bool /*Real*/)
{
    // only players and pets have base stats
    if (GetTarget()->GetTypeId() == TYPEID_PLAYER  || ((Creature*)GetTarget())->IsPet())
    {
        for(int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
            if (m_modifier.m_miscvalue & (1<<i))
                GetTarget()->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, float(m_modifier.m_amount), apply);
    }
}

/********************************/
/***           STAT           ***/
/********************************/

void Aura::HandleAuraModStat(bool apply, bool /*Real*/)
{
    if (m_modifier.m_miscvalue < -2 || m_modifier.m_miscvalue > 4)
    {
        sLog.outError("WARNING: Spell %u effect %u have unsupported misc value (%i) for SPELL_AURA_MOD_STAT ",GetId(),GetEffIndex(),m_modifier.m_miscvalue);
        return;
    }

    Unit *target = GetTarget();

    for(int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        // -1 or -2 is all stats ( misc < -2 checked in function beginning )
        if (m_modifier.m_miscvalue < 0 || m_modifier.m_miscvalue == i)
        {
            //m_target->ApplyStatMod(Stats(i), m_modifier.m_amount,apply);
            float change = target->CheckAuraStackingAndApply(this, UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, float(m_modifier.m_amount), apply, 0, i+1);
            if (change != 0)
                target->ApplyStatBuffMod(Stats(i), (change < 0 && !IsStacking() ? -change : change), apply);
        }
    }
}

void Aura::HandleModPercentStat(bool apply, bool /*Real*/)
{
    if (m_modifier.m_miscvalue < -1 || m_modifier.m_miscvalue > 4)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_PERCENT_STAT not valid");
        return;
    }

    // only players and pets have base stats
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER && !((Creature*)GetTarget())->IsPet())
        return;

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        if (m_modifier.m_miscvalue == i || m_modifier.m_miscvalue == -1)
            GetTarget()->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), BASE_PCT, float(m_modifier.m_amount), apply);
    }
}

void Aura::HandleModSpellDamagePercentFromStat(bool /*apply*/, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonusDone
    // This information for client side use only
    // Recalculate bonus
    ((Player*)GetTarget())->UpdateSpellDamageAndHealingBonus();
}

void Aura::HandleModSpellHealingPercentFromStat(bool /*apply*/, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    ((Player*)GetTarget())->UpdateSpellDamageAndHealingBonus();
}

void Aura::HandleModHealingDone(bool /*apply*/, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;
    // implemented in Unit::SpellHealingBonusDone
    // this information is for client side only
    ((Player*)GetTarget())->UpdateSpellDamageAndHealingBonus();
}

void Aura::HandleModTotalPercentStat(bool apply, bool /*Real*/)
{
    if (m_modifier.m_miscvalue < -1 || m_modifier.m_miscvalue > 4)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_PERCENT_STAT not valid");
        return;
    }

    Unit *target = GetTarget();

    //save current and max HP before applying aura
    uint32 curHPValue = target->GetHealth();
    uint32 maxHPValue = target->GetMaxHealth();

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        if (m_modifier.m_miscvalue == i || m_modifier.m_miscvalue == -1)
        {
            target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT, float(m_modifier.m_amount), apply);
            if (target->GetTypeId() == TYPEID_PLAYER || ((Creature*)target)->IsPet())
                target->ApplyStatPercentBuffMod(Stats(i), float(m_modifier.m_amount), apply );
        }
    }

    //recalculate current HP/MP after applying aura modifications (only for spells with 0x10 flag)
    if (m_modifier.m_miscvalue == STAT_STAMINA && maxHPValue > 0 && GetSpellProto()->HasAttribute(SPELL_ATTR_ABILITY))
    {
        // newHP = (curHP / maxHP) * newMaxHP = (newMaxHP * curHP) / maxHP -> which is better because no int -> double -> int conversion is needed
        // Multiplication of large numbers cause uint32 overflow so using trick
        // a*b/c = (a/c) * (b/c) * c + (a%c) * (b%c) / c + (a/c) * (b%c) + (a%c) * (b/c)
        uint32 max_hp = target->GetMaxHealth();
        // max_hp * curHPValue / maxHPValue
        uint32 newHPValue =
            (max_hp/maxHPValue) * (curHPValue/maxHPValue) * maxHPValue
            + (max_hp%maxHPValue) * (curHPValue%maxHPValue) / maxHPValue
            + (max_hp/maxHPValue) * (curHPValue%maxHPValue)
            + (max_hp%maxHPValue) * (curHPValue/maxHPValue);
        target->SetHealth(newHPValue);
    }
}

void Aura::HandleAuraModResistenceOfIntellectPercent(bool /*apply*/, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    if (m_modifier.m_miscvalue != SPELL_SCHOOL_MASK_NORMAL)
    {
        // support required adding replace UpdateArmor by loop by UpdateResistence at intellect update
        // and include in UpdateResistence same code as in UpdateArmor for aura mod apply.
        sLog.outError("Aura SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT(182) need adding support for non-armor resistances!");
        return;
    }

    // Recalculate Armor
    GetTarget()->UpdateArmor();
}

/********************************/
/***      HEAL & ENERGIZE     ***/
/********************************/
void Aura::HandleAuraModTotalHealthPercentRegen(bool apply, bool /*Real*/)
{
    m_isPeriodic = apply;
}

void Aura::HandleAuraModTotalManaPercentRegen(bool apply, bool /*Real*/)
{
    if (m_modifier.periodictime == 0)
        m_modifier.periodictime = 1000;

    m_periodicTimer = m_modifier.periodictime;
    m_isPeriodic = apply;
}

void Aura::HandleModRegen(bool apply, bool /*Real*/)        // eating
{
    if (m_modifier.periodictime == 0)
        m_modifier.periodictime = 5000;

    m_periodicTimer = 5000;
    m_isPeriodic = apply;
}

void Aura::HandleModPowerRegen(bool apply, bool Real)       // drinking
{
    if (!Real)
        return;

    Powers pt = GetTarget()->getPowerType();
    if (m_modifier.periodictime == 0)
    {
        // Anger Management (only spell use this aura for rage)
        if (pt == POWER_RAGE)
            m_modifier.periodictime = 3000;
        else
            m_modifier.periodictime = 2000;
    }

    m_periodicTimer = 5000;

    if (GetTarget()->GetTypeId() == TYPEID_PLAYER && m_modifier.m_miscvalue == POWER_MANA)
        ((Player*)GetTarget())->UpdateManaRegen();

    m_isPeriodic = apply;
}

void Aura::HandleModPowerRegenPCT(bool /*apply*/, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    // Update manaregen value
    if (m_modifier.m_miscvalue == POWER_MANA)
        ((Player*)GetTarget())->UpdateManaRegen();
}

void Aura::HandleAuraModIncreaseHealth(bool apply, bool Real)
{
    Unit *target = GetTarget();

    // Special case with temporary increase max/current health
    switch(GetId())
    {
        case 12976:                                         // Warrior Last Stand triggered spell
        case 28726:                                         // Nightmare Seed ( Nightmare Seed )
        case 31616:                                         // Nature's Guardian
        case 34511:                                         // Valor (Bulwark of Kings, Bulwark of the Ancient Kings)
        case 44055: case 55915: case 55917: case 67596:     // Tremendous Fortitude (Battlemaster's Alacrity)
        case 50322:                                         // Survival Instincts
        case 53479:                                         // Hunter pet - Last Stand
        case 54443:                                         // Demonic Empowerment (Voidwalker)
        case 55233:                                         // Vampiric Blood
        case 59465:                                         // Brood Rage (Ahn'Kahet)
        {
            if (Real)
            {
                if (apply)
                {
                    // Demonic Empowerment (Voidwalker) & Vampiric Blood - special cases, store percent in data
                    // recalculate to full amount at apply for proper remove
                    if (GetId() == 54443 || GetId() == 55233)
                        m_modifier.m_amount = target->GetMaxHealth() * m_modifier.m_amount / 100;

                    target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_modifier.m_amount), apply);
                    target->ModifyHealth(m_modifier.m_amount);
                }
                else
                {
                    if (int32(target->GetHealth()) > m_modifier.m_amount)
                        target->ModifyHealth(-m_modifier.m_amount);
                    else
                        target->SetHealth(1);
                    target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_modifier.m_amount), apply);
                }
            }
            return;
        }
    }

    // generic case
    target->CheckAuraStackingAndApply(this, UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModIncreaseEnergy(bool apply, bool Real)
{
    Unit* target = GetTarget();

    if (!target)
        return;

    Powers powerType = target->getPowerType();

    if (int32(powerType) != m_modifier.m_miscvalue)
    {
        DEBUG_LOG("HandleAuraModIncreaseEnergy: unit %u change energy %u but current type %u", target->GetObjectGuid().GetCounter(), m_modifier.m_miscvalue, powerType);
        powerType = Powers(m_modifier.m_miscvalue);
    }

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);

    target->HandleStatModifier(unitMod, TOTAL_VALUE, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModIncreaseEnergyPercent(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();

    if (!target)
        return;

    Powers powerType = target->getPowerType();

    if (int32(powerType) != m_modifier.m_miscvalue)
    {
        DEBUG_LOG("HandleAuraModIncreaseEnergy: unit %u change energy %u but current type %u", target->GetObjectGuid().GetCounter(), m_modifier.m_miscvalue, powerType);
        powerType = Powers(m_modifier.m_miscvalue);
    }

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);

    target->HandleStatModifier(unitMod, TOTAL_PCT, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModIncreaseHealthPercent(bool apply, bool /*Real*/)
{
    Unit* target = GetTarget();

    if (!target)
        return;

    uint32 oldhealth = target->GetHealth();

    target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_PCT, float(m_modifier.m_amount), apply);

    // spell special cases when current health set to max value at apply
    if (apply)
    {
        switch (GetId())
        {
            case 60430:                                         // Molten Fury
            case 61254:                                         // Will of Sartharion
            case 64193:                                         // Heartbreak
            case 65737:                                         // Heartbreak
            case 64582:                                         // Emergency Mode (Ulduar - Mimiron)
                target->SetHealth(target->GetMaxHealth());
                break;
            default:
                if (oldhealth > target->GetMaxHealth())
                    target->SetHealth(target->GetMaxHealth());
                break;
        }
    }
    else
    {
        if (oldhealth > target->GetMaxHealth())
            target->SetHealth(target->GetMaxHealth());
        else
            target->SetHealth(oldhealth);
    }
}

/********************************/
/***          FIGHT           ***/
/********************************/

void Aura::HandleAuraModParryPercent(bool /*apply*/, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)GetTarget())->UpdateParryPercentage();
}

void Aura::HandleAuraModDodgePercent(bool /*apply*/, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)GetTarget())->UpdateDodgePercentage();
    //sLog.outError("BONUS DODGE CHANCE: + %f", float(m_modifier.m_amount));
}

void Aura::HandleAuraModBlockPercent(bool /*apply*/, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)GetTarget())->UpdateBlockPercentage();
    //sLog.outError("BONUS BLOCK CHANCE: + %f", float(m_modifier.m_amount));
}

void Aura::HandleAuraModRegenInterrupt(bool /*apply*/, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)GetTarget())->UpdateManaRegen();
}

void Aura::HandleAuraModCritPercent(bool apply, bool Real)
{
    Unit *target = GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    // apply item specific bonuses for already equipped weapon
    if (Real)
    {
        for(int i = 0; i < MAX_ATTACK; ++i)
            if (Item* pItem = ((Player*)target)->GetWeaponForAttack(WeaponAttackType(i),true,false))
                ((Player*)target)->_ApplyWeaponDependentAuraCritMod(pItem, WeaponAttackType(i), this, apply);
    }

    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // m_modifier.m_miscvalue comparison with item generated damage types

    float amount = float(m_modifier.m_amount);

    if (GetSpellProto()->EquippedItemClass == -1)
    {
        if(IsStacking())
        {
            ((Player*)target)->HandleBaseModValue(CRIT_PERCENTAGE,         FLAT_MOD, amount, apply);
            ((Player*)target)->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, amount, apply);
            ((Player*)target)->HandleBaseModValue(RANGED_CRIT_PERCENTAGE,  FLAT_MOD, amount, apply);
        }
        else
        {
            float current = ((Player*)target)->GetBaseModValue(NONSTACKING_CRIT_PERCENTAGE, FLAT_MOD);

            if(amount < current)
                return;

            // unapply old aura
            if(current)
            {
                ((Player*)target)->HandleBaseModValue(CRIT_PERCENTAGE,         FLAT_MOD, current, false);
                ((Player*)target)->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, current, false);
                ((Player*)target)->HandleBaseModValue(RANGED_CRIT_PERCENTAGE,  FLAT_MOD, current, false);
            }

            if(!apply)
                amount = target->GetMaxPositiveAuraModifier(SPELL_AURA_MOD_CRIT_PERCENT, true);

            if(amount)
            {
                ((Player*)target)->HandleBaseModValue(CRIT_PERCENTAGE,         FLAT_MOD, amount, true);
                ((Player*)target)->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, amount, true);
                ((Player*)target)->HandleBaseModValue(RANGED_CRIT_PERCENTAGE,  FLAT_MOD, amount, true);
            }

            ((Player*)target)->SetBaseModValue(NONSTACKING_CRIT_PERCENTAGE, FLAT_MOD, amount);
        }
    }
    else
    {
        // done in Player::_ApplyWeaponDependentAuraMods
    }
}

void Aura::HandleModHitChance(bool apply, bool /*Real*/)
{
    Unit *target = GetTarget();

    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        // stacking check is made further
        ((Player*)target)->UpdateMeleeHitChances();
        ((Player*)target)->UpdateRangedHitChances();
    }
    else
    {
        target->m_modMeleeHitChance = target->GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
        target->m_modRangedHitChance = target->GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
    }
}

void Aura::HandleModSpellHitChance(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)GetTarget())->UpdateSpellHitChances();
    }
    else
    {
        GetTarget()->m_modSpellHitChance += apply ? m_modifier.m_amount: (-m_modifier.m_amount);
    }
}

void Aura::HandleModSpellCritChance(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    if (GetTarget()->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)GetTarget())->UpdateAllSpellCritChances();
    }
    else
    {
        GetTarget()->m_baseSpellCritChance += apply ? m_modifier.m_amount:(-m_modifier.m_amount);
    }
}

void Aura::HandleModSpellCritChanceShool(bool /*apply*/, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    for(int school = SPELL_SCHOOL_NORMAL; school < MAX_SPELL_SCHOOL; ++school)
        if (m_modifier.m_miscvalue & (1<<school))
            ((Player*)GetTarget())->UpdateSpellCritChance(school);
}

/********************************/
/***         ATTACK SPEED     ***/
/********************************/

void Aura::HandleModCastingSpeed(bool apply, bool /*Real*/)
{
    Unit *target = GetTarget();

    if(IsStacking())
        target->ApplyCastTimePercentMod(float(m_modifier.m_amount),apply);
    else
    {
        float amount = float(m_modifier.m_amount);
        bool bIsPositive = amount > 0;

        // don't apply weaker aura
        if((bIsPositive && amount < target->m_modSpellSpeedPctPos) ||
            (!bIsPositive && amount > target->m_modSpellSpeedPctNeg))
        {
            return;
        }

        // unapply old (weaker) aura
        if (bIsPositive)
        {
            if(target->m_modSpellSpeedPctPos)
                target->ApplyCastTimePercentMod(target->m_modSpellSpeedPctPos, false);
        }
        else
        {
            if(target->m_modSpellSpeedPctNeg)
                target->ApplyCastTimePercentMod(target->m_modSpellSpeedPctNeg, false);
        }

        if(!apply)
        {
            if (bIsPositive)
                amount = target->GetMaxPositiveAuraModifier(m_modifier.m_auraname, true);
            else
                amount = target->GetMaxNegativeAuraModifier(m_modifier.m_auraname, true);
        }

        target->ApplyCastTimePercentMod(amount, true);

        if (bIsPositive)
            target->m_modSpellSpeedPctPos = amount;
        else
            target->m_modSpellSpeedPctNeg = amount;
    }
}

void Aura::HandleModAttackSpeed(bool apply, bool /*Real*/)
{
    GetTarget()->ApplyAttackTimePercentMod(BASE_ATTACK,float(m_modifier.m_amount),apply);
}

void Aura::HandleModMeleeSpeedPct(bool apply, bool /*Real*/)
{
    Unit *target = GetTarget();

    if (IsStacking())
    {
        target->ApplyAttackTimePercentMod(BASE_ATTACK, m_modifier.m_amount, apply);
        target->ApplyAttackTimePercentMod(OFF_ATTACK, m_modifier.m_amount, apply);
    }
    else
    {
        bool bIsPositive = m_modifier.m_amount > 0;

        if((bIsPositive && m_modifier.m_amount < target->m_modAttackSpeedPct[NONSTACKING_POS_MOD_MELEE]) ||
            (!bIsPositive && m_modifier.m_amount > target->m_modAttackSpeedPct[NONSTACKING_NEG_MOD_MELEE]))
            return;

        float amount = float(m_modifier.m_amount);

        // unapply old aura
        if(target->m_modAttackSpeedPct[bIsPositive ? NONSTACKING_POS_MOD_MELEE : NONSTACKING_NEG_MOD_MELEE])
        {
            target->ApplyAttackTimePercentMod(BASE_ATTACK, target->m_modAttackSpeedPct[bIsPositive ? NONSTACKING_POS_MOD_MELEE : NONSTACKING_NEG_MOD_MELEE], false);
            target->ApplyAttackTimePercentMod(OFF_ATTACK, target->m_modAttackSpeedPct[bIsPositive ? NONSTACKING_POS_MOD_MELEE : NONSTACKING_NEG_MOD_MELEE], false);
        }

        if(!apply)
        {
            if (bIsPositive)
                amount = target->GetMaxPositiveAuraModifier(SPELL_AURA_MOD_MELEE_HASTE, true);
            else
                amount = target->GetMaxNegativeAuraModifier(SPELL_AURA_MOD_MELEE_HASTE, true);
        }

        target->ApplyAttackTimePercentMod(BASE_ATTACK, amount, true);
        target->ApplyAttackTimePercentMod(OFF_ATTACK, amount, true);

        target->m_modAttackSpeedPct[bIsPositive ? NONSTACKING_POS_MOD_MELEE : NONSTACKING_NEG_MOD_MELEE] = amount;
    }
}

void Aura::HandleAuraModRangedHaste(bool apply, bool /*Real*/)
{
    GetTarget()->ApplyAttackTimePercentMod(RANGED_ATTACK, float(m_modifier.m_amount), apply);
}

void Aura::HandleRangedAmmoHaste(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;
    GetTarget()->ApplyAttackTimePercentMod(RANGED_ATTACK, float(m_modifier.m_amount), apply);
}

/********************************/
/***        ATTACK POWER      ***/
/********************************/

void Aura::HandleAuraModAttackPower(bool apply, bool /*Real*/)
{
    GetTarget()->CheckAuraStackingAndApply(this, UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModRangedAttackPower(bool apply, bool /*Real*/)
{
    if ((GetTarget()->getClassMask() & CLASSMASK_WAND_USERS)!=0)
        return;

    GetTarget()->CheckAuraStackingAndApply(this, UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModAttackPowerPercent(bool apply, bool /*Real*/)
{
    //UNIT_FIELD_ATTACK_POWER_MULTIPLIER = multiplier - 1
    GetTarget()->CheckAuraStackingAndApply(this, UNIT_MOD_ATTACK_POWER, TOTAL_PCT, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraModRangedAttackPowerPercent(bool apply, bool /*Real*/)
{
    if ((GetTarget()->getClassMask() & CLASSMASK_WAND_USERS)!=0)
        return;

    //UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER = multiplier - 1
    GetTarget()->CheckAuraStackingAndApply(this, UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_PCT, float(m_modifier.m_amount), apply);
}

/********************************/
/***        DAMAGE BONUS      ***/
/********************************/
void Aura::HandleModDamageDone(bool apply, bool Real)
{
    Unit *target = GetTarget();

    // apply item specific bonuses for already equipped weapon
    if (Real && target->GetTypeId() == TYPEID_PLAYER)
    {
        for(int i = 0; i < MAX_ATTACK; ++i)
            if (Item* pItem = ((Player*)target)->GetWeaponForAttack(WeaponAttackType(i),true,false))
                ((Player*)target)->_ApplyWeaponDependentAuraDamageMod(pItem, WeaponAttackType(i), this, apply);
    }

    // m_modifier.m_miscvalue is bitmask of spell schools
    // 1 ( 0-bit ) - normal school damage (SPELL_SCHOOL_MASK_NORMAL)
    // 126 - full bitmask all magic damages (SPELL_SCHOOL_MASK_MAGIC) including wands
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // m_modifier.m_miscvalue comparison with item generated damage types

    if ((m_modifier.m_miscvalue & SPELL_SCHOOL_MASK_NORMAL) != 0)
    {
        // apply generic physical damage bonuses including wand case
        if (GetSpellProto()->EquippedItemClass == -1 || target->GetTypeId() != TYPEID_PLAYER)
        {
            target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, float(m_modifier.m_amount), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, float(m_modifier.m_amount), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_VALUE, float(m_modifier.m_amount), apply);
        }
        else
        {
            // done in Player::_ApplyWeaponDependentAuraMods
        }

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            if (m_positive)
                target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS, m_modifier.m_amount, apply);
            else
                target->ApplyModInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG, m_modifier.m_amount, apply);
        }
    }

    // Skip non magic case for speedup
    if ((m_modifier.m_miscvalue & SPELL_SCHOOL_MASK_MAGIC) == 0)
        return;

    if (GetSpellProto()->EquippedItemClass != -1 || GetSpellProto()->EquippedItemInventoryTypeMask != 0)
    {
        // wand magic case (skip generic to all item spell bonuses)
        // done in Player::_ApplyWeaponDependentAuraMods

        // Skip item specific requirements for not wand magic damage
        return;
    }

    // Magic damage modifiers implemented in Unit::SpellDamageBonusDone
    // This information for client side use only
    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        if (m_positive)
        {
            for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            {
                if((m_modifier.m_miscvalue & (1<<i)) != 0)
                    target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i, m_modifier.m_amount, apply);
            }
        }
        else
        {
            for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            {
                if((m_modifier.m_miscvalue & (1<<i)) != 0)
                    target->ApplyModInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + i, m_modifier.m_amount, apply);
            }
        }

        ((Player*)target)->UpdateSpellDamageAndHealingBonus();

        Pet* pet = target->GetPet();
        if (pet)
            pet->UpdateAttackPowerAndDamage();
    }
}

void Aura::HandleModDamagePercentDone(bool apply, bool Real)
{
    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "AURA MOD DAMAGE type:%u negative:%u", m_modifier.m_miscvalue, m_positive ? 0 : 1);
    Unit *target = GetTarget();

    // apply item specific bonuses for already equipped weapon
    if (Real && target->GetTypeId() == TYPEID_PLAYER)
    {
        for(int i = 0; i < MAX_ATTACK; ++i)
            if (Item* pItem = ((Player*)target)->GetWeaponForAttack(WeaponAttackType(i),true,false))
                ((Player*)target)->_ApplyWeaponDependentAuraDamageMod(pItem, WeaponAttackType(i), this, apply);
    }

    // m_modifier.m_miscvalue is bitmask of spell schools
    // 1 ( 0-bit ) - normal school damage (SPELL_SCHOOL_MASK_NORMAL)
    // 126 - full bitmask all magic damages (SPELL_SCHOOL_MASK_MAGIC) including wand
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // m_modifier.m_miscvalue comparison with item generated damage types

    if ((m_modifier.m_miscvalue & SPELL_SCHOOL_MASK_NORMAL) != 0)
    {
        // apply generic physical damage bonuses including wand case
        if (GetSpellProto()->EquippedItemClass == -1 || target->GetTypeId() != TYPEID_PLAYER)
        {
            target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, float(m_modifier.m_amount), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, float(m_modifier.m_amount), apply);
            target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_PCT, float(m_modifier.m_amount), apply);
        }
        else
        {
            // done in Player::_ApplyWeaponDependentAuraMods
        }
        // For show in client
        if (target->GetTypeId() == TYPEID_PLAYER)
            target->ApplyModSignedFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT, m_modifier.m_amount/100.0f, apply);
    }

    // Skip non magic case for speedup
    if ((m_modifier.m_miscvalue & SPELL_SCHOOL_MASK_MAGIC) == 0)
        return;

    if (GetSpellProto()->EquippedItemClass != -1 || GetSpellProto()->EquippedItemInventoryTypeMask != 0)
    {
        // wand magic case (skip generic to all item spell bonuses)
        // done in Player::_ApplyWeaponDependentAuraMods

        // Skip item specific requirements for not wand magic damage
        return;
    }

    // Magic damage percent modifiers implemented in Unit::SpellDamageBonusDone
    // Send info to client
    if (target->GetTypeId() == TYPEID_PLAYER)
        for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            target->ApplyModSignedFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT + i, m_modifier.m_amount/100.0f, apply);
}

void Aura::HandleModOffhandDamagePercent(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "AURA MOD OFFHAND DAMAGE");

    GetTarget()->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, float(m_modifier.m_amount), apply);
}

/********************************/
/***        POWER COST        ***/
/********************************/

void Aura::HandleModPowerCostPCT(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    float amount = m_modifier.m_amount/100.0f;
    for(int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (m_modifier.m_miscvalue & (1<<i))
            GetTarget()->ApplyModSignedFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + i, amount, apply);
}

void Aura::HandleModPowerCost(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    for(int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (m_modifier.m_miscvalue & (1<<i))
            GetTarget()->ApplyModInt32Value(UNIT_FIELD_POWER_COST_MODIFIER + i, m_modifier.m_amount, apply);
}

/*********************************************************/
/***                    OTHERS                         ***/
/*********************************************************/

void Aura::HandleShapeshiftBoosts(bool apply)
{
    uint32 spellId1 = 0;
    uint32 spellId2 = 0;
    uint32 HotWSpellId = 0;
    uint32 MasterShaperSpellId = 0;

    ShapeshiftForm form = ShapeshiftForm(GetModifier()->m_miscvalue);

    Unit *target = GetTarget();

    switch(form)
    {
        case FORM_CAT:
            spellId1 = 3025;
            HotWSpellId = 24900;
            MasterShaperSpellId = 48420;
            break;
        case FORM_TREE:
            spellId1 = 34123;
            MasterShaperSpellId = 48422;
            break;
        case FORM_TRAVEL:
            spellId1 = 5419;
            break;
        case FORM_AQUA:
            spellId1 = 5421;
            break;
        case FORM_BEAR:
            spellId1 = 1178;
            spellId2 = 21178;
            HotWSpellId = 24899;
            MasterShaperSpellId = 48418;
            break;
        case FORM_DIREBEAR:
            spellId1 = 9635;
            spellId2 = 21178;
            HotWSpellId = 24899;
            MasterShaperSpellId = 48418;
            break;
        case FORM_BATTLESTANCE:
            spellId1 = 21156;
            break;
        case FORM_DEFENSIVESTANCE:
            spellId1 = 7376;
            break;
        case FORM_BERSERKERSTANCE:
            spellId1 = 7381;
            break;
        case FORM_MOONKIN:
            spellId1 = 24905;
            spellId2 = 69366;
            MasterShaperSpellId = 48421;
            break;
        case FORM_SPIRITOFREDEMPTION:
            spellId1 = 27792;
            spellId2 = 27795;                               // must be second, this important at aura remove to prevent to early iterator invalidation.
            break;
        case FORM_SHADOW:
            spellId1 = 49868;
            spellId2 = 71167;
            break;
        case FORM_GHOSTWOLF:
            spellId1 = 67116;
            break;
        case FORM_AMBIENT:
        case FORM_GHOUL:
        case FORM_STEALTH:
        case FORM_CREATURECAT:
        case FORM_CREATUREBEAR:
        default:
            break;
    }

    if (apply)
    {
        if (spellId1)
        {
            if(target->GetTypeId() == TYPEID_PLAYER)
                ((Player*)target)->RemoveSpellCooldown(spellId1);
            target->CastSpell(target, spellId1, true, NULL, this );
        }
        if (spellId2)
        {
            if(target->GetTypeId() == TYPEID_PLAYER)
                ((Player*)target)->RemoveSpellCooldown(spellId2);
            target->CastSpell(target, spellId2, true, NULL, this);
        }

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            const PlayerSpellMap& sp_list = ((Player *)target)->GetSpellMap();
            for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
            {
                if (itr->second.state == PLAYERSPELL_REMOVED) continue;
                if (itr->first==spellId1 || itr->first==spellId2) continue;
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);
                if (!spellInfo || !IsNeedCastSpellAtFormApply(spellInfo, form))
                    continue;
                target->CastSpell(target, itr->first, true, NULL, this);
            }
            // remove auras that do not require shapeshift, but are not active in this specific form (like Improved Barkskin)
            Unit::SpellAuraHolderMap& tAuras = target->GetSpellAuraHolderMap();
            for (Unit::SpellAuraHolderMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
            {
                SpellEntry const *spellInfo = itr->second->GetSpellProto();
                if ((itr->second->IsPassive() && spellInfo->HasAttribute(SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT)
                    && (spellInfo->StancesNot & (1<<(form-1)))) || (spellInfo->Id == 66530 && form == FORM_DIREBEAR))  // Improved Barkskin must not affect Dire Bear Form, but does not have correct data in dbc
                {
                    target->RemoveAurasDueToSpell(itr->second->GetId());
                    itr = tAuras.begin();
                }
                else
                    ++itr;
            }


            // Master Shapeshifter
            if (MasterShaperSpellId)
            {
                Unit::AuraList const& ShapeShifterAuras = target->GetAurasByType(SPELL_AURA_DUMMY);
                for(Unit::AuraList::const_iterator i = ShapeShifterAuras.begin(); i != ShapeShifterAuras.end(); ++i)
                {
                    if ((*i)->GetSpellProto()->SpellIconID == 2851)
                    {
                        int32 ShiftMod = (*i)->GetModifier()->m_amount;
                        target->CastCustomSpell(target, MasterShaperSpellId, &ShiftMod, NULL, NULL, true);
                        break;
                    }
                }
            }

            // Leader of the Pack
            if (((Player*)target)->HasSpell(17007))
            {
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(24932);
                if (spellInfo && (spellInfo->Stances & (1<<(form-1))))
                    target->CastSpell(target, 24932, true, NULL, this);
            }

            // Savage Roar
            if (form == FORM_CAT && ((Player*)target)->HasAura(52610))
                target->CastSpell(target, 62071, true);

            // Survival of the Fittest (Armor part)
            if (form == FORM_BEAR || form == FORM_DIREBEAR)
            {
                Unit::AuraList const& modAuras = target->GetAurasByType(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
                for (Unit::AuraList::const_iterator i = modAuras.begin(); i != modAuras.end(); ++i)
                {
                    if ((*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_DRUID &&
                        (*i)->GetSpellProto()->SpellIconID == 961)
                    {
                        int32 bp = (*i)->GetSpellProto()->CalculateSimpleValue(EFFECT_INDEX_2);
                        if (bp)
                            target->CastCustomSpell(target, 62069, &bp, NULL, NULL, true, NULL, this);
                        break;
                    }
                }
            }

            // Improved Moonkin Form
            if (form == FORM_MOONKIN)
            {
                Unit::AuraList const& dummyAuras = target->GetAurasByType(SPELL_AURA_DUMMY);
                for(Unit::AuraList::const_iterator i = dummyAuras.begin(); i != dummyAuras.end(); ++i)
                {
                    if ((*i)->GetSpellProto()->SpellFamilyName==SPELLFAMILY_DRUID &&
                        (*i)->GetSpellProto()->SpellIconID == 2855)
                    {
                        uint32 spell_id = 0;
                        switch((*i)->GetId())
                        {
                            case 48384:spell_id=50170;break;//Rank 1
                            case 48395:spell_id=50171;break;//Rank 2
                            case 48396:spell_id=50172;break;//Rank 3
                            default:
                                sLog.outError("Aura::HandleShapeshiftBoosts: Not handled rank of IMF (Spell: %u)",(*i)->GetId());
                                break;
                        }

                        if (spell_id)
                            target->CastSpell(target, spell_id, true, NULL, this);
                        break;
                    }
                }
            }

            // Heart of the Wild
            if (HotWSpellId)
            {
                Unit::AuraList const& mModTotalStatPct = target->GetAurasByType(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
                for(Unit::AuraList::const_iterator i = mModTotalStatPct.begin(); i != mModTotalStatPct.end(); ++i)
                {
                    if ((*i)->GetSpellProto()->SpellIconID == 240 && (*i)->GetModifier()->m_miscvalue == 3)
                    {
                        int32 HotWMod = (*i)->GetModifier()->m_amount;
                        if (GetModifier()->m_miscvalue == FORM_CAT  || GetModifier()->m_miscvalue == FORM_BEAR || GetModifier()->m_miscvalue == FORM_DIREBEAR)
                            HotWMod /= 2;

                        target->CastCustomSpell(target, HotWSpellId, &HotWMod, NULL, NULL, true, NULL, this);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        if (spellId1)
            target->RemoveAurasDueToSpell(spellId1);
        if (spellId2)
            target->RemoveAurasDueToSpell(spellId2);
        if (MasterShaperSpellId)
            target->RemoveAurasDueToSpell(MasterShaperSpellId);

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            // re-apply passive spells that don't need shapeshift but were inactive in current form:
            const PlayerSpellMap& sp_list = ((Player *)target)->GetSpellMap();
            for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
            {
                if (itr->second.state == PLAYERSPELL_REMOVED) continue;
                if (itr->first==spellId1 || itr->first==spellId2) continue;
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);
                if (!spellInfo || !IsPassiveSpell(spellInfo))
                    continue;
                if (spellInfo->HasAttribute(SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT) && (spellInfo->StancesNot & (1<<(form-1))))
                    target->CastSpell(target, itr->first, true, NULL, this);
            }
        }

        Unit::SpellAuraHolderMap& tAuras = target->GetSpellAuraHolderMap();
        for (Unit::SpellAuraHolderMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
        {
            if (itr->second->IsRemovedOnShapeLost())
            {
                target->RemoveAurasDueToSpell(itr->second->GetId());
                itr = tAuras.begin();
            }
            else
                ++itr;
        }
    }
}

void Aura::HandleAuraEmpathy(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_UNIT)
        return;

    CreatureInfo const * ci = ObjectMgr::GetCreatureTemplate(GetTarget()->GetEntry());
    if (ci && ci->type == CREATURE_TYPE_BEAST)
        GetTarget()->ApplyModUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO, apply);
}

void Aura::HandleAuraUntrackable(bool apply, bool /*Real*/)
{
    if (apply)
        GetTarget()->SetByteFlag(UNIT_FIELD_BYTES_1, 2, UNIT_STAND_FLAGS_UNTRACKABLE);
    else
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        Unit::AuraList const& auras = GetTarget()->GetAurasByType(SPELL_AURA_UNTRACKABLE);
        if ( auras.size() > 1)
            return;

        GetTarget()->RemoveByteFlag(UNIT_FIELD_BYTES_1, 2, UNIT_STAND_FLAGS_UNTRACKABLE);
    }
}

void Aura::HandleAuraModPacify(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    if (!GetTarget())
        return;

    if (apply)
        GetTarget()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    else
        if (!GetTarget()->HasAuraType(SPELL_AURA_MOD_PACIFY) &&
            !GetTarget()->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
            GetTarget()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
}

void Aura::HandleAuraModPacifyAndSilence(bool apply, bool Real)
{
    HandleAuraModPacify(apply, Real);
    HandleAuraModSilence(apply, Real);
}

void Aura::HandleAuraGhost(bool apply, bool /*Real*/)
{
    if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
        return;

    if (apply)
    {
        GetTarget()->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
    }
    else
    {
        GetTarget()->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
    }
}

void Aura::HandleShieldBlockValue(bool apply, bool /*Real*/)
{
    BaseModType modType = FLAT_MOD;
    if (m_modifier.m_auraname == SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT)
        modType = PCT_MOD;

    if (GetTarget()->GetTypeId() == TYPEID_PLAYER)
        ((Player*)GetTarget())->HandleBaseModValue(SHIELD_BLOCK_VALUE, modType, float(m_modifier.m_amount), apply);
}

void Aura::HandleAuraRetainComboPoints(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    Unit* target = GetTarget();

    // combo points was added in SPELL_EFFECT_ADD_COMBO_POINTS handler
    // remove only if aura expire by time (in case combo points amount change aura removed without combo points lost)
    if (!apply && m_removeMode == AURA_REMOVE_BY_EXPIRE && target->GetComboTargetGuid())
        if (Unit* unit = ObjectAccessor::GetUnit(*GetTarget(),target->GetComboTargetGuid()))
            target->AddComboPoints(unit, -m_modifier.m_amount);
}

void Aura::HandleModUnattackable( bool Apply, bool Real )
{
    if (Real && Apply)
     {
        GetTarget()->CombatStop();
        GetTarget()->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);
     }
    GetTarget()->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE,Apply);
}

void Aura::HandleSpiritOfRedemption( bool apply, bool Real )
{
    // spells required only Real aura add/remove
    if (!Real)
        return;

    Unit *target = GetTarget();

    // prepare spirit state
    if (apply)
    {
        if (target->GetTypeId()==TYPEID_PLAYER)
        {
            // disable breath/etc timers
            ((Player*)target)->StopMirrorTimers();

            // set stand state (expected in this form)
            if (!target->IsStandState())
                target->SetStandState(UNIT_STAND_STATE_STAND);
        }

        target->SetHealth(1);
    }
    // die at aura end
    else
        target->DealDamage(target, target->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, GetSpellProto(), false);
}

void Aura::HandleSchoolAbsorb(bool apply, bool Real)
{
    if (!Real)
        return;

    Unit* caster = GetCaster();
    if (!caster)
        return;

    Unit *target = GetTarget();
    SpellEntry const* spellProto = GetSpellProto();
    if (apply)
    {
        // prevent double apply bonuses
        if (target->GetTypeId()!=TYPEID_PLAYER || !((Player*)target)->GetSession()->PlayerLoading())
        {
            float DoneActualBenefit = 0.0f;
            switch(spellProto->SpellFamilyName)
            {
                case SPELLFAMILY_GENERIC:
                    // Stoicism
                    if (spellProto->Id == 70845)
                        DoneActualBenefit = caster->GetMaxHealth() * 0.20f;
                    break;
                case SPELLFAMILY_PRIEST:
                    // Power Word: Shield
                    if (spellProto->GetSpellFamilyFlags().test<CF_PRIEST_POWER_WORD_SHIELD>())
                    {
                        //+80.68% from +spell bonus
                        int32 spellPower = caster->SpellBaseHealingBonusDone(GetSpellSchoolMask(spellProto));
                        float SpellBonus = spellPower * 0.8068f;
                        //Borrowed Time
                        Unit::AuraList const& borrowedTime = caster->GetAurasByType(SPELL_AURA_DUMMY);
                        for(Unit::AuraList::const_iterator itr = borrowedTime.begin(); itr != borrowedTime.end(); ++itr)
                        {
                            SpellEntry const* i_spell = (*itr)->GetSpellProto();
                            if (i_spell->SpellFamilyName==SPELLFAMILY_PRIEST && i_spell->SpellIconID == 2899 && i_spell->EffectMiscValue[(*itr)->GetEffIndex()] == 24)
                            {
                                SpellBonus += spellPower * (*itr)->GetModifier()->m_amount / 100;
                                break;
                            }
                        }
                        // extra absorb from talents
                        int32 BaseBonus = 0, PctAddMod = 0;
                        Unit::AuraList const& pctModAuras = caster->GetAurasByType(SPELL_AURA_ADD_PCT_MODIFIER);
                        for (Unit::AuraList::const_iterator itr = pctModAuras.begin(); itr != pctModAuras.end(); ++itr)
                        {
                            SpellEntry const* i_spell = (*itr)->GetSpellProto();
                            if (i_spell->SpellFamilyName != SPELLFAMILY_PRIEST || (*itr)->GetEffIndex() != EFFECT_INDEX_0)
                                continue;
                            // Twin Disciplines / Spiritual Healing
                            if (i_spell->SpellIconID == 2292 || i_spell->SpellIconID == 46)
                                PctAddMod += (*itr)->GetModifier()->m_amount;
                            // Improved Power Word: Shield
                            else if (i_spell->SpellIconID == 566)
                                SpellBonus *= (100.0f + (*itr)->GetModifier()->m_amount) / 100.0f;
                            // Item - Priest T10 Healer 4P Bonus
                            else if (i_spell->Id == 70798)
                            {
                                BaseBonus -= m_modifier.m_amount * (*itr)->GetModifier()->m_amount / 100;   // base bonus already added as SPELLMOD_ALL_EFFECTS
                                PctAddMod += (*itr)->GetModifier()->m_amount;
                            }
                        }
                        Unit::AuraList const& healingPctAuras = caster->GetAurasByType(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
                        for (Unit::AuraList::const_iterator itr = healingPctAuras.begin(); itr != healingPctAuras.end(); ++itr)
                        {
                            SpellEntry const* i_spell = (*itr)->GetSpellProto();
                            // Focused Power
                            if (i_spell->SpellFamilyName == SPELLFAMILY_PRIEST && i_spell->SpellIconID == 2210)
                            {
                                PctAddMod += (*itr)->GetModifier()->m_amount;
                                break;
                            }
                        }
                        if (PctAddMod)
                        {
                            BaseBonus += m_modifier.m_amount * PctAddMod / 100;
                            SpellBonus *= (100.0f + PctAddMod) / 100.0f;
                        }
                        DoneActualBenefit = BaseBonus + SpellBonus;
                    }

                    break;
                case SPELLFAMILY_MAGE:
                    // Frost Ward, Fire Ward
                    if (spellProto->GetSpellFamilyFlags().test<CF_MAGE_FIRE_WARD, CF_MAGE_FROST_WARD>())
                        //+10% from +spell bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonusDone(GetSpellSchoolMask(spellProto)) * 0.1f;
                    // Ice Barrier
                    else if (spellProto->GetSpellFamilyFlags().test<CF_MAGE_ICE_BARRIER>())
                        //+80.67% from +spell bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonusDone(GetSpellSchoolMask(spellProto)) * 0.8067f;
                    break;
                case SPELLFAMILY_WARLOCK:
                    // Shadow Ward
                    if (spellProto->GetSpellFamilyFlags().test<CF_WARLOCK_MISC_BUFFS>())
                        //+30% from +spell bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonusDone(GetSpellSchoolMask(spellProto)) * 0.30f;
                    break;
                case SPELLFAMILY_PALADIN:
                    // Sacred Shield
                    // (check not strictly needed, only Sacred Shield has SPELL_AURA_SCHOOL_ABSORB in SPELLFAMILY_PALADIN at this time)
                    if (spellProto->GetSpellFamilyFlags().test<CF_PALADIN_SACRED_SHIELD>())
                    {
                        // +75% from spell power
                        DoneActualBenefit = caster->SpellBaseHealingBonusDone(GetSpellSchoolMask(spellProto)) * 0.75f;
                    }
                    break;
                default:
                    break;
            }

            DoneActualBenefit *= caster->CalculateLevelPenalty(GetSpellProto());

            m_modifier.m_amount += (int32)DoneActualBenefit;
            m_modifier.m_baseamount += (int32)DoneActualBenefit;
        }
    }
    else
    {
        if (caster && caster->GetTypeId() == TYPEID_PLAYER && spellProto->Id == 47788 &&
            m_removeMode == AURA_REMOVE_BY_EXPIRE)
        {
            if (Aura *aur = caster->GetAura(63231, EFFECT_INDEX_0))
            {
                ((Player*)caster)->SendModifyCooldown(spellProto->Id,-aur->GetSpellProto()->CalculateSimpleValue(EFFECT_INDEX_0)*IN_MILLISECONDS);
            }
        }
    }
}

void Aura::PeriodicTick()
{
    Unit* target = GetTarget();
    SpellAuraHolderPtr holder = GetHolder();
    SpellEntry const* spellProto = GetSpellProto();

    if (!holder || !target || !spellProto)
        return;

    if (target->IsImmuneToSpell(spellProto, GetAffectiveCaster() ? GetAffectiveCaster()->IsFriendlyTo(target) : true))
        return;

    switch(m_modifier.m_auraname)
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        {
            // don't damage target if not alive, possible death persistent effects
            if (!target->IsInWorld() ||  !target->isAlive())
                return;

            Unit* pCaster = GetAffectiveCaster();
            if (!pCaster)
                return;

            if (!pCaster->IsInWorld())
                return;

            if (spellProto->Effect[GetEffIndex()] == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
                pCaster->SpellHitResult(target, spellProto, false) != SPELL_MISS_NONE)
                return;

            // Check for immune (not use charges)
            if (IsSpellCauseDamage(spellProto) && target->IsImmunedToDamage(GetSpellSchoolMask(spellProto)))
                return;

            // some auras remove at specific health level or more
            switch (GetId())
            {
                case 31956:
                case 35321:
                case 38363:
                case 38801:
                case 39215:
                case 43093:
                case 48920:
                case 70292:
                case 71316:
                case 71317:
                {
                    if (target->GetHealth() == target->GetMaxHealth() )
                    {
                        target->RemoveAurasDueToSpell(GetId());
                        return;
                    }
                    break;
                }
                case 38772:
                {
                    uint32 percent =
                        GetEffIndex() < EFFECT_INDEX_2 && spellProto->Effect[GetEffIndex()] == SPELL_EFFECT_DUMMY ?
                        pCaster->CalculateSpellDamage(target, spellProto, SpellEffectIndex(GetEffIndex() + 1)) :
                        100;
                    if (target->GetHealth() * 100 >= target->GetMaxHealth() * percent )
                    {
                        target->RemoveAurasDueToSpell(GetId());
                        return;
                    }
                    break;
                }
                case 70541: // Infest (Lich King)
                case 73779:
                case 73780:
                case 73781:
                {
                    if (target->GetHealth() >= target->GetMaxHealth() * 0.9f )
                    {
                        target->RemoveAurasDueToSpell(GetId());
                        return;
                    }
                    else
                    {
                        // increasing damage (15% more each tick)
                        // don't increase first tick damage
                        if (GetModifier()->m_miscvalue > 0)
                            GetModifier()->m_amount = GetModifier()->m_amount * 1.15f;
                        else
                            GetModifier()->m_miscvalue += 1;
                    }
                    break;
                }
                case 70672: // Gaseous Bloat (Putricide)
                case 72455:
                case 72832:
                case 72833:
                {
                    // drop 1 stack
                    if (GetHolder()->ModStackAmount(-1))
                    {
                        target->RemoveAurasDueToSpell(GetId());
                        return;
                    }

                    break;
                }
                case 74562: // SPELL_FIERY_COMBUSTION - Ruby sanctum boss Halion, added mark (74567, dummy) every tick
                {
                    target->CastSpell(target, 74567, true, NULL, NULL, GetCasterGuid());
                    break;
                }
                case 74792: // SPELL_SOUL_CONSUMPTION - Ruby sanctum boss Halion, added mark (74795, dummy) every tick
                {
                    target->CastSpell(target, 74795, true, NULL, NULL, GetCasterGuid());
                    break;
                }
                case 67297:
                case 65950:
                    pCaster->CastSpell(target, 65951, true);
                    break;
                case 66001:
                case 67282:
                    pCaster->CastSpell(target, 66002, true);
                    break;
                case 67281:
                case 67283:
                    pCaster->CastSpell(target, 66000, true);
                    break;
                case 67296:
                case 67298:
                    pCaster->CastSpell(target, 65952, true);
                    break;
                // Unbound Plague (Putricide)
                case 70911:
                case 72854:
                case 72855:
                case 72856:
                    m_modifier.m_miscvalue += 1; // store ticks number in miscvalue
                    m_modifier.m_amount = m_modifier.m_baseamount * pow(2.7f, m_modifier.m_miscvalue * 0.223f);
                    break;
                // Boiling Blood (Saurfang)
                case 72385:
                case 72441:
                case 72442:
                case 72443:
                    target->CastSpell(target, 72202, true); // Blood Link
                    break;
                default:
                    break;
            }

            DamageInfo damageInfo = DamageInfo(pCaster, target, spellProto);
            damageInfo.CleanDamage(0, 0, BASE_ATTACK, MELEE_HIT_NORMAL);
            damageInfo.damageType = DOT;

            // ignore non positive values (can be result apply spellmods to aura damage
            uint32 amount = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;

            if (m_modifier.m_auraname == SPELL_AURA_PERIODIC_DAMAGE)
                damageInfo.damage = amount;
            else
                damageInfo.damage = uint32(target->GetMaxHealth()*amount/100);

            // SpellDamageBonus for magic spells
            if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE || spellProto->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)
                target->SpellDamageBonusTaken(&damageInfo,GetStackAmount());
            // MeleeDamagebonus for weapon based spells
            else
            {
                damageInfo.attackType = GetWeaponAttackType(spellProto);
                target->MeleeDamageBonusTaken(&damageInfo, GetStackAmount());
            }

            // Calculate armor mitigation if it is a physical spell
            // But not for bleed mechanic spells
            if ((GetSpellSchoolMask(spellProto) & SPELL_SCHOOL_MASK_NORMAL) &&
                GetEffectMechanic(spellProto, m_effIndex) != MECHANIC_BLEED)
            {
                uint32 pdamageReductedArmor = pCaster->CalcArmorReducedDamage(target, damageInfo.damage);
                damageInfo.cleanDamage += damageInfo.damage - pdamageReductedArmor;
                damageInfo.damage = pdamageReductedArmor;
            }

            // Curse of Agony damage-per-tick calculation
            if (spellProto->SpellFamilyName==SPELLFAMILY_WARLOCK && spellProto->GetSpellFamilyFlags().test<CF_WARLOCK_CURSE_OF_AGONY>() && spellProto->SpellIconID==544)
            {
                // 1..4 ticks, 1/2 from normal tick damage
                if (GetAuraTicks() <= 4)
                    damageInfo.damage = damageInfo.damage/2;
                // 9..12 ticks, 3/2 from normal tick damage
                else if (GetAuraTicks() >= 9)
                    damageInfo.damage += (damageInfo.damage + 1) / 2;       // +1 prevent 0.5 damage possible lost at 1..4 ticks
                // 5..8 ticks have normal tick damage
            }

            // This method can modify pdamage
            bool isCrit = IsCritFromAbilityAura(pCaster, &damageInfo);

            // send critical in hit info for threat calculation
            if (isCrit)
            {
                damageInfo.hitOutCome = MELEE_HIT_CRIT;
                // Resilience - reduce crit damage
                damageInfo.damage -= target->GetSpellCritDamageReduction(damageInfo.damage);
            }

            // only from players
            // FIXME: need use SpellDamageBonus instead?
            if (pCaster->GetTypeId() == TYPEID_PLAYER)
                damageInfo.damage -= target->GetSpellDamageReduction(damageInfo.damage);

            if (GetSpellProto()->Id == 50344) // Dream Funnel Oculus drake spell
                damageInfo.damage = uint32(pCaster->GetMaxHealth()*0.05f);

            target->CalculateDamageAbsorbAndResist(pCaster, &damageInfo, !GetSpellProto()->HasAttribute(SPELL_ATTR_EX_CANT_REFLECTED));

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s attacked %s for %u dmg inflicted by %u abs is %u",
                GetAffectiveCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), damageInfo.damage, GetId(), damageInfo.absorb);

            pCaster->DealDamageMods(&damageInfo);

            // Set trigger flag
            damageInfo.procAttacker = PROC_FLAG_ON_DO_PERIODIC; //  | PROC_FLAG_SUCCESSFUL_HARMFUL_SPELL_HIT;
            damageInfo.procVictim   = PROC_FLAG_ON_TAKE_PERIODIC;// | PROC_FLAG_TAKEN_HARMFUL_SPELL_HIT;
            damageInfo.procEx = isCrit ? PROC_EX_CRITICAL_HIT : PROC_EX_NORMAL_HIT;

            damageInfo.damage = (damageInfo.damage <= damageInfo.absorb + damageInfo.resist) ? 0 : (damageInfo.damage - damageInfo.absorb - damageInfo.resist);

            if (damageInfo.damage <= 0)
                damageInfo.procEx &= ~PROC_EX_DIRECT_DAMAGE;
            else
                damageInfo.procEx |= PROC_EX_DIRECT_DAMAGE;

            uint32 overkill = damageInfo.damage > target->GetHealth() ? damageInfo.damage - target->GetHealth() : 0;
            SpellPeriodicAuraLogInfo pInfo(this, damageInfo.damage, overkill, damageInfo.absorb, damageInfo.resist, 0.0f, isCrit);
            target->SendPeriodicAuraLog(&pInfo);

            if (damageInfo.damage)
                damageInfo.procVictim |= PROC_FLAG_TAKEN_ANY_DAMAGE;

            pCaster->ProcDamageAndSpell(&damageInfo);

            pCaster->DealDamage(target, &damageInfo, true);

            // Drain Soul (chance soul shard)
            if (pCaster->GetTypeId() == TYPEID_PLAYER && spellProto->SpellFamilyName == SPELLFAMILY_WARLOCK && spellProto->GetSpellFamilyFlags().test<CF_WARLOCK_DRAIN_SOUL>())
            {
                // Only from non-grey units
                if (roll_chance_i(10) &&                    // 1-2 from drain with final, 0-1 from damage
                    ((Player*)pCaster)->isHonorOrXPTarget(target) &&
                    (target->GetTypeId() != TYPEID_UNIT || ((Player*)pCaster)->isAllowedToLoot((Creature*)target)))
                {
                    pCaster->CastSpell(pCaster, 43836, true, NULL, this);
                }
            }

            break;
        }
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        {
            // don't damage target if not alive, possible death persistent effects
            if (!target->IsInWorld() ||  !target->isAlive())
                return;

            Unit *pCaster = GetAffectiveCaster();
            if (!pCaster)
                return;

            if (!pCaster->IsInWorld() || !pCaster->isAlive())
                return;

            if (spellProto->Effect[GetEffIndex()] == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
                pCaster->SpellHitResult(target, spellProto, false) != SPELL_MISS_NONE)
                return;

            // Check for immune
            if (IsSpellCauseDamage(spellProto) && target->IsImmunedToDamage(GetSpellSchoolMask(spellProto)))
                return;

            DamageInfo damageInfo = DamageInfo(pCaster, target, spellProto);
            damageInfo.CleanDamage(0, 0, BASE_ATTACK, MELEE_HIT_NORMAL );
            damageInfo.damageType = DOT;

            damageInfo.damage = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;

            //Calculate armor mitigation if it is a physical spell
            if (GetSpellSchoolMask(spellProto) & SPELL_SCHOOL_MASK_NORMAL)
            {
                uint32 pdamageReductedArmor = pCaster->CalcArmorReducedDamage(target, damageInfo.damage);
                damageInfo.cleanDamage += damageInfo.damage - pdamageReductedArmor;
                damageInfo.damage = pdamageReductedArmor;
            }

            target->SpellDamageBonusTaken(&damageInfo, GetStackAmount());

            bool isCrit = IsCritFromAbilityAura(pCaster, &damageInfo);

            // send critical in hit info for threat calculation
            if (isCrit)
            {
                damageInfo.hitOutCome = MELEE_HIT_CRIT;
                // Resilience - reduce crit damage
                damageInfo.damage -= target->GetSpellCritDamageReduction(damageInfo.damage);
            }

            // only from players
            // FIXME: need use SpellDamageBonus instead?
            if (GetAffectiveCasterGuid().IsPlayer())
                damageInfo.damage -= target->GetSpellDamageReduction(damageInfo.damage);

            target->CalculateDamageAbsorbAndResist(pCaster, &damageInfo, !spellProto->HasAttribute(SPELL_ATTR_EX_CANT_REFLECTED));

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s health leech of %s for %u dmg inflicted by %u abs is %u",
                GetAffectiveCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), damageInfo.damage, GetId(),damageInfo.absorb);

            pCaster->DealDamageMods(&damageInfo);

            pCaster->SendSpellNonMeleeDamageLog(target, GetId(), damageInfo.damage, damageInfo.SchoolMask(), damageInfo.absorb, damageInfo.resist, false, 0, isCrit);

            float multiplier = spellProto->EffectMultipleValue[GetEffIndex()] > 0 ? spellProto->EffectMultipleValue[GetEffIndex()] : 1;

            // Set trigger flag
            damageInfo.procAttacker = PROC_FLAG_ON_DO_PERIODIC; //  | PROC_FLAG_SUCCESSFUL_HARMFUL_SPELL_HIT;
            damageInfo.procVictim   = PROC_FLAG_ON_TAKE_PERIODIC;// | PROC_FLAG_TAKEN_HARMFUL_SPELL_HIT;
            damageInfo.procEx = isCrit ? PROC_EX_CRITICAL_HIT : PROC_EX_NORMAL_HIT;

            damageInfo.damage = (damageInfo.damage <= damageInfo.absorb + damageInfo.resist) ? 0 : (damageInfo.damage - damageInfo.absorb - damageInfo.resist);
            if (damageInfo.damage)
                damageInfo.procVictim |= PROC_FLAG_TAKEN_ANY_DAMAGE;

            pCaster->ProcDamageAndSpell(&damageInfo);
            int32 new_damage = pCaster->DealDamage(target, &damageInfo, false);

            if (!target->isAlive() && pCaster->IsNonMeleeSpellCasted(false))
                for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
                    if (Spell* spell = pCaster->GetCurrentSpell(CurrentSpellTypes(i)))
                        if (spell->m_spellInfo->Id == GetId())
                            spell->cancel();

            if (Player *modOwner = pCaster->GetSpellModOwner())
                modOwner->ApplySpellMod(GetId(), SPELLMOD_MULTIPLE_VALUE, multiplier);

            int32 heal = pCaster->SpellHealingBonusTaken(pCaster, spellProto, int32(new_damage * multiplier), DOT, GetStackAmount());

            uint32 absorbHeal = 0;
            pCaster->CalculateHealAbsorb(heal, &absorbHeal);

            int32 gain = pCaster->DealHeal(pCaster, heal - absorbHeal, spellProto, false, absorbHeal);
            pCaster->getHostileRefManager().threatAssist(pCaster, gain * 0.5f * sSpellMgr.GetSpellThreatMultiplier(spellProto), spellProto);
            break;
        }
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_OBS_MOD_HEALTH:
        {
            // don't heal target if not alive, mostly death persistent effects from items
            if (!target->isAlive())
                return;

            Unit *pCaster = GetAffectiveCaster();
            if (!pCaster)
                return;

            // heal for caster damage (must be alive)
            if (target != pCaster && spellProto->SpellVisual == 163 && !pCaster->isAlive())
                return;

            DamageInfo damageInfo = DamageInfo(pCaster, target, spellProto);
            damageInfo.damageType = NODAMAGE;

            // ignore non positive values (can be result apply spellmods to aura damage
            uint32 amount  = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;

            if (m_modifier.m_auraname == SPELL_AURA_OBS_MOD_HEALTH)
                damageInfo.damage = uint32(target->GetMaxHealth() * amount / 100);
            else
            {
                damageInfo.damage = amount;
                // Wild Growth (1/7 - 6 + 2*ramainTicks) %
                if (spellProto->SpellFamilyName == SPELLFAMILY_DRUID && spellProto->SpellIconID == 2864)
                {
                    int32 ticks = GetAuraMaxTicks();
                    int32 remainingTicks = ticks - GetAuraTicks();
                    int32 addition = int32(amount)*ticks*(-6+2*remainingTicks)/100;

                    if (GetAuraTicks() > 1 && ticks > 1)
                        // Item - Druid T10 Restoration 2P Bonus
                        if (Aura *aura = pCaster->GetAura(70658, EFFECT_INDEX_0))
                            addition += abs(int32((addition * aura->GetModifier()->m_amount) / ((ticks-1)* 100)));

                    damageInfo.damage = damageInfo.damage + addition;
                }
            }
            damageInfo.CleanDamage(-int32(damageInfo.damage), 0, BASE_ATTACK, MELEE_HIT_NORMAL);

            damageInfo.damage = target->SpellHealingBonusTaken(pCaster, spellProto, damageInfo.damage, DOT, GetStackAmount());

            // This method can modify pdamage
            bool isCrit = IsCritFromAbilityAura(pCaster, &damageInfo);

            pCaster->CalculateHealAbsorb(damageInfo.damage, &damageInfo.absorb);
            damageInfo.damage -= damageInfo.absorb;

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s heal of %s for %u health  (absorbed %u) inflicted by %u",
                GetAffectiveCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), damageInfo.damage, damageInfo.absorb, GetId());

            int32 gain = target->ModifyHealth(damageInfo.damage);
            damageInfo.cleanDamage = damageInfo.damage;
            damageInfo.damage = uint32(gain);
            uint32 overDamage = damageInfo.cleanDamage - damageInfo.damage;
            SpellPeriodicAuraLogInfo pInfo(this, damageInfo.cleanDamage, overDamage, damageInfo.absorb, 0, 0.0f, isCrit);
            target->SendPeriodicAuraLog(&pInfo);

            // Set trigger flag
            damageInfo.procAttacker = PROC_FLAG_ON_DO_PERIODIC;
            damageInfo.procVictim   = PROC_FLAG_ON_TAKE_PERIODIC;
            damageInfo.procEx = PROC_EX_PERIODIC_POSITIVE | (isCrit ? PROC_EX_CRITICAL_HIT : PROC_EX_NORMAL_HIT);

            pCaster->ProcDamageAndSpell(&damageInfo);

            // add HoTs to amount healed in bgs
            if (pCaster->GetTypeId() == TYPEID_PLAYER )
                if ( BattleGround *bg = ((Player*)pCaster)->GetBattleGround() )
                    bg->UpdatePlayerScore(((Player*)pCaster), SCORE_HEALING_DONE, gain);

            target->getHostileRefManager().threatAssist(pCaster, float(gain) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(spellProto), spellProto);

            // heal for caster damage (Health funnel only! spellProto->SpellVisual == 163 wrong - some repair spells here!)
            if (target != pCaster && spellProto->GetSpellFamilyFlags().test<CF_WARLOCK_HEALTH_FUNNEL>())
            {
                uint32 dmg = spellProto->manaPerSecond;
                if (pCaster->GetHealth() <= dmg && pCaster->GetTypeId()==TYPEID_PLAYER)
                {
                    pCaster->RemoveAurasDueToSpell(GetId());

                    // finish current generic/channeling spells, don't affect autorepeat
                    pCaster->FinishSpell(CURRENT_GENERIC_SPELL);
                    pCaster->FinishSpell(CURRENT_CHANNELED_SPELL);
                }
                else
                {
                    DamageInfo funneldamageInfo = DamageInfo(pCaster, pCaster, spellProto);
                    funneldamageInfo.damage = gain;
                    funneldamageInfo.absorb = 0;
                    funneldamageInfo.damageType = DOT;
                    pCaster->DealDamageMods(&funneldamageInfo);
                    pCaster->SendSpellNonMeleeDamageLog(pCaster, GetId(), funneldamageInfo.damage, GetSpellSchoolMask(spellProto), funneldamageInfo.absorb, 0, false, 0, false);
                    pCaster->DealDamage(pCaster, &funneldamageInfo, true);
                }
            }

            // Light's Favor (Lich King)
            // recalculate bonus damage done after each tick
            if (GetId() == 69382)
            {
                if (Aura *aur = GetHolder()->GetAuraByEffectIndex(EFFECT_INDEX_1))
                    aur->GetModifier()->m_amount = int32(target->GetHealthPercent());
            }
            else if (GetId() == 20578 || GetId() == 52749 || GetId() == 54045)
            {
                // cannibalize anim
                target->HandleEmoteCommand(EMOTE_STATE_CANNIBALIZE);
            }


//            uint32 procAttacker = PROC_FLAG_ON_DO_PERIODIC;//   | PROC_FLAG_SUCCESSFUL_HEAL;
//            uint32 procVictim   = 0;//ROC_FLAG_ON_TAKE_PERIODIC | PROC_FLAG_TAKEN_HEAL;
            // ignore item heals
//            if (procSpell && !haveCastItem)
//                pCaster->ProcDamageAndSpell(target, procAttacker, procVictim, PROC_EX_NORMAL_HIT, pdamage, BASE_ATTACK, spellProto);
            break;
        }
        case SPELL_AURA_PERIODIC_MANA_LEECH:
        {
            // don't damage target if not alive, possible death persistent effects
            if (!target->isAlive())
                return;

            // don't energize isolated units (banished)
            if (target->hasUnitState(UNIT_STAT_ISOLATED))
                return;

            if (m_modifier.m_miscvalue < 0 || m_modifier.m_miscvalue >= MAX_POWERS)
                return;

            Powers power = Powers(m_modifier.m_miscvalue);

            // power type might have changed between aura applying and tick (druid's shapeshift)
            if (target->getPowerType() != power)
                return;

            Unit* pCaster = GetAffectiveCaster();
            if (!pCaster)
                return;

            if (!pCaster->isAlive())
                return;

            if (GetSpellProto()->Effect[GetEffIndex()] == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
                pCaster->SpellHitResult(target, spellProto, false) != SPELL_MISS_NONE)
                return;

            // Check for immune (not use charges)
            if (IsSpellCauseDamage(spellProto) && target->IsImmunedToDamage(GetSpellSchoolMask(spellProto)))
                return;

            // ignore non positive values (can be result apply spellmods to aura damage
            uint32 pdamage = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;

            // Special case: draining x% of mana (up to a maximum of 2*x% of the caster's maximum mana)
            // It's mana percent cost spells, m_modifier.m_amount is percent drain from target
            if (spellProto->ManaCostPercentage)
            {
                // max value
                uint32 maxmana = pCaster->GetMaxPower(power)  * pdamage * 2 / 100;
                pdamage = target->GetMaxPower(power) * pdamage / 100;
                if (pdamage > maxmana)
                    pdamage = maxmana;
            }

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s power leech of %s for %u dmg inflicted by %u",
                GetAffectiveCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), pdamage, GetId());

            int32 drain_amount = target->GetPower(power) > pdamage ? pdamage : target->GetPower(power);

            // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
            if (power == POWER_MANA)
                drain_amount -= target->GetSpellCritDamageReduction(drain_amount);

            target->ModifyPower(power, -drain_amount);

            float gain_multiplier = 0.0f;

            if (pCaster->GetMaxPower(power) > 0)
            {
                gain_multiplier = spellProto->EffectMultipleValue[GetEffIndex()];

                if (Player *modOwner = pCaster->GetSpellModOwner())
                    modOwner->ApplySpellMod(GetId(), SPELLMOD_MULTIPLE_VALUE, gain_multiplier);
            }

            SpellPeriodicAuraLogInfo pInfo(this, drain_amount, 0, 0, 0, gain_multiplier);
            target->SendPeriodicAuraLog(&pInfo);

            if (int32 gain_amount = int32(drain_amount * gain_multiplier))
            {
                int32 gain = pCaster->ModifyPower(power, gain_amount);

                if (GetId() == 5138)                        // Drain Mana
                    if (Aura* petPart = GetHolder()->GetAuraByEffectIndex(EFFECT_INDEX_1))
                        if (int pet_gain = gain_amount * petPart->GetModifier()->m_amount / 100)
                            pCaster->CastCustomSpell(pCaster, 32554, &pet_gain, NULL, NULL, true);

                target->AddThreat(pCaster, float(gain) * 0.5f, pInfo.critical, GetSpellSchoolMask(spellProto), spellProto);
                if (pCaster->GetTypeId() == TYPEID_PLAYER && spellProto->Id == 5138 && pCaster->HasSpell(30326))
                    if (pCaster->GetPet())
                    {
                        GroupPetList m_groupPets = pCaster->GetPets();
                        if (!m_groupPets.empty())
                        {
                            for (GroupPetList::const_iterator itr = m_groupPets.begin(); itr != m_groupPets.end(); ++itr)
                                if (Pet* _pet = pCaster->GetMap()->GetPet(*itr))
                                    if (_pet && _pet->isAlive())
                                        pCaster->CastCustomSpell(_pet, 32554, &gain_amount, NULL, NULL, true, NULL, NULL, pCaster->GetObjectGuid());
                        }
                    }

            }

            // Some special cases
            switch (GetId())
            {
                case 32960:                                 // Mark of Kazzak
                {
                    if (target->GetTypeId() == TYPEID_PLAYER && target->getPowerType() == POWER_MANA)
                    {
                        // Drain 5% of target's mana
                        pdamage = target->GetMaxPower(POWER_MANA) * 5 / 100;
                        drain_amount = target->GetPower(POWER_MANA) > pdamage ? pdamage : target->GetPower(POWER_MANA);
                        target->ModifyPower(POWER_MANA, -drain_amount);

                        SpellPeriodicAuraLogInfo pInfo(this, drain_amount, 0, 0, 0, 0.0f);
                        target->SendPeriodicAuraLog(&pInfo);
                    }
                    // no break here
                }
                case 21056:                                 // Mark of Kazzak
                case 31447:                                 // Mark of Kaz'rogal
                {
                    uint32 triggerSpell = 0;
                    switch (GetId())
                    {
                        case 21056: triggerSpell = 21058; break;
                        case 31447: triggerSpell = 31463; break;
                        case 32960: triggerSpell = 32961; break;
                    }
                    if (target->GetTypeId() == TYPEID_PLAYER && target->GetPower(power) == 0)
                    {
                        target->CastSpell(target, triggerSpell, true, NULL, this);
                        target->RemoveAurasDueToSpell(GetId());
                    }
                    break;
                }
            }
            break;
        }
        case SPELL_AURA_PERIODIC_ENERGIZE:
        {
            // don't energize target if not alive, possible death persistent effects
            if (!target->isAlive())
                return;

            // don't energize isolated units (banished)
            if (target->hasUnitState(UNIT_STAT_ISOLATED))
                return;

            // ignore non positive values (can be result apply spellmods to aura damage
            uint32 pdamage = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s energize %s for %u dmg inflicted by %u",
                GetAffectiveCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), pdamage, GetId());

            if (m_modifier.m_miscvalue < 0 || m_modifier.m_miscvalue >= MAX_POWERS)
                break;

            Powers power = Powers(m_modifier.m_miscvalue);

            if (target->GetMaxPower(power) == 0)
                break;

            SpellPeriodicAuraLogInfo pInfo(this, pdamage, 0, 0, 0, 0.0f);
            target->SendPeriodicAuraLog(&pInfo);

            int32 gain = target->ModifyPower(power,pdamage);

            if (Unit* pCaster = GetCaster())
                target->getHostileRefManager().threatAssist(pCaster, float(gain) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(spellProto), spellProto);
            break;
        }
        case SPELL_AURA_OBS_MOD_MANA:
        {
            // don't energize target if not alive, possible death persistent effects
            if (!target->isAlive())
                return;

            // don't energize isolated units (banished)
            if (target->hasUnitState(UNIT_STAT_ISOLATED))
                return;

            Powers powerType = ( (m_modifier.m_miscvalue < 0) ? POWER_MANA : Powers(m_modifier.m_miscvalue));

            // ignore non positive values (can be result apply spellmods to aura damage
            uint32 amount = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;

            uint32 pdamage = uint32(target->GetMaxPower(powerType) * amount / 100);

            DETAIL_FILTER_LOG(LOG_FILTER_PERIODIC_AFFECTS, "PeriodicTick: %s energize %s for %u power %u inflicted by %u",
                GetAffectiveCasterGuid().GetString().c_str(), target->GetGuidStr().c_str(), pdamage, powerType, GetId());

            if (target->GetMaxPower(powerType) == 0)
                break;

            SpellPeriodicAuraLogInfo pInfo(this, pdamage, 0, 0, 0, 0.0f);
            target->SendPeriodicAuraLog(&pInfo);

            int32 gain = target->ModifyPower(powerType, pdamage);

            if (Unit* pCaster = GetCaster())
                target->getHostileRefManager().threatAssist(pCaster, float(gain) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(spellProto), spellProto);
            break;
        }
        case SPELL_AURA_POWER_BURN_MANA:
        {
            // don't mana burn target if not alive, possible death persistent effects
            if (!target->isAlive())
                return;

            Unit *pCaster = GetAffectiveCaster();
            if (!pCaster)
                return;

            // Check for immune (not use charges)
            if (IsSpellCauseDamage(spellProto) && target->IsImmunedToDamage(GetSpellSchoolMask(spellProto)))
                return;

            DamageInfo damageInfo(pCaster, target, spellProto);

            damageInfo.damage = m_modifier.m_amount > 0 ? m_modifier.m_amount : 0;

            Powers powerType = Powers(m_modifier.m_miscvalue);

            if (!target->isAlive() || target->getPowerType() != powerType)
                return;

            // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
            if (powerType == POWER_MANA)
                damageInfo.damage -= target->GetSpellCritDamageReduction(damageInfo.damage);

            damageInfo.cleanDamage = abs(target->ModifyPower(powerType, -damageInfo.damage));

            damageInfo.damage = uint32(damageInfo.cleanDamage * spellProto->EffectMultipleValue[GetEffIndex()]);

            // maybe has to be sent different to client, but not by SMSG_PERIODICAURALOG
            pCaster->CalculateSpellDamage(&damageInfo);

            damageInfo.target->CalculateAbsorbResistBlock(pCaster, &damageInfo, spellProto);

            pCaster->DealDamageMods(&damageInfo);

            pCaster->SendSpellNonMeleeDamageLog(&damageInfo);

            // Set trigger flag
            damageInfo.procAttacker = PROC_FLAG_ON_DO_PERIODIC; //  | PROC_FLAG_SUCCESSFUL_HARMFUL_SPELL_HIT;
            damageInfo.procVictim   = PROC_FLAG_ON_TAKE_PERIODIC;// | PROC_FLAG_TAKEN_HARMFUL_SPELL_HIT;
            damageInfo.procEx       = createProcExtendMask(&damageInfo, SPELL_MISS_NONE);

            if (damageInfo.damage)
            {
                damageInfo.procVictim  |= PROC_FLAG_TAKEN_ANY_DAMAGE;
                damageInfo.procEx      |= PROC_EX_DIRECT_DAMAGE;
            }

            pCaster->ProcDamageAndSpell(&damageInfo);
            pCaster->DealSpellDamage(&damageInfo, true);

            break;
        }
        case SPELL_AURA_MOD_REGEN:
        {
            // don't heal target if not alive, possible death persistent effects
            if (!target->isAlive())
                return;

            int32 gain = target->ModifyHealth(m_modifier.m_amount);
            if (Unit *caster = GetAffectiveCaster())
                target->getHostileRefManager().threatAssist(caster, float(gain) * 0.5f  * sSpellMgr.GetSpellThreatMultiplier(spellProto), spellProto);
            break;
        }
        case SPELL_AURA_MOD_POWER_REGEN:
        {
            // don't energize target if not alive, possible death persistent effects
            if (!target->isAlive())
                return;

            // don't energize isolated units (banished)
            if (target->hasUnitState(UNIT_STAT_ISOLATED))
                return;

            Powers pt = target->getPowerType();
            if (int32(pt) != m_modifier.m_miscvalue)
                return;

            if ( spellProto->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED )
            {
                // eating anim
                target->HandleEmoteCommand(EMOTE_ONESHOT_EAT);
            }
            // Anger Management
            // amount = 1+ 16 = 17 = 3,4*5 = 10,2*5/3
            // so 17 is rounded amount for 5 sec tick grow ~ 1 range grow in 3 sec
            if (pt == POWER_RAGE)
                target->ModifyPower(pt, m_modifier.m_amount * 3 / 5);
            break;
        }
        // Here tick dummy auras
        case SPELL_AURA_DUMMY:                              // some spells have dummy aura
        {
            PeriodicDummyTick();
            break;
        }
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        {
            TriggerSpell();
            break;
        }
        default:
            break;
    }
}

void Aura::PeriodicDummyTick()
{
    SpellEntry const* spell = GetSpellProto();
    Unit *target = GetTarget();
    switch (spell->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (spell->Id)
            {
                // Forsaken Skills
                case 7054:
                {
                    // Possibly need cast one of them (but
                    // 7038 Forsaken Skill: Swords
                    // 7039 Forsaken Skill: Axes
                    // 7040 Forsaken Skill: Daggers
                    // 7041 Forsaken Skill: Maces
                    // 7042 Forsaken Skill: Staves
                    // 7043 Forsaken Skill: Bows
                    // 7044 Forsaken Skill: Guns
                    // 7045 Forsaken Skill: 2H Axes
                    // 7046 Forsaken Skill: 2H Maces
                    // 7047 Forsaken Skill: 2H Swords
                    // 7048 Forsaken Skill: Defense
                    // 7049 Forsaken Skill: Fire
                    // 7050 Forsaken Skill: Frost
                    // 7051 Forsaken Skill: Holy
                    // 7053 Forsaken Skill: Shadow
                    return;
                }
                // Steal Flesh (The Culling of Stratholme - Salramm the Fleshcrafter)
                case 52708:
                {
                    if (Unit *caster = GetCaster())
                        caster->CastSpell(caster, 52712, true );

                    target->CastSpell(target, 52711, true);
                    return;
                }
                case 7057:                                  // Haunting Spirits
                    if (roll_chance_i(33))
                        target->CastSpell(target,m_modifier.m_amount,true,NULL,this);
                    return;
//              // Panda
//              case 19230: break;
//              // Gossip NPC Periodic - Talk
//              case 33208: break;
//              // Gossip NPC Periodic - Despawn
//              case 33209: break;
//              // Steal Weapon
//              case 36207: break;
//              // Simon Game START timer, (DND)
//              case 39993: break;
//              // Knockdown Fel Cannon: break; The Aggro Burst
//              case 40119: break;
//              // Old Mount Spell
//              case 40154: break;
//              // Magnetic Pull
//              case 40581: break;
//              // Ethereal Ring: break; The Bolt Burst
//              case 40801: break;
//              // Crystal Prison
//              case 40846: break;
//              // Copy Weapon
//              case 41054: break;
//              // Dementia
//              case 41404: break;
//              // Ethereal Ring Visual, Lightning Aura
//              case 41477: break;
//              // Ethereal Ring Visual, Lightning Aura (Fork)
//              case 41525: break;
//              // Ethereal Ring Visual, Lightning Jumper Aura
//              case 41567: break;
//              // No Man's Land
//              case 41955: break;
//              // Headless Horseman - Fire
//              case 42074: break;
//              // Headless Horseman - Visual - Large Fire
//              case 42075: break;
//              // Headless Horseman - Start Fire, Periodic Aura
//              case 42140: break;
//              // Ram Speed Boost
//              case 42152: break;
//              // Headless Horseman - Fires Out Victory Aura
//              case 42235: break;
//              // Pumpkin Life Cycle
//              case 42280: break;
//              // Brewfest Request Chick Chuck Mug Aura
//              case 42537: break;
//              // Squashling
//              case 42596: break;
//              // Headless Horseman Climax, Head: Periodic
//              case 42603: break;
                case 42621:                                 // Fire Bomb
                {
                    // Cast the summon spells (42622 to 42627) with increasing chance
                    uint32 rand = urand(0, 99);
                    for (uint32 i = 1; i <= 6; ++i)
                    {
                        if (rand < i * (i+1) /2 * 5)
                        {
                            target->CastSpell(target, spell->Id + i, true);
                            break;
                        }
                    }
                    break;
                }
//              // Headless Horseman - Conflagrate, Periodic Aura
//              case 42637: break;
//              // Headless Horseman - Create Pumpkin Treats Aura
//              case 42774: break;
//              // Headless Horseman Climax - Summoning Rhyme Aura
//              case 42879: break;
//              // Tricky Treat
//              case 42919: break;
//              // Giddyup!
//              case 42924: break;
//              // Ram - Trot
//              case 42992: break;
//              // Ram - Canter
//              case 42993: break;
//              // Ram - Gallop
//              case 42994: break;
//              // Ram Level - Neutral
//              case 43310: break;
//              // Headless Horseman - Maniacal Laugh, Maniacal, Delayed 17
//              case 43884: break;
//              // Wretched!
//              case 43963: break;
//              // Headless Horseman - Maniacal Laugh, Maniacal, other, Delayed 17
//              case 44000: break;
//              // Energy Feedback
//              case 44328: break;
//              // Romantic Picnic
//              case 45102: break;
//              // Romantic Picnic
//              case 45123: break;
//              // Looking for Love
//              case 45124: break;
//              // Kite - Lightning Strike Kite Aura
//              case 45197: break;
//              // Rocket Chicken
//              case 45202: break;
//              // Copy Offhand Weapon
//              case 45205: break;
//              // Upper Deck - Kite - Lightning Periodic Aura
//              case 45207: break;
//              // Kite -Sky  Lightning Strike Kite Aura
//              case 45251: break;
//              // Ribbon Pole Dancer Check Aura
//              case 45390: break;
//              // Holiday - Midsummer, Ribbon Pole Periodic Visual
//              case 45406: break;
//              // Parachute
//              case 45472: break;
//              // Alliance Flag, Extra Damage Debuff
//              case 45898: break;
//              // Horde Flag, Extra Damage Debuff
//              case 45899: break;
//              // Ahune - Summoning Rhyme Aura
//              case 45926: break;
//              // Ahune - Slippery Floor
//              case 45945: break;
//              // Ahune's Shield
//              case 45954: break;
//              // Nether Vapor Lightning
//              case 45960: break;
//              // Darkness
//              case 45996: break;
                case 46041:                                 // Summon Blood Elves Periodic
                    target->CastSpell(target, 46037, true, NULL, this);
                    target->CastSpell(target, roll_chance_i(50) ? 46038 : 46039, true, NULL, this);
                    target->CastSpell(target, 46040, true, NULL, this);
                    return;
//              // Transform Visual Missile Periodic
//              case 46205: break;
//              // Find Opening Beam End
//              case 46333: break;
//              // Ice Spear Control Aura
//              case 46371: break;
//              // Hailstone Chill
//              case 46458: break;
//              // Hailstone Chill, Internal
//              case 46465: break;
//              // Chill, Internal Shifter
//              case 46549: break;
//              // Summon Ice Spear Knockback Delayer
//              case 46878: break;
                case 47214: // Burninate Effect
                {
                    Unit * caster = GetCaster();
                    if (!caster)
                        return;

                    if (target->GetEntry() == 26570)
                    {
                        if (target->HasAura(54683, EFFECT_INDEX_0))
                            return;
                        else
                        {
                            // Credit Scourge
                            caster->CastSpell(caster, 47208, true);
                            // set ablaze
                            target->CastSpell(target, 54683, true);
                            ((Creature*)target)->ForcedDespawn(4000);
                        }
                    }
                    break;
                }
//              // Fizzcrank Practice Parachute
//              case 47228: break;
//              // Send Mug Control Aura
//              case 47369: break;
//              // Direbrew's Disarm (precast)
//              case 47407: break;
//              // Mole Machine Port Schedule
//              case 47489: break;
//              case 47941: break; // Crystal Spike
//              case 48200: break; // Healer Aura
                case 48630:                                 // Summon Gauntlet Mobs Periodic
                case 59275:                                 // Below may need some adjustment, pattern for amount of summon and where is not verified 100% (except for odd/even tick)
                {
                    bool chance = roll_chance_i(50);

                    target->CastSpell(target, chance ? 48631 : 48632, true, NULL, this);

                    if (GetAuraTicks() % 2)                 // which doctor at odd tick
                        target->CastSpell(target, chance ? 48636 : 48635, true, NULL, this);
                    else                                    // or harponeer, at even tick
                        target->CastSpell(target, chance ? 48634 : 48633, true, NULL, this);

                    return;
                }
//              case 49313: break; // Proximity Mine Area Aura
//              // Mole Machine Portal Schedule
//              case 49466: break;
                case 49555:                                 // Corpse Explode (Drak'tharon Keep - Trollgore)
                case 59807:                                 // Corpse Explode (heroic)
                {
                    if (GetAuraTicks() == 3 && target->GetTypeId() == TYPEID_UNIT)
                        ((Creature*)target)->ForcedDespawn();
                    if (GetAuraTicks() != 2)
                        return;

                    if (Unit* pCaster = GetCaster())
                        pCaster->CastSpell(target, spell->Id == 49555 ? 49618 : 59809, true);

                    return;
                }
//              case 49592: break; // Temporal Rift
//              case 49957: break; // Cutting Laser
//              case 50085: break; // Slow Fall
//              // Listening to Music
//              case 50493: break;
//              // Love Rocket Barrage
//              case 50530: break;
                case 50789:                                 // Summon iron dwarf (left or right)
                case 59860:
                    target->CastSpell(target, roll_chance_i(50) ? 50790 : 50791, true, NULL, this);
                    return;
                case 50792:                                 // Summon iron trogg (left or right)
                case 59859:
                    target->CastSpell(target, roll_chance_i(50) ? 50793 : 50794, true, NULL, this);
                    return;
                case 50801:                                 // Summon malformed ooze (left or right)
                case 59858:
                    target->CastSpell(target, roll_chance_i(50) ? 50802 : 50803, true, NULL, this);
                    return;
                case 50824:                                 // Summon earthen dwarf
                    target->CastSpell(target, roll_chance_i(50) ? 50825 : 50826, true, NULL, this);
                    return;
                case 52441:                                 // Cool Down
                    target->CastSpell(target, 52443, true);
                    return;
                case 53035:                                 // Summon Anub'ar Champion Periodic (Azjol Nerub)
                    target->CastSpell(target, 53014, true); // Summon Anub'ar Champion
                    return;
                case 53036:                                 // Summon Anub'ar Necromancer Periodic (Azjol Nerub)
                    target->CastSpell(target, 53015, true); // Summon Anub'ar Necromancer
                    return;
                case 53037:                                 // Summon Anub'ar Crypt Fiend Periodic (Azjol Nerub)
                    target->CastSpell(target, 53016, true); // Summon Anub'ar Crypt Fiend
                    return;
                case 53520:                                 // Carrion Beetles
                    target->CastSpell(target, 53521, true, NULL, this);
                    target->CastSpell(target, 53521, true, NULL, this);
                    return;
                case 55592:                                 // Clean
                    switch(urand(0,2))
                    {
                        case 0: target->CastSpell(target, 55731, true); break;
                        case 1: target->CastSpell(target, 55738, true); break;
                        case 2: target->CastSpell(target, 55739, true); break;
                    }
                    return;
                case 54798: // FLAMING Arrow Triggered Effect
                {
                    Unit * caster = GetCaster();
                    if (!caster)
                        return;

                    Player *rider = caster->GetCharmerOrOwnerPlayerOrPlayerItself();
                    if (!rider)
                        return;

                    if (target->GetEntry() == 29358)
                    {
                        if (target->HasAura(54683, EFFECT_INDEX_0))
                            return;
                        else
                        {
                            // Credit Frostworgs
                            rider->CastSpell(rider, 54896, true);
                            // set ablaze
                            target->CastSpell(target, 54683, true);
                            ((Creature*)target)->ForcedDespawn(6000);
                        }
                    }
                    else if (target->GetEntry() == 29351)
                    {
                        if (target->HasAura(54683, EFFECT_INDEX_0))
                            return;
                        else
                        {
                            // Credit Frost Giants
                            rider->CastSpell(rider, 54893, true);
                            // set ablaze
                            target->CastSpell(target, 54683, true);
                            ((Creature*)target)->ForcedDespawn(6000);
                        }
                    }

                    break;
                }
                case 62038: // Biting Cold (Ulduar: Hodir)
                {
                    if (target->GetTypeId() != TYPEID_PLAYER)
                        return;

                    Unit * caster = GetCaster();
                    if (!caster)
                        return;

                    if (!target->HasAura(62821))     // Toasty Fire
                    {
                        // dmg dealing every second
                        target->CastSpell(target, 62188, true, 0, 0, caster->GetObjectGuid());
                    }

                    // aura stack increase every 3 (data in m_miscvalue) seconds and decrease every 1s
                    // Reset reapply counter at move and decrease stack amount by 1
                    if (((Player*)target)->isMoving() || target->HasAura(62821))
                    {
                        if (SpellAuraHolderPtr holder = target->GetSpellAuraHolder(62039))
                        {
                            if (holder->ModStackAmount(-1))
                                target->RemoveSpellAuraHolder(holder);
                        }
                        m_modifier.m_miscvalue = 3;
                        return;
                    }
                    // We are standing at the moment, countdown
                    if (m_modifier.m_miscvalue > 0)
                    {
                        --m_modifier.m_miscvalue;
                        return;
                    }

                    target->CastSpell(target, 62039, true);

                    // recast every ~3 seconds
                    m_modifier.m_miscvalue = 3;
                    return;
                }
                case 62566:                                 // Healthy Spore Summon Periodic
                {
                    target->CastSpell(target, 62582, true);
                    target->CastSpell(target, 62591, true);
                    target->CastSpell(target, 62592, true);
                    target->CastSpell(target, 62593, true);
                    return;
                }
                case 62717:                                 // Slag Pot (periodic dmg)
                case 63477:
                {
                    Unit *caster = GetCaster();

                    if (caster && target)
                        caster->CastSpell(target, (spell->Id == 62717) ? 65722 : 65723, true, 0, this, this->GetCasterGuid(), this->GetSpellProto());
                    return;
                }
                case 63276:                                   // Mark of the Faceless (General Vezax - Ulduar)
                {

                    Unit *caster = GetCaster();

                    if (caster && target)
                        caster->CastCustomSpell(target, 63278, 0, &(spell->EffectBasePoints[0]), 0, false, 0, 0, caster->GetObjectGuid() , spell);
                    return;
                }
                case 69008:                                 // Soulstorm (OOC aura)
                case 68870:                                 // Soulstorm
                {
                    uint32 triggerSpells[8] = {68898, 68904, 68886, 68905, 68896, 68906, 68897, 68907};
                    target->CastSpell(target, triggerSpells[GetAuraTicks() % 8], true);
                    return;
                }
// Exist more after, need add later
                default:
                    break;
            }

            // Drink (item drink spells)
            if (GetEffIndex() > EFFECT_INDEX_0 && spell->EffectApplyAuraName[GetEffIndex()-1] == SPELL_AURA_MOD_POWER_REGEN)
            {
                if (target->GetTypeId() != TYPEID_PLAYER)
                    return;
                // Search SPELL_AURA_MOD_POWER_REGEN aura for this spell and add bonus
                if (Aura* aura = GetHolder()->GetAuraByEffectIndex(SpellEffectIndex(GetEffIndex() - 1)))
                {
                    aura->GetModifier()->m_amount = m_modifier.m_amount;
                    ((Player*)target)->UpdateManaRegen();
                    // Disable continue
                    m_isPeriodic = false;
                    return;
                }
                return;
            }

            // Prey on the Weak
            if (spell->SpellIconID == 2983)
            {
                Unit *victim = target->getVictim();
                if (victim && (target->GetHealth() * 100 / target->GetMaxHealth() > victim->GetHealth() * 100 / victim->GetMaxHealth()))
                {
                    if (!target->HasAura(58670))
                    {
                        int32 basepoints = GetBasePoints();
                        target->CastCustomSpell(target, 58670, &basepoints, 0, 0, true);
                    }
                }
                else
                    target->RemoveAurasDueToSpell(58670);
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // Mirror Image
//            if (spell->Id == 55342)
//                return;
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            switch (spell->Id)
            {
                // Frenzied Regeneration
                case 22842:
                {
                    // Converts up to 10 rage per second into health for $d.  Each point of rage is converted into ${$m2/10}.1% of max health.
                    // Should be manauser
                    if (target->getPowerType() != POWER_RAGE)
                        return;
                    uint32 rage = target->GetPower(POWER_RAGE);
                    // Nothing todo
                    if (rage == 0)
                        return;
                    int32 mod = (rage < 100) ? rage : 100;
                    int32 points = target->CalculateSpellDamage(target, spell, EFFECT_INDEX_1);
                    int32 regen = target->GetMaxHealth() * (mod * points / 10) / 1000;
                    target->CastCustomSpell(target, 22845, &regen, NULL, NULL, true, NULL, this);
                    target->SetPower(POWER_RAGE, rage-mod);
                    return;
                }
                // Force of Nature
                case 33831:
                    return;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_WARLOCK:
            switch (spell->Id)
            {
                case 48018:
                {
                    GameObject* obj = target->GetGameObject(spell->Id);
                    if (!obj)
                    {
                         target->RemoveAurasDueToSpell(spell->Id);
                         target->RemoveAurasDueToSpell(62388);
                         return;
                    }
                    // We must take a range of teleport spell, not summon.
                    const SpellEntry* goToCircleSpell = sSpellStore.LookupEntry(48020);
                    if (target->IsWithinDist(obj,GetSpellMaxRange(sSpellRangeStore.LookupEntry(goToCircleSpell->rangeIndex))))
                        target->CastSpell(target, 62388, true);
                    else
                        target->RemoveAurasDueToSpell(62388);
                    break;
                }
                default:
                    break;
            }
            break;
        case SPELLFAMILY_ROGUE:
        {
            switch (spell->Id)
            {
                // Killing Spree
                case 51690:
                {
                    if (target->hasUnitState(UNIT_STAT_STUNNED) || target->isFeared())
                        return;

                    Spell::UnitList targets;
                    {
                        // eff_radius ==0
                        float radius = GetSpellMaxRange(sSpellRangeStore.LookupEntry(spell->rangeIndex));

                        MaNGOS::AnyUnfriendlyVisibleUnitInObjectRangeCheck u_check(target, target, radius);
                        MaNGOS::UnitListSearcher<MaNGOS::AnyUnfriendlyVisibleUnitInObjectRangeCheck> checker(targets, u_check);
                        Cell::VisitAllObjects(target, checker, radius);
                    }

                    if (targets.empty())
                        return;

                    Spell::UnitList::const_iterator itr = targets.begin();
                    std::advance(itr, rand()%targets.size());
                    Unit* victim = *itr;

                    target->CastSpell(victim, 57840, true);
                    target->CastSpell(victim, 57841, true);
                    return;
                }
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Explosive Shot
            if (spell->GetSpellFamilyFlags().test<CF_HUNTER_EXPLOSIVE_SHOT>())
            {
                target->CastCustomSpell(target, 53352, &m_modifier.m_amount, 0, 0, true, 0, this, GetCasterGuid());
                return;
            }
            switch (spell->Id)
            {
                // Harpooner's Mark
                // case 40084:
                //    return;
                // Feeding Frenzy Rank 1 & 2
                case 53511:
                case 53512:
                {
                    Unit* victim = target->getVictim();
                    if ( victim && victim->GetHealth() * 100 < victim->GetMaxHealth() * 35 )
                        target->CastSpell(target, spell->Id == 53511 ? 60096 : 60097, true, NULL, this);
                    return;
                }
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            // Astral Shift
            if (spell->Id == 52179)
            {
                // Periodic need for remove visual on stun/fear/silence lost
                if (!target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED | UNIT_FLAG_FLEEING | UNIT_FLAG_SILENCED))
                    target->RemoveAurasDueToSpell(52179);
                return;
            }
            break;
        }
        default:
            break;
    }
}

void Aura::PeriodicCheck()
{

    SpellAuraHolderPtr holder = GetHolder();
    SpellEntry const* spellProto = GetSpellProto();

    if (!holder || !spellProto)
        return;

    Unit* target = GetTarget();
    Unit* caster = GetCaster();

    if (!caster || !target)
        return;

    switch(m_modifier.m_auraname)
    {
        case SPELL_AURA_MOD_CONFUSE:
        case SPELL_AURA_MOD_FEAR:
        case SPELL_AURA_MOD_STUN:
        case SPELL_AURA_MOD_ROOT:
        case SPELL_AURA_TRANSFORM:
        {
            if (caster->GetObjectGuid().IsPlayer() && target->GetObjectGuid().IsCreature())
            {
                if (caster->MagicSpellHitResult(target, spellProto) != SPELL_MISS_NONE)
                {
                    caster->SendSpellDamageResist(target, spellProto->Id);
                    target->RemoveAurasDueToSpell(GetId());
                }
            }
            break;
        }
        default:
            break;
    }
}

void Aura::HandlePreventFleeing(bool apply, bool Real)
{
    if (!Real)
        return;

    Unit::AuraList const& fearAuras = GetTarget()->GetAurasByType(SPELL_AURA_MOD_FEAR);
    if (!fearAuras.empty())
    {
        if (apply)
            GetTarget()->SetFeared(false, fearAuras.front()->GetCasterGuid());
        else
            GetTarget()->SetFeared(true);
    }
}

void Aura::HandleManaShield(bool apply, bool Real)
{
    if (!Real)
        return;

    // prevent double apply bonuses
    if (apply && (GetTarget()->GetTypeId()!=TYPEID_PLAYER || !((Player*)GetTarget())->GetSession()->PlayerLoading()))
    {
        if (Unit* caster = GetCaster())
        {
            float DoneActualBenefit = 0.0f;
            switch(GetSpellProto()->SpellFamilyName)
            {
                case SPELLFAMILY_MAGE:
                    if (GetSpellProto()->GetSpellFamilyFlags().test<CF_MAGE_MANA_SHIELD>())
                    {
                        // Mana Shield
                        // +50% from +spd bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonusDone(GetSpellSchoolMask(GetSpellProto())) * 0.5f;
                        break;
                    }
                    break;
                default:
                    break;
            }

            DoneActualBenefit *= caster->CalculateLevelPenalty(GetSpellProto());

            m_modifier.m_amount += (int32)DoneActualBenefit;
        }
    }
}

void Aura::HandleAuraSafeFall( bool Apply, bool Real )
{
    // implemented in WorldSession::HandleMovementOpcodes

    // only special case
    if (Apply && Real && GetId() == 32474 && GetTarget()->GetTypeId() == TYPEID_PLAYER)
        ((Player*)GetTarget())->ActivateTaxiPathTo(506, GetId());
}

bool Aura::IsCritFromAbilityAura(Unit* caster, DamageInfo* damageInfo)
{
    if (!GetSpellProto()->IsFitToFamily<SPELLFAMILY_ROGUE, CF_ROGUE_RUPTURE>() && // Rupture
        !GetSpellProto()->IsFitToFamily<SPELLFAMILY_ROGUE, CF_ROGUE_DEADLY_POISON>())
        return false;

    if (caster->IsSpellCrit(GetTarget(), GetSpellProto(), GetSpellSchoolMask(GetSpellProto())))
    {
        damageInfo->damage = caster->SpellCriticalDamageBonus(GetSpellProto(), damageInfo->damage, GetTarget());
        return true;
    }

    return false;
}

bool Aura::IsLastAuraOnHolder()
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (i != GetEffIndex() && GetHolder()->GetAuraByEffectIndex(SpellEffectIndex(i)))
            return false;
    return true;
}

bool Aura::HasMechanic(uint32 mechanic) const
{
    return GetSpellProto()->Mechanic == mechanic ||
        GetSpellProto()->EffectMechanic[m_effIndex] == mechanic;
}


SpellAuraHolder::SpellAuraHolder(SpellEntry const* spellproto, Unit *target, WorldObject *caster, Item *castItem) :
m_spellProto(spellproto), m_target(target), m_castItemGuid(castItem ? castItem->GetObjectGuid() : ObjectGuid()),
m_auraSlot(MAX_AURAS), m_auraFlags(AFLAG_NONE), m_auraLevel(1), m_procCharges(0),
m_stackAmount(1), m_timeCla(1000), m_removeMode(AURA_REMOVE_BY_DEFAULT), m_AuraDRGroup(DIMINISHING_NONE),
m_permanent(false), m_isRemovedOnShapeLost(true), m_deleted(false), m_in_use(0)
{
    MANGOS_ASSERT(target);
    MANGOS_ASSERT(spellproto && spellproto == sSpellStore.LookupEntry( spellproto->Id ) && "`info` must be pointer to sSpellStore element");
    m_aurasStorage.clear();

    if (!caster)
        m_casterGuid = target->GetObjectGuid();
    else
    {
        // remove this assert when not unit casters will be supported
        MANGOS_ASSERT(caster->isType(TYPEMASK_UNIT))
        m_casterGuid = caster->GetObjectGuid();
    }

    m_originalCasterGuid = m_casterGuid;  // Must be setted after by special case

    m_applyTime      = time(NULL);
    m_isPassive      = IsPassiveSpell(spellproto);
    m_isDeathPersist = IsDeathPersistentSpell(spellproto);
    m_isSingleTarget = IsSingleTargetSpell(spellproto);
    m_procCharges    = spellproto->procCharges;

    m_isRemovedOnShapeLost = (GetCasterGuid() == m_target->GetObjectGuid() &&
                              m_spellProto->Stances &&
                              !m_spellProto->HasAttribute(SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT) &&
                              !m_spellProto->HasAttribute(SPELL_ATTR_NOT_SHAPESHIFT));

    Unit* unitCaster = caster && caster->isType(TYPEMASK_UNIT) ? (Unit*)caster : NULL;

    m_duration = m_maxDuration = CalculateSpellDuration(spellproto, unitCaster);

    if (m_maxDuration == -1 || (m_isPassive && spellproto->DurationIndex == 0))
        m_permanent = true;

    if (unitCaster)
    {
        if (Player* modOwner = unitCaster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_CHARGES, m_procCharges);
    }

    // some custom stack values at aura holder create
    switch (m_spellProto->Id)
    {
        // some auras applied with max stack
        case 24575:                                         // Brittle Armor
        case 24659:                                         // Unstable Power
        case 24662:                                         // Restless Strength
        case 26464:                                         // Mercurial Shield
        case 34027:                                         // Kill Command
        case 53257:                                         // Cobra strike
        case 55166:                                         // Tidal Force
        case 58914:                                         // Kill Command (pet part)
        case 62519:                                         // Attuned to Nature
        case 63050:                                         // Sanity (Ulduar - Yogg Saron)
        case 64455:                                         // Feral Essence
        case 66228:                                         // Nether Power (ToC: Lord Jaraxxus)
        case 67106:                                         // Nether Power (ToC: Lord Jaraxxus)
        case 67107:                                         // Nether Power (ToC: Lord Jaraxxus)
        case 67108:                                         // Nether Power (ToC: Lord Jaraxxus)
        case 71564:                                         // Deadly Precision
        case 74396:                                         // Fingers of Frost
        case 70672:                                         // Gaseous Bloat (Putricide)
            m_stackAmount = m_spellProto->StackAmount;
            break;
    }
}

void SpellAuraHolder::AddAura(Aura const& aura, SpellEffectIndex index)
{
    if (/*Aura* _aura = */GetAuraByEffectIndex(index))
    {
        DEBUG_LOG("SpellAuraHolder::AddAura attempt to add aura (effect %u) to holder of spell %u, but holder already have active aura!", index, GetId());
        RemoveAura(index);
    }
    if (!m_aurasStorage.empty())
    {
        AuraStorage::iterator itr = m_aurasStorage.find(index);
        if (itr != m_aurasStorage.end())
        {
            MAPLOCK_WRITE(m_target, MAP_LOCK_TYPE_AURAS);
            m_aurasStorage.erase(itr);
        }
    }

    m_aurasStorage.insert(AuraStorage::value_type(index,aura));
    m_auraFlags |= (1 << index);
}

void SpellAuraHolder::RemoveAura(SpellEffectIndex index)
{
    m_auraFlags &= ~(1 << index);
}

void SpellAuraHolder::CleanupsBeforeDelete()
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        RemoveAura(SpellEffectIndex(i));

    if (!m_aurasStorage.empty())
        m_aurasStorage.clear();
}

Aura* SpellAuraHolder::GetAuraByEffectIndex(SpellEffectIndex index)
{
    if ((m_auraFlags & (1 << index)) && !m_aurasStorage.empty())
    {
        AuraStorage::iterator itr = m_aurasStorage.find(index);
        if (itr != m_aurasStorage.end())
            return &itr->second;
    }
    return (Aura*)NULL;
}

Aura const* SpellAuraHolder::GetAura(SpellEffectIndex index) const
{
    if ((m_auraFlags & (1 << index)) && !m_aurasStorage.empty())
    {
        AuraStorage::const_iterator itr = m_aurasStorage.find(index);
        if (itr != m_aurasStorage.end())
            return &itr->second;
    }
    return (Aura*)NULL;
}

void SpellAuraHolder::ApplyAuraModifiers(bool apply, bool real)
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX && !IsDeleted(); ++i)
        if (Aura *aur = GetAuraByEffectIndex(SpellEffectIndex(i)))
            aur->ApplyModifier(apply, real);
}

void SpellAuraHolder::_AddSpellAuraHolder()
{
    if (!GetId())
        return;

    if (!m_target)
        return;

    // Try find slot for aura
    uint8 slot = NULL_AURA_SLOT;

    // Lookup free slot
    if (m_target->GetVisibleAurasCount() < MAX_AURAS)
    {
        Unit::VisibleAuraMap const& visibleAuras = m_target->GetVisibleAuras();
        for(uint8 i = 0; i < MAX_AURAS; ++i)
        {
            Unit::VisibleAuraMap::const_iterator itr = visibleAuras.find(i);
            if (itr == visibleAuras.end())
            {
                slot = i;
                // update for out of range group members (on 1 slot use)
                m_target->UpdateAuraForGroup(slot);
                break;
            }
        }
    }

    Unit* caster = GetCaster();

    // set infinity cooldown state for spells
    if (caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (m_spellProto->HasAttribute(SPELL_ATTR_DISABLED_WHILE_ACTIVE))
        {
            Item* castItem = m_castItemGuid ? ((Player*)caster)->GetItemByGuid(m_castItemGuid) : NULL;
            ((Player*)caster)->AddSpellAndCategoryCooldowns(m_spellProto,castItem ? castItem->GetEntry() : 0,true);
        }
    }

    uint8 flags = GetAuraFlags() | ((GetCasterGuid() == GetTarget()->GetObjectGuid()) ? AFLAG_NOT_CASTER : AFLAG_NONE)/* | ((GetSpellMaxDuration(m_spellProto) > 0 && !m_spellProto->HasAttribute(SPELL_ATTR_EX5_NO_DURATION)) ? AFLAG_DURATION : AFLAG_NONE)*/ | (IsPositive() ? AFLAG_POSITIVE : AFLAG_NEGATIVE);
    SetAuraFlags(flags);

    SetAuraLevel(caster ? caster->getLevel() : sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL));

    if (IsNeedVisibleSlot(caster))
    {
        SetAuraSlot( slot );
        if (slot < MAX_AURAS)                        // slot found send data to client
        {
            SetVisibleAura(false);
            SendAuraUpdate(false);
        }

        //*****************************************************
        // Update target aura state flag on holder apply
        // TODO: Make it easer
        //*****************************************************

        // Sitdown on apply aura req seated
        if ((m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED) && !m_target->IsSitState())
            m_target->SetStandState(UNIT_STAND_STATE_SIT);

        // register aura diminishing on apply
        if (getDiminishGroup() != DIMINISHING_NONE )
            m_target->ApplyDiminishingAura(getDiminishGroup(), true);

        // Update Seals information
        if (IsSealSpell(m_spellProto))
            m_target->ModifyAuraState(AURA_STATE_JUDGEMENT, true);

        // Conflagrate aura state on Immolate and Shadowflame
        if (m_spellProto->IsFitToFamily<SPELLFAMILY_WARLOCK, CF_WARLOCK_IMMOLATE, CF_WARLOCK_SHADOWFLAME2>())
            m_target->ModifyAuraState(AURA_STATE_CONFLAGRATE, true);

        // Faerie Fire (druid versions)
        if (m_spellProto->IsFitToFamily<SPELLFAMILY_DRUID, CF_DRUID_FAERIE_FIRE>())
            m_target->ModifyAuraState(AURA_STATE_FAERIE_FIRE, true);

        // Sting (hunter's pet ability)
        if (m_spellProto->Category == 1133)
            m_target->ModifyAuraState(AURA_STATE_FAERIE_FIRE, true);

        // Victorious
        if (m_spellProto->IsFitToFamily<SPELLFAMILY_WARRIOR, CF_WARRIOR_VICTORIOUS>())
            m_target->ModifyAuraState(AURA_STATE_WARRIOR_VICTORY_RUSH, true);

        // Swiftmend state on Regrowth & Rejuvenation
        if (m_spellProto->IsFitToFamily<SPELLFAMILY_DRUID, CF_DRUID_REJUVENATION, CF_DRUID_REGROWTH>())
            m_target->ModifyAuraState(AURA_STATE_SWIFTMEND, true);

        // Deadly poison aura state
        if (m_spellProto->IsFitToFamily<SPELLFAMILY_ROGUE, CF_ROGUE_DEADLY_POISON>())
            m_target->ModifyAuraState(AURA_STATE_DEADLY_POISON, true);

        // Enrage aura state
        if (m_spellProto->Dispel == DISPEL_ENRAGE)
            m_target->ModifyAuraState(AURA_STATE_ENRAGE, true);

        // Bleeding aura state
        if (GetAllSpellMechanicMask(m_spellProto) & (1 << (MECHANIC_BLEED-1)))
            m_target->ModifyAuraState(AURA_STATE_BLEEDING, true);

        switch(m_spellProto->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
                {
                    if (GetSpellSchoolMask(m_spellProto) == (SPELL_SCHOOL_MASK_ARCANE | SPELL_SCHOOL_MASK_HOLY))
                        m_target->ModifyAuraState(AURA_STATE_LIGHT_TARGET, true);
                    else if (GetSpellSchoolMask(m_spellProto) == (SPELL_SCHOOL_MASK_ARCANE | SPELL_SCHOOL_MASK_SHADOW))
                        m_target->ModifyAuraState(AURA_STATE_DARK_TARGET, true);
                    break;
                }
            default:
                break;
        }
    }
}

void SpellAuraHolder::_RemoveSpellAuraHolder()
{
    // Remove all triggered by aura spells vs unlimited duration
    // except same aura replace case
    if (m_removeMode!=AURA_REMOVE_BY_STACK)
        CleanupTriggeredSpells();

    Unit* caster = GetCaster();

    if (caster && IsPersistent())
        if (DynamicObject *dynObj = caster->GetDynObject(GetId()))
            dynObj->RemoveAffected(m_target);

    // remove at-store spell cast items (for all remove modes?)
    if (m_target->GetTypeId() == TYPEID_PLAYER && m_removeMode != AURA_REMOVE_BY_DEFAULT && m_removeMode != AURA_REMOVE_BY_DELETE)
        if (ObjectGuid castItemGuid = GetCastItemGuid())
            if (Item* castItem = ((Player*)m_target)->GetItemByGuid(castItemGuid))
                ((Player*)m_target)->DestroyItemWithOnStoreSpell(castItem, GetId());

    // reset cooldown state for spells
    if (caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (GetSpellProto()->Attributes & SPELL_ATTR_DISABLED_WHILE_ACTIVE)
            // note: item based cooldowns and cooldown spell mods with charges ignored (unknown existing cases)
            ((Player*)caster)->SendCooldownEvent(GetSpellProto());
    }

    //passive auras do not get put in slots - said who? ;)
    // Note: but totem can be not accessible for aura target in time remove (to far for find in grid)
    //if (m_isPassive && !(caster && caster->GetTypeId() == TYPEID_UNIT && ((Creature*)caster)->IsTotem()))
    //    return;

    uint8 slot = GetAuraSlot();

    if (slot >= MAX_AURAS)                                   // slot not set
        return;

    if (!m_target->GetVisibleAura(slot))
        return;

    // unregister aura diminishing (and store last time)
    if (getDiminishGroup() != DIMINISHING_NONE )
        m_target->ApplyDiminishingAura(getDiminishGroup(), false);

    SetAuraFlags(AFLAG_NONE);
    SetAuraLevel(0);
    SetVisibleAura(true);

    if (m_removeMode != AURA_REMOVE_BY_DELETE)
    {
        SendAuraUpdate(true);

        // update for out of range group members
        m_target->UpdateAuraForGroup(slot);

        //*****************************************************
        // Update target aura state flag (at last aura remove)
        //*****************************************************
        // Enrage aura state
        if (m_spellProto->Dispel == DISPEL_ENRAGE)
            m_target->ModifyAuraState(AURA_STATE_ENRAGE, false);

        // Bleeding aura state
        if (GetAllSpellMechanicMask(m_spellProto) & (1 << (MECHANIC_BLEED-1)))
        {
            bool found = false;

            Unit::SpellAuraHolderMap const& holders = m_target->GetSpellAuraHolderMap();
            for (Unit::SpellAuraHolderMap::const_iterator itr = holders.begin(); itr != holders.end(); ++itr)
            {
                if (GetAllSpellMechanicMask(itr->second->GetSpellProto()) & (1 << (MECHANIC_BLEED-1)))
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                m_target->ModifyAuraState(AURA_STATE_BLEEDING, false);
        }

        uint32 removeState = 0;
        ClassFamilyMask removeFamilyFlag = m_spellProto->GetSpellFamilyFlags();
        switch(m_spellProto->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
                {
                    if (GetSpellSchoolMask(m_spellProto) == (SPELL_SCHOOL_MASK_ARCANE | SPELL_SCHOOL_MASK_HOLY))
                        removeState = AURA_STATE_LIGHT_TARGET;
                    else if (GetSpellSchoolMask(m_spellProto) == (SPELL_SCHOOL_MASK_ARCANE | SPELL_SCHOOL_MASK_SHADOW))
                        removeState = AURA_STATE_DARK_TARGET;
                    else if (GetSpellSchoolMask(m_spellProto) == (SPELL_SCHOOL_MASK_ARCANE | SPELL_SCHOOL_MASK_FIRE))
                        removeState = AURA_STATE_SPELLFIRE;
                }
                break;
            case SPELLFAMILY_PALADIN:
                if (IsSealSpell(m_spellProto))
                    removeState = AURA_STATE_JUDGEMENT;     // Update Seals information
                break;
            case SPELLFAMILY_WARLOCK:
                // Conflagrate aura state on Immolate and Shadowflame,
                if (m_spellProto->GetSpellFamilyFlags().test<CF_WARLOCK_IMMOLATE, CF_WARLOCK_SHADOWFLAME2>())
                {
                    removeFamilyFlag = ClassFamilyMask::create<CF_WARLOCK_IMMOLATE, CF_WARLOCK_SHADOWFLAME2>();
                    removeState = AURA_STATE_CONFLAGRATE;
                }
                break;
            case SPELLFAMILY_DRUID:
                if (m_spellProto->GetSpellFamilyFlags().test<CF_DRUID_FAERIE_FIRE>())
                    removeState = AURA_STATE_FAERIE_FIRE;   // Faerie Fire (druid versions)
                else if (m_spellProto->GetSpellFamilyFlags().test<CF_DRUID_REJUVENATION, CF_DRUID_REGROWTH>())
                {
                    removeFamilyFlag = ClassFamilyMask::create<CF_DRUID_REJUVENATION, CF_DRUID_REGROWTH>();
                    removeState = AURA_STATE_SWIFTMEND;     // Swiftmend aura state
                }
                break;
            case SPELLFAMILY_WARRIOR:
                if (m_spellProto->GetSpellFamilyFlags().test<CF_WARRIOR_VICTORIOUS>())
                    removeState = AURA_STATE_WARRIOR_VICTORY_RUSH; // Victorious
                break;
            case SPELLFAMILY_ROGUE:
                if (m_spellProto->GetSpellFamilyFlags().test<CF_ROGUE_DEADLY_POISON>())
                    removeState = AURA_STATE_DEADLY_POISON; // Deadly poison aura state
                break;
            case SPELLFAMILY_HUNTER:
                if (m_spellProto->GetSpellFamilyFlags().test<CF_HUNTER_PET_SPELLS>())
                    removeState = AURA_STATE_FAERIE_FIRE;   // Sting (hunter versions)
                break;
            default:
                break;
        }

        // Remove state (but need check other auras for it)
        if (removeState)
        {
            bool found = false;
            Unit::SpellAuraHolderMap const& holders = m_target->GetSpellAuraHolderMap();
            for (Unit::SpellAuraHolderMap::const_iterator i = holders.begin(); i != holders.end(); ++i)
            {
                SpellEntry const *auraSpellInfo = (*i).second->GetSpellProto();
                if (auraSpellInfo->IsFitToFamily(SpellFamily(m_spellProto->SpellFamilyName), removeFamilyFlag))
                {
                    found = true;
                    break;
                }
            }

            // this has been last aura
            if (!found)
                m_target->ModifyAuraState(AuraState(removeState), false);
        }

    }
}

void SpellAuraHolder::CleanupTriggeredSpells()
{
    if (!m_spellProto)
        return;

    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (!m_spellProto->EffectApplyAuraName[i])
            continue;

        uint32 tSpellId = m_spellProto->EffectTriggerSpell[i];
        if (!tSpellId)
            continue;

        SpellEntry const* tProto = sSpellStore.LookupEntry(tSpellId);
        if (!tProto)
            continue;

        if (GetSpellDuration(tProto) != -1)
            continue;

        // needed for spell 43680, maybe others
        // TODO: is there a spell flag, which can solve this in a more sophisticated way?
        if (m_spellProto->EffectApplyAuraName[i] == SPELL_AURA_PERIODIC_TRIGGER_SPELL &&
            GetSpellDuration(m_spellProto) == int32(m_spellProto->EffectAmplitude[i]))
            continue;

        m_target->RemoveAurasDueToSpell(tSpellId);
    }
}

bool SpellAuraHolder::ModStackAmount(int32 num)
{
    uint32 protoStackAmount = m_spellProto->StackAmount;

    // Can`t mod
    if (!protoStackAmount)
        return true;

    if (num != 0)
        HandleSpellSpecificBoostsForward(num > 0);

    // Modify stack but limit it
    int32 stackAmount = m_stackAmount + num;
    if (stackAmount > (int32)protoStackAmount)
        stackAmount = protoStackAmount;
    else if (stackAmount <=0) // Last aura from stack removed
    {
        m_stackAmount = 0;
        return true; // need remove aura
    }

    // Update stack amount
    SetStackAmount(stackAmount);
    return false;
}

void SpellAuraHolder::SetStackAmount(uint32 stackAmount)
{
    Unit *target = GetTarget();
    Unit *caster = GetCaster();
    if (!target || !caster)
        return;

    bool refresh = stackAmount >= m_stackAmount;
    if (stackAmount != m_stackAmount)
    {
        m_stackAmount = stackAmount;

        for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (Aura* aur = GetAuraByEffectIndex(SpellEffectIndex(i)))
            {
                int32 bp = aur->GetBasePoints();
                int32 amount = m_stackAmount * caster->CalculateSpellDamage(target, m_spellProto, SpellEffectIndex(i), &bp);
                // Reapply if amount change
                if (amount != aur->GetModifier()->m_amount)
                {
                    aur->ApplyModifier(false, true);
                    aur->GetModifier()->m_amount = amount;
                    aur->ApplyModifier(true, true);

                    // change duration if aura refreshes
                    if (refresh)
                    {
                        int32 maxduration = GetSpellMaxDuration(aur->GetSpellProto());
                        int32 duration = GetSpellDuration(aur->GetSpellProto());

                        // new duration based on combo points
                        if (duration != maxduration)
                        {
                            if (Unit *caster = aur->GetCaster())
                            {
                                duration += int32((maxduration - duration) * caster->GetComboPoints() / 5);
                                SetAuraMaxDuration(duration);
                                SetAuraDuration(duration);
                                refresh = false;
                            }
                        }
                    }
                }
            }
        }
    }

    if (refresh)
        // Stack increased refresh duration
        RefreshHolder();
    else
        // Stack decreased only send update
        SendAuraUpdate(false);
}

Unit* SpellAuraHolder::GetCaster() const
{
    if (!m_target)
        return NULL;

    if (GetCasterGuid().IsEmpty())
        return NULL;

    if (m_target->IsInWorld())
        if (GetCasterGuid() == m_target->GetObjectGuid())
            return m_target;

    return ObjectAccessor::GetUnit(*m_target, m_casterGuid);// player will search at any maps
}

Unit* SpellAuraHolder::GetAffectiveCaster() const
{
    if (!m_target)
        return NULL;

    return GetAffectiveCasterGuid() != GetCasterGuid() ?
        ObjectAccessor::GetUnit(*m_target, GetAffectiveCasterGuid()) :
        GetCaster();
}

void SpellAuraHolder::SetAffectiveCasterGuid(ObjectGuid guid)
{
    if (!guid || !guid.IsUnit() || !m_target)
        return;

    m_originalCasterGuid = guid;
    SetCasterGuid(m_target->GetObjectGuid());
};

bool SpellAuraHolder::IsWeaponBuffCoexistableWith() const
{
    // only item casted spells
    if (!GetCastItemGuid())
        return false;

    // Exclude Debuffs
    if (!IsPositive())
        return false;

    // Exclude Stackable Buffs [ie: Blood Reserve]
    if (GetSpellProto()->StackAmount)
        return false;

    // only self applied player buffs
    if (m_target->GetTypeId() != TYPEID_PLAYER || m_target->GetObjectGuid() != GetCasterGuid())
        return false;

    Item* castItem = ((Player*)m_target)->GetItemByGuid(GetCastItemGuid());
    if (!castItem)
        return false;

    // Limit to Weapon-Slots
    if (!castItem->IsEquipped() ||
        (castItem->GetSlot() != EQUIPMENT_SLOT_MAINHAND && castItem->GetSlot() != EQUIPMENT_SLOT_OFFHAND))
        return false;

    return true;
}

bool SpellAuraHolder::IsNeedVisibleSlot(Unit const* caster) const
{
    bool totemAura = caster && caster->GetTypeId() == TYPEID_UNIT && ((Creature*)caster)->IsTotem();

    if (m_spellProto->procFlags)
        return true;
    else if (HasAuraWithTriggerEffect(m_spellProto))
        return true;

    // passive auras (except totem auras) do not get placed in the slots
    return !m_isPassive || totemAura || HasAreaAuraEffect(m_spellProto);
}

void SpellAuraHolder::BuildUpdatePacket(WorldPacket& data) const
{
    data << uint8(GetAuraSlot());
    data << uint32(GetId());

    uint8 auraFlags = GetAuraFlags();
    data << uint8(auraFlags);
    data << uint8(GetAuraLevel());

    uint32 stackCount = m_procCharges ? m_procCharges*m_stackAmount : m_stackAmount;
    data << uint8(stackCount <= 255 ? stackCount : 255);

    if (!(auraFlags & AFLAG_NOT_CASTER))
    {
        data << GetAffectiveCasterGuid().WriteAsPacked();
    }

    if (auraFlags & AFLAG_DURATION)
    {
        data << uint32(GetAuraMaxDuration());
        data << uint32(GetAuraDuration());
    }
}

void SpellAuraHolder::SendAuraUpdate(bool remove) const
{
    WorldPacket data(SMSG_AURA_UPDATE);
    data << m_target->GetPackGUID();

    if (remove)
    {
        data << uint8(GetAuraSlot());
        data << uint32(0);
    }
    else
        BuildUpdatePacket(data);

    m_target->SendMessageToSet(&data, true);
}

void SpellAuraHolder::HandleSpellSpecificBoosts(bool apply)
{
    // it's impossible in theory, but possible at fact...
    if (!GetSpellProto())
        return;

    // if holder applyed in deleted case, need do nothing
    if (apply && IsDeleted())
    {
        sLog.outError("SpellAuraHolder::HandleSpellSpecificBoosts called `apply`  for deleted holder %u !", GetId());
        return;
    }

    bool cast_at_remove = false;                            // if spell must be casted at last aura from stack remove
    uint32 spellId1 = 0;
    uint32 spellId2 = 0;
    uint32 spellId3 = 0;
    uint32 spellId4 = 0;

    // Linked spells (boost chain)
    SpellLinkedSet linkedSet = sSpellMgr.GetSpellLinked(GetId(), SPELL_LINKED_TYPE_BOOST);
    if (linkedSet.size() > 0)
    {
        for (SpellLinkedSet::const_iterator itr = linkedSet.begin(); itr != linkedSet.end(); ++itr)
        {
            apply ?
                m_target->CastSpell(m_target, *itr, true, NULL, NULL, GetCasterGuid()) :
                m_target->RemoveAurasByCasterSpell(*itr, GetCasterGuid());
        }
    }

    // pet auras add (effects not checked! must be checked at load from DB)
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (PetAura const* petSpell = sSpellMgr.GetPetAura(GetId(), SpellEffectIndex(i)))
        {
            if (apply)
                m_target->AddPetAura(petSpell);
            else
                m_target->RemovePetAura(petSpell);
        }
    }

    linkedSet.clear();
    if (!apply)
    {
        // Linked spells (CastOnRemove chain)
        linkedSet = sSpellMgr.GetSpellLinked(GetId(), SPELL_LINKED_TYPE_CASTONREMOVE);
        if (linkedSet.size() > 0)
        {
            for (SpellLinkedSet::const_iterator itr = linkedSet.begin(); itr != linkedSet.end(); ++itr)
                m_target->CastSpell(m_target, *itr, true, NULL, NULL, GetCasterGuid());
        }

        // Linked spells (RemoveOnRemove chain)
        linkedSet = sSpellMgr.GetSpellLinked(GetId(), SPELL_LINKED_TYPE_REMOVEONREMOVE);
        if (linkedSet.size() > 0)
        {
            for (SpellLinkedSet::const_iterator itr = linkedSet.begin(); itr != linkedSet.end(); ++itr)
                m_target->RemoveAurasByCasterSpell(*itr, GetCasterGuid());
        }
    }
    linkedSet.clear();

    switch(GetSpellProto()->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            // Bandages
            if (GetSpellProto()->Mechanic == MECHANIC_BANDAGE && !apply)
            {
                cast_at_remove = true;
                spellId1 = 11196;                     // Recently Bandaged
                break;
            }

            switch(GetId())
            {
                case 29865:                                 // Deathbloom (10 man)
                {
                    if (!apply && m_removeMode == AURA_REMOVE_BY_EXPIRE)
                    {
                        cast_at_remove = true;
                        spellId1 = 55594;
                    }
                    else
                        return;
                    break;
                }
                default:
                    return;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // Ice Barrier (non stacking from one caster)
            if (m_spellProto->SpellIconID == 32)
            {
                if ((!apply && m_removeMode == AURA_REMOVE_BY_DISPEL) || m_removeMode == AURA_REMOVE_BY_SHIELD_BREAK)
                {
                    Unit::AuraList const& dummyAuras = m_target->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraList::const_iterator itr = dummyAuras.begin(); itr != dummyAuras.end(); ++itr)
                    {
                        // Shattered Barrier
                        if ((*itr)->GetSpellProto()->SpellIconID == 2945)
                        {
                            cast_at_remove = true;
                            // first rank have 50% chance
                            if ((*itr)->GetId() != 44745 || roll_chance_i(50))
                                spellId1 = 55080;
                            break;
                        }
                    }
                }
                else
                    return;
                break;
            }
            else if (!apply && m_spellProto->GetSpellFamilyFlags().test<CF_MAGE_ARCANE_MISSILES_CHANNEL>())
            {
                if (Unit * caster = GetCaster())
                {
                    // Remove missile barrage
                    if (SpellAuraHolderPtr holder = caster->GetSpellAuraHolder(44401, caster->GetObjectGuid()))
                        if (holder->DropAuraCharge())
                            caster->RemoveSpellAuraHolder(holder);

                    // Remove Arcane Blast
                    if (caster->HasAura(36032))
                        caster->RemoveAurasByCasterSpell(36032, caster->GetObjectGuid());
                }
            }

            switch(GetId())
            {
                case 11129:                                 // Combustion (remove triggered aura stack)
                {
                    if (!apply)
                        spellId1 = 28682;
                    else
                        return;
                    break;
                }
                case 28682:                                 // Combustion (remove main aura)
                {
                    if (!apply)
                        spellId1 = 11129;
                    else
                        return;
                    break;
                }
                case 44401:                                 // Missile Barrage (triggered)
                case 48108:                                 // Hot Streak (triggered)
                case 57761:                                 // Fireball! (Brain Freeze triggered)
                {
                    // consumed aura (at proc charges 0)
                    if (!apply && m_removeMode == AURA_REMOVE_BY_DEFAULT)
                    {
                        Unit* caster = GetCaster();
                        if (caster && caster->HasAura(70752))   // Item - Mage T10 2P Bonus
                        {
                            cast_at_remove = true;
                            spellId1 = 70753;                   // Pushing the Limit
                        }
                    }
                    else
                        return;
                    break;
                }
                default:
                    return;
            }
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            if (!apply)
            {
                // Remove Blood Frenzy only if target no longer has any Deep Wound or Rend (applying is handled by procs)
                if (GetSpellProto()->Mechanic == MECHANIC_BLEED)
                {

                    // If target still has one of Warrior's bleeds, do nothing
                    Unit::AuraList const& PeriodicDamage = m_target->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for(Unit::AuraList::const_iterator i = PeriodicDamage.begin(); i != PeriodicDamage.end(); ++i)
                        if ( (*i)->GetCasterGuid() == GetCasterGuid() &&
                            (*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_WARRIOR &&
                            (*i)->GetSpellProto()->Mechanic == MECHANIC_BLEED)
                            return;

                    spellId1 = 30069;                           // Blood Frenzy (Rank 1)
                    spellId2 = 30070;                           // Blood Frenzy (Rank 2)
                    break;
                }
                else if (GetId() == 44521 && m_target && m_target->GetTypeId() == TYPEID_PLAYER)
                {
                    Player* plr = (Player*)m_target;
                    plr->SetPower(POWER_MANA, plr->GetMaxPower(POWER_MANA));
                    plr->SetPower(POWER_RAGE, 0);
                    plr->SetPower(POWER_ENERGY, plr->GetMaxPower(POWER_ENERGY));
                    return;
                }
                return;
            }
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Fear (non stacking)
            if (m_spellProto->GetSpellFamilyFlags().test<CF_WARLOCK_FEAR>())
            {
                if (!apply)
                {
                    Unit* caster = GetCaster();
                    if(!caster)
                        return;

                    Unit::AuraList const& dummyAuras = caster->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraList::const_iterator itr = dummyAuras.begin(); itr != dummyAuras.end(); ++itr)
                    {
                        SpellEntry const* dummyEntry = (*itr)->GetSpellProto();
                        // Improved Fear
                        if (dummyEntry->SpellFamilyName == SPELLFAMILY_WARLOCK && dummyEntry->SpellIconID == 98)
                        {
                            cast_at_remove = true;
                            switch((*itr)->GetModifier()->m_amount)
                            {
                                // Rank 1
                                case 0: spellId1 = 60946; break;
                                // Rank 1
                                case 1: spellId1 = 60947; break;
                            }
                            break;
                        }
                    }
                }
                else
                    return;
            }
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            // Shadow Word: Pain (need visual check fro skip improvement talent) or Vampiric Touch
            if ((m_spellProto->SpellIconID == 234 && m_spellProto->SpellVisual) || m_spellProto->SpellIconID == 2213)
            {
                if (!apply && m_removeMode == AURA_REMOVE_BY_DISPEL)
                {
                    Unit* caster = GetCaster();
                    if(!caster)
                        return;

                    Unit::AuraList const& dummyAuras = caster->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraList::const_iterator itr = dummyAuras.begin(); itr != dummyAuras.end(); ++itr)
                    {
                        // Shadow Affinity
                        if ((*itr)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_PRIEST
                            && (*itr)->GetSpellProto()->SpellIconID == 178)
                        {
                            // custom cast code
                            int32 basepoints0 = (*itr)->GetModifier()->m_amount * caster->GetCreateMana() / 100;
                            caster->CastCustomSpell(caster, 64103, &basepoints0, NULL, NULL, true, NULL);
                            return;
                        }
                    }
                }
                else
                    return;
            }

            switch(GetId())
            {
                // Abolish Disease (remove 1 more poison effect with Body and Soul)
                case 552:
                {
                    if (apply)
                    {
                        int chance =0;
                        Unit::AuraList const& dummyAuras = m_target->GetAurasByType(SPELL_AURA_DUMMY);
                        for(Unit::AuraList::const_iterator itr = dummyAuras.begin(); itr != dummyAuras.end(); ++itr)
                        {
                            SpellEntry const* dummyEntry = (*itr)->GetSpellProto();
                            // Body and Soul (talent ranks)
                            if (dummyEntry->SpellFamilyName == SPELLFAMILY_PRIEST && dummyEntry->SpellIconID == 2218 &&
                                dummyEntry->SpellVisual==0)
                            {
                                chance = (*itr)->GetSpellProto()->CalculateSimpleValue(EFFECT_INDEX_1);
                                break;
                            }
                        }

                        if (roll_chance_i(chance))
                            spellId1 = 64134;               // Body and Soul (periodic dispel effect)
                    }
                    else
                        spellId1 = 64134;                   // Body and Soul (periodic dispel effect)
                    break;
                }
                // Dispersion mana reg and immunity
                case 47585:
                    spellId1 = 60069;                       // Dispersion
                    spellId2 = 63230;                       // Dispersion
                    break;
                default:
                    return;
            }
           break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Rejuvenation
            if (GetSpellProto()->GetSpellFamilyFlags().test<CF_DRUID_REJUVENATION>())
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (caster->HasAura(64760))                 // Item - Druid T8 Restoration 4P Bonus
                {
                    Aura* aura = GetAuraByEffectIndex(EFFECT_INDEX_0);
                    if (!aura)
                        return;

                    int32 heal = aura->GetModifier()->m_amount;
                    caster->CastCustomSpell(m_target, 64801, &heal, NULL, NULL, true, NULL);
                }
            }
            // Enrage (Druid Bear)
            else if (GetId() == 5229)
            {
                if (apply)
                {
                    if (m_target->HasAura(70726))           // Druid T10 Feral 4P Bonus
                        spellId1 = 70725;                   // Enraged Defense
                    else
                        return;
                }
                else
                {
                    spellId1 = 70725;
                    spellId2 = 51185;                       // King of the Jungle (Enrage damage aura)
                }
            }
            else
                return;
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            // remove debuf savage combat
            if (GetSpellProto()->GetSpellFamilyFlags().test<CF_ROGUE_CRIPPLING_POISON, CF_ROGUE_DEADLY_POISON,CF_ROGUE_WOUND_POISON, CF_ROGUE_POISON_UNK>())
            {
                // search poison
                bool found = false;
                if (m_target->HasAuraState(AURA_STATE_DEADLY_POISON))
                    found = true;
                else
                {
                    Unit::SpellAuraHolderMap const& auras = m_target->GetSpellAuraHolderMap();
                    for (Unit::SpellAuraHolderMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                    {
                        if (itr->second->GetSpellProto()->SpellFamilyName == SPELLFAMILY_ROGUE &&
                            itr->second->GetSpellProto()->Dispel == DISPEL_POISON)
                        {
                            found = true;
                            break;
                        }
                    }
                }

                if (!found)
                {
                    m_target->RemoveAurasDueToSpell(58684); // Savage Combat rank 1
                    m_target->RemoveAurasDueToSpell(58683); // Savage Combat rank 2
                }
            }
            else
                return;
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            switch (GetId())
            {
                case 13161:                                 // Aspect of the Beast
                {
                    if (Pet* pet = m_target->GetPet())
                    {
                        spellId1 = 61669;                   // Aspect of the Beast - pet part
                        if (!apply)
                        {
                            pet->RemoveAurasDueToSpell(spellId1);
                            return;
                        }
                    }
                    break;
                }
                case 19574:                                 // Bestial Wrath - immunity
                case 34471:                                 // The Beast Within - immunity
                {
                    spellId1 = 24395;
                    spellId2 = 24396;
                    spellId3 = 24397;
                    spellId4 = 26592;
                    break;
                }
                case 34027:                                 // Kill Command, owner aura (spellmods)
                {
                    if (m_target->HasAura(35029))       // Focused Fire, rank 1
                        spellId1 = 60110;               // Kill Command, Focused Fire rank 1 bonus
                    else if (m_target->HasAura(35030))  // Focused Fire, rank 2
                        spellId1 = 60113;               // Kill Command, Focused Fire rank 2 bonus
                    else
                        return;
                    break;
                }
                case 34074:                                 // Aspect of the Viper
                {
                    if (!apply || m_target->HasAura(60144)) // Viper Attack Speed
                        spellId1 = 61609;                   // Vicious Viper
                    else
                        return;
                    break;
                }
                case 34455:          // Ferocious inspiration and ranks
                    spellId1 = 75593;
                    break;
                case 34459:
                    spellId1 = 75446;
                    break;
                case 34460:
                    spellId1 = 75447;
                    break;
                default:
                    // Aspect of the Dragonhawk dodge
                    else if (GetSpellProto()->GetSpellFamilyFlags().test<CF_HUNTER_ASPECT_OF_THE_DRAGONHAWK>())
                    {
                        spellId1 = 61848;

                        // triggered spell have same category as main spell and cooldown
                        if (apply && m_target->GetTypeId()==TYPEID_PLAYER)
                            ((Player*)m_target)->RemoveSpellCooldown(61848);
                    }
                    else
                        return;
                    break;
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            if (m_spellProto->Id == 19746)                  // Aura Mastery (on Concentration Aura remove and apply)
            {
                Unit *caster = GetCaster();
                if (!caster)
                    return;

                if (apply && caster->HasAura(31821))
                    caster->CastSpell(caster, 64364, true, NULL);
                else if (!apply)
                    caster->RemoveAurasDueToSpell(64364);
            }
            if (m_spellProto->Id == 31821)                  // Aura Mastery (on Aura Mastery original buff remove)
            {
                Unit *caster = GetCaster();
                if (!caster)
                    return;

                if (apply && caster->HasAura(19746))
                    caster->CastSpell(caster, 64364, true, NULL);
                else if (!apply)
                    caster->RemoveAurasDueToSpell(64364);
            }
            if (m_spellProto->Id == 31884)                  // Avenging Wrath
            {
                if (!apply)
                    spellId1 = 57318;                       // Sanctified Wrath (triggered)
                else
                {
                    int32 percent = 0;
                    Unit::AuraList const& dummyAuras = m_target->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraList::const_iterator itr = dummyAuras.begin(); itr != dummyAuras.end(); ++itr)
                    {
                        if ((*itr)->GetSpellProto()->SpellIconID == 3029)
                        {
                            percent = (*itr)->GetModifier()->m_amount;
                            break;
                        }
                    }

                    // apply in special way
                    if (percent)
                    {
                        spellId1 = 57318;                    // Sanctified Wrath (triggered)
                        // prevent aura deletion, specially in multi-boost case
                        SetInUse(true);
                        m_target->CastCustomSpell(m_target, spellId1, &percent, &percent, NULL, true, NULL);
                        SetInUse(false);
                    }
                    return;
                }
                break;
            }

            // Only process on player casting paladin aura
            // all aura bonuses applied also in aura area effect way to caster
            if (GetCasterGuid() != m_target->GetObjectGuid() || !GetCasterGuid().IsPlayer())
                return;

            if (GetSpellSpecific(m_spellProto->Id) != SPELL_AURA)
                return;

            // Sanctified Retribution and Swift Retribution (they share one aura), but not Retribution Aura (already gets modded)
            if (!GetSpellProto()->GetSpellFamilyFlags().test<CF_PALADIN_RETRIBUTION_AURA>())
                spellId1 = 63531;                           // placeholder for talent spell mods
            // Improved Concentration Aura (auras bonus)
            spellId2 = 63510;                               // placeholder for talent spell mods
            // Improved Devotion Aura (auras bonus)
            spellId3 = 63514;                               // placeholder for talent spell mods
            break;
        }
        default:
            break;
    }

    // Old method override. need rewrite.
    if (spellId1)
        linkedSet.insert(spellId1);
    if (spellId2)
        linkedSet.insert(spellId2);
    if (spellId3)
        linkedSet.insert(spellId3);
    if (spellId4)
        linkedSet.insert(spellId4);

    if (linkedSet.size() > 0)
    {
        // prevent aura deletion, specially in multi-boost case
        SetInUse(true);
        for (SpellLinkedSet::const_iterator itr = linkedSet.begin(); itr != linkedSet.end(); ++itr)
        {
            (apply || cast_at_remove) ?
                m_target->CastSpell(m_target, *itr, true, NULL, NULL, GetCasterGuid()) :
                m_target->RemoveAurasByCasterSpell(*itr, GetCasterGuid());
        }
        SetInUse(false);
    }
}

void SpellAuraHolder::HandleSpellSpecificBoostsForward(bool apply)
{
    // in fact, possible call of this method for not fully created holder...
    if (!GetSpellProto())
        return;

    // if holder applyed in deleted case, need do nothing
    if (apply && IsDeleted())
    {
        sLog.outError("SpellAuraHolder::HandleSpellSpecificBoostsForward called `apply`  for deleted holder %u !", GetId());
        return;
    }

    // Custom proc system (proc before apply or before fade)
    Unit* pCaster = GetCaster();
    if (!pCaster)
        pCaster = m_target;

    if (!pCaster)
        return;

    uint32 procFlag = apply ? PROC_FLAG_ON_AURA_APPLY : PROC_FLAG_ON_AURA_FADE;
    uint32 procEx   = 0;
    switch (m_removeMode)
    {
        case AURA_REMOVE_BY_EXPIRE:
        case AURA_REMOVE_BY_DEFAULT:
            procEx   |= PROC_EX_EXPIRE;
            break;
        case AURA_REMOVE_BY_SHIELD_BREAK:
            procEx   |= PROC_EX_SHIELD_BREAK;
            break;
        case AURA_REMOVE_BY_DISPEL:
            procEx   |= PROC_EX_DISPEL;
            break;
        default:
            break;
    };

    pCaster->ProcDamageAndSpell(m_target, procFlag, PROC_FLAG_NONE, procEx, 0, GetWeaponAttackType(GetSpellProto()), GetSpellProto());

    // Linked spells (boostforward chain)
    SpellLinkedSet linkedSet = sSpellMgr.GetSpellLinked(GetId(), SPELL_LINKED_TYPE_BOOSTFORWARD);

    // Custom cases
    switch(GetSpellProto()->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch(GetId())
            {
                case 70337:                                 // Necrotic Plague (Lich King)
                case 73912:
                case 73913:
                case 73914:
                {
                    if (!apply)
                    {
                        GetTarget()->CastSpell(GetTarget(), 70338, true);
                        if (m_removeMode != AURA_REMOVE_BY_DISPEL)
                            GetTarget()->CastSpell(GetTarget(), 70338, true);
                    }
                    break;
                }
                case 70338:                                 // Necrotic Plague (stacking) (Lich King)
                case 73785:
                case 73786:
                case 73787:
                {
                    if (apply)
                    {
                        GetTarget()->CastSpell(GetTarget(), 74074, true); // Plague Siphon

                        if (Unit *caster = GetCaster())
                        {
                            if (SpellAuraHolderPtr holder = caster->GetSpellAuraHolder(GetId()))
                                SetStackAmount(holder->GetStackAmount());
                        }
                    }
                    else if (m_removeMode != AURA_REMOVE_BY_DEFAULT)
                    {
                        if (m_removeMode == AURA_REMOVE_BY_DISPEL && m_stackAmount > 1)
                            --m_stackAmount;
                        else
                            ++m_stackAmount;

                        GetTarget()->CastSpell(GetTarget(), 70338, true);
                    }
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Shadow embrace (healing reduction part)
            if (m_spellProto->GetSpellFamilyFlags().test<CF_WARLOCK_MISC_DEBUFFS>() && m_spellProto->SpellIconID == 2209)
            {
                switch(GetId())
                {
                    case 32386:
                        linkedSet.insert(60448);
                        break;
                    case 32388:
                        linkedSet.insert(60465);
                        break;
                    case 32389:
                        linkedSet.insert(60466);
                        break;
                    case 32390:
                        linkedSet.insert(60467);
                        break;
                    case 32391:
                        linkedSet.insert(60468);
                        break;
                    default:
                        break;
                }
                break;
            }
            // Improved Health Funnel (damage reducing part)
            else if (m_spellProto->GetSpellFamilyFlags().test<CF_WARLOCK_HEALTH_FUNNEL>())
            {
                if (pCaster->HasAura(18703))
                    linkedSet.insert(60955);
                else if (pCaster->HasAura(18704))
                    linkedSet.insert(60956);
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Cobra strike
            if (m_spellProto->Id == 53257)
            {
                if (m_target->GetObjectGuid().IsPet())
                {
                    if (!apply)
                        if (Unit* owner = ((Pet*)m_target)->GetOwner())
                            if (SpellAuraHolderPtr holder = owner->GetSpellAuraHolder(m_spellProto->Id))
                                if (holder->ModStackAmount(-1))
                                    owner->RemoveSpellAuraHolder(holder);
                }
                else if (apply)
                {
                    if (Pet* pet = m_target->GetPet())
                        if (pet->isAlive())
                            pet->CastSpell(pet,m_spellProto->Id,true);
                }
                return;
            }
            else
                break;
        }
        default:
            break;
    }

    if (linkedSet.size() > 0)
    {
        // prevent aura deletion, specially in multi-boost case
        SetInUse(true);
        for (SpellLinkedSet::const_iterator itr = linkedSet.begin(); itr != linkedSet.end(); ++itr)
        {
            apply ?
                m_target->CastSpell(m_target, *itr, true, NULL, NULL, GetCasterGuid()) :
                m_target->RemoveAurasByCasterSpell(*itr, GetCasterGuid());
        }
        SetInUse(false);
    }
}

SpellAuraHolder::~SpellAuraHolder()
{
//    DEBUG_LOG("SpellAuraHolder:: destructor for SpellAuraHolder of spell %u called.", GetId());
}

void SpellAuraHolder::Update(uint32 diff)
{
    if (!m_spellProto)
    {
        DEBUG_LOG("SpellAuraHolder::Update attempt call Update on holder, but holder not have spellproto!");
        return;
    }

    if (m_duration > 0)
    {
        m_duration -= diff;
        if (m_duration < 0)
            m_duration = 0;

        m_timeCla -= diff;

        if (m_timeCla <= 0)
        {
            if (Unit* caster = GetCaster())
            {
                Powers powertype = Powers(GetSpellProto()->powerType);
                int32 manaPerSecond = GetSpellProto()->manaPerSecond + GetSpellProto()->manaPerSecondPerLevel * caster->getLevel();
                m_timeCla = 1*IN_MILLISECONDS;

                if (manaPerSecond)
                {
                    if (powertype == POWER_HEALTH)
                        caster->ModifyHealth(-manaPerSecond);
                    else
                        caster->ModifyPower(powertype, -manaPerSecond);
                }
            }
        }
    }

    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (Aura *aura = GetAuraByEffectIndex(SpellEffectIndex(i)))
            aura->UpdateAura(diff);

    if (!m_target || !m_target->IsInWorld())
        return;

    // Channeled aura required check distance from caster
    if (IsChanneledSpell(m_spellProto) && GetCasterGuid() != m_target->GetObjectGuid())
    {
        Unit* caster = GetCaster();
        if(!caster)
        {
            if (m_target)
                m_target->RemoveAurasByCasterSpell(GetId(), GetCasterGuid());
            return;
        }

        // need check distance for channeled target only
        if (caster->GetChannelObjectGuid() == m_target->GetObjectGuid())
        {
            // Get spell range
            float max_range = GetSpellMaxRange(sSpellRangeStore.LookupEntry(m_spellProto->rangeIndex));

            if(Player* modOwner = caster->GetSpellModOwner())
                modOwner->ApplySpellMod(GetId(), SPELLMOD_RANGE, max_range);

            if(!caster->IsWithinDistInMap(m_target, max_range))
            {
                caster->InterruptSpell(CURRENT_CHANNELED_SPELL);
                return;
            }
        }
    }
}

void SpellAuraHolder::RefreshHolder()
{
    SetAuraDuration(GetAuraMaxDuration());
    SendAuraUpdate(false);
}

void SpellAuraHolder::SetAuraMaxDuration(int32 duration)
{
    m_maxDuration = duration;

    // possible overwrite persistent state
    if (duration > 0)
    {
        if (!(IsPassive() && GetSpellProto()->DurationIndex == 0))
            SetPermanent(false);

        SetAuraFlags(GetAuraFlags() | AFLAG_DURATION);
    }
    else
        SetAuraFlags(GetAuraFlags() & ~AFLAG_DURATION);
}

bool SpellAuraHolder::HasMechanic(uint32 mechanic) const
{
    if (mechanic == m_spellProto->Mechanic)
        return true;

    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (GetAura(SpellEffectIndex(i)) && m_spellProto->EffectMechanic[i] == mechanic)
            return true;
    return false;
}

bool SpellAuraHolder::HasMechanicMask(uint32 mechanicMask) const
{
    if (mechanicMask & (1 << (m_spellProto->Mechanic - 1)))
        return true;

    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (GetAura(SpellEffectIndex(i)) && m_spellProto->EffectMechanic[i] && ((1 << (m_spellProto->EffectMechanic[i] -1)) & mechanicMask))
            return true;
    return false;
}

bool SpellAuraHolder::IsPersistent() const
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (Aura const* aur = GetAura(SpellEffectIndex(i)))
            if (aur->IsPersistent())
                return true;
    return false;
}

bool SpellAuraHolder::IsAreaAura() const
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (Aura const* aur = GetAura(SpellEffectIndex(i)))
            if (aur->IsAreaAura())
                return true;
    return false;
}

bool SpellAuraHolder::IsPositive() const
{
    if (GetAuraFlags() & AFLAG_POSITIVE)
        return true;
    else if (GetAuraFlags() & AFLAG_NEGATIVE)
        return false;

    // check, if no aura flags defined
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (Aura const* aur = GetAura(SpellEffectIndex(i)))
            if (!aur->IsPositive())
                return false;
    return true;
}

bool SpellAuraHolder::IsEmptyHolder() const
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (m_auraFlags & (1 << i))
            return false;
    return true;
}

void SpellAuraHolder::UnregisterSingleCastHolder()
{
    if (IsSingleTarget())
    {
        if (Unit* caster = GetCaster())
            caster->GetSingleCastSpellTargets().erase(GetSpellProto());

        m_isSingleTarget = false;
    }
}

void SpellAuraHolder::SetVisibleAura(bool remove)
{
    SpellAuraHolderPtr currentHolder = SpellAuraHolderPtr();
    if (!remove)
    {
        for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (Aura const* aura = GetAura(SpellEffectIndex(i)))
            {
                currentHolder = aura->GetHolder();
                break;
            }
        }
        //MANGOS_ASSERT(currentHolder);
    }
    m_target->SetVisibleAura(m_auraSlot, currentHolder);
}

uint32 Aura::CalculateCrowdControlBreakDamage()
{
    if (!GetTarget())
        return 0;

    if (!IsCrowdControlAura(m_modifier.m_auraname))
        return 0;

    // auras with this attribute not have damage cap
    if (GetSpellProto()->HasAttribute(SPELL_ATTR_EX_BREAKABLE_BY_ANY_DAMAGE) &&
        ((GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_DIRECT_DAMAGE) ||
        (GetSpellProto()->HasAttribute(SPELL_ATTR_BREAKABLE_BY_DAMAGE) && !GetSpellProto()->HasAttribute(SPELL_ATTR_STOP_ATTACK_TARGET))))
        return 0;

    // Damage cap for CC effects
    uint32 damageCap = (int32)((float)GetTarget()->GetMaxHealth() * sWorld.getConfig(CONFIG_FLOAT_CROWDCONTROL_HP_BASE));

    if (damageCap < 50)
        damageCap = 50;

    Unit* caster = GetCaster();

    if (!caster)
        return damageCap;

    MAPLOCK_READ(caster,MAP_LOCK_TYPE_AURAS);

    return damageCap;
}

bool Aura::IsAffectedByCrowdControlEffect(uint32 damage)
{
    if (!IsCrowdControlAura(m_modifier.m_auraname))
        return false;

    if (int32(damage) > abs(m_modifier.m_baseamount))
    {
        m_modifier.m_baseamount = 0;
        return false;
    }

    m_modifier.m_baseamount -= damage;
    return true;
}
