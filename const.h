/*-----------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * Various constants, put in a separate file to keep them out of the way
 */

// Game locations.  Only list those locations we care about.
typedef enum
{
    locWilderness,
    locToEF
} Location;

// Stats
typedef enum
{
    attrStrength,
    attrLearning,
    attrWillpower,
    attrDexterity,
    attrToughness,
    attrCharisma,
    attrAppearance,
    attrMana,
    attrPerception
} PlayerAttr;

const int numAttrs = attrPerception + 1;

// Commands, and things resembling commands
typedef enum
{
    // Invalid/incomplete command
    cmdInvalid,
    cmdIncomplete,
    // Normal game commands
    // Put movement commands first so we can do an array of them easily.
    // DO NOT change their order.
    cmdMoveSW,
    cmdMoveS,
    cmdMoveSE,
    cmdMoveW,
    cmdWait,
    cmdMoveE,
    cmdMoveNW,
    cmdMoveN,
    cmdMoveNE,
    cmdActivateTrap,
    cmdApplySkill,
    cmdAscend,
    cmdCast,
    cmdChat,
    cmdChangeHighlightMode,
    cmdChangeTactic,
    cmdCheckLiteracy,
    cmdCleanEars,
    cmdClose,
    cmdContinuousSearch,
    cmdCreateShortLogfile,
    cmdCreateVerboseLogfile,
    cmdCreateScreenshot,
    cmdDescend,
    cmdDescribeWeather,
    cmdDip,
    cmdDisplayAvailableTalents,
    cmdDisplaySkills,
    cmdDisplayBackground,
    cmdDisplayBill,
    cmdDisplayBurdenLevels,
    cmdDisplayConfiguration,
    cmdDisplayChaosPowers,
    cmdDisplayCharacterInformation,
    cmdDisplayCompanions,
    cmdDisplayCurrentWealth,
    cmdDisplayDeity,
    cmdDisplayElapsedGameTime,
    cmdDisplayIdentifiedItems,
    cmdDisplayKickStats,
    cmdDisplayKilledMonsters,
    cmdDisplayLevelMap,
    cmdDisplayMessageBuffer,
    cmdDisplayMissileStats,
    cmdDisplayMonsterWoundStatus,
    cmdDisplayQuestStatus,
    cmdDisplayRecipes,
    cmdDisplayRequiredExp,
    cmdDisplayTalents,
    cmdDisplayVersion,
    cmdDisplayWeaponSkills,
    cmdDisplayWeaponStats,
    cmdDrink,
    cmdDrop,
    cmdDropComfortably,
    cmdEat,
    cmdExamine,
    cmdExtDrop,
    cmdExtPay,
    cmdExtUse,
    cmdGive,
    cmdHandle,
    cmdInventory,
    cmdInvokeMindcraft,
    cmdIssueOrder,
    cmdKick,
    cmdLook,
    cmdMarkSpells,
    cmdMiscEquip,
    cmdName,
    cmdHelp,
    cmdOpen,
    cmdPay,
    cmdPickUpFast,
    cmdPickUp,
    cmdPickUpComfortably,
    cmdPray,
    cmdQuit,
    cmdRead,
    cmdRecall,
    cmdRedrawScreen,
    cmdSacrifice,
    cmdSave,
    cmdSearch,
    cmdSetTactics1,
    cmdSetTactics2,
    cmdSetTactics3,
    cmdSetTactics4,
    cmdSetTactics5,
    cmdSetTactics6,
    cmdSetTactics7,
    cmdSetVariable,
    cmdShoot,
    cmdSwapPosition,
    cmdSwitchAutoPickup,
    cmdSwitchDynamicDisplay,
    cmdSwitchMoreKey,
    cmdUnlock,
    cmdUseItem,
    cmdUseClassPower,
    cmdUseSpecialAbility,
    cmdUseTool,
    cmdWalkSW,
    cmdWalkS,
    cmdWalkSE,
    cmdWalkW,
    cmdWalkSpot,
    cmdWalkE,
    cmdWalkNW,
    cmdWalkN,
    cmdWalkNE,
    cmdWipeFace,
    cmdZap,
    // Commands for targeting, looking, locating
    cmdCancel,
    cmdConfirm,
    cmdMore,
    cmdNextTarget,
    cmdPrevTarget,
    cmdTarget,
    // ADOM Sage commands - not to be passed to ADOM
    cmdRepeat,
    cmdDynamicDisplaySpeed,
    cmdDynamicDisplayEnergy,
    cmdDynamicDisplayGold,
    cmdDynamicDisplayAmmo,
    cmdDynamicDisplayTurns,
    cmdMacro0,
    cmdMacroA = cmdMacro0 + 10,
    cmdMacroZ = cmdMacroA + 25,
} Command;
const int numCommands = cmdMacroZ + 1;

inline bool is_macro(Command cmd)
{
    return (cmd >= cmdMacro0 && cmd <= cmdMacroZ);
}
const int numMacros = 36;

// Spells
typedef enum
{
    spellUnknown,
    spellAcidBall,
    spellAcidBolt,
    spellBless,
    spellBurningHands,
    spellCalmMonster,
    spellCreateItem,
    spellCureCriticalWounds,
    spellCureDisease,
    spellCureLightWounds,
    spellCureSeriousWounds,
    spellDarkness,
    spellDeathRay,
    spellDestroyUndead,
    spellDisarmTrap,
    spellEarthquake,
    spellFarsight,
    spellFireball,
    spellFireBolt,
    spellFrostBolt,
    spellGreaterIdentify,
    spellHeal,
    spellIceBall,
    spellIdentify,
    spellImprovedFireball,
    spellInvisibility,
    spellKnock,
    spellKnowAlignment,
    spellLight,
    spellLightningBall,
    spellLightningBolt,
    spellMagicLock,
    spellMagicMap,
    spellMagicMissile,
    spellMysticShovel,
    spellNeutralizePoison,
    spellPetrification,
    spellRemoveCurse,
    spellRevelation,
    spellScareMonster,
    spellSlowMonster,
    spellSlowPoison,
    spellStrengthOfAtlas,
    spellStunRay,
    spellSummonMonsters,
    spellTeleportation,
    spellWeb,
    spellWish
} Spell;
const int numSpells = spellWish + 1;

// Mindcraft
typedef enum
{
    mindTeleportControl,
    mindConfusionBlast,
    mindConfusionWave,
    mindMindBlast,
    mindMentalShield,
    mindMindWave,
    mindTeleBlast,
    mindEyesOfMind,
    mindGreaterMentalBlast,
    mindGreaterTeleBlast,
    mindRegeneration,
    mindTeleportSelf,
    mindTeleportOther,
    mindGreaterMentalWave
} Mindcraft;
const int numMindcrafts = mindGreaterMentalWave + 1;

// Elements and pseudo-elements; types of attacks
typedef enum
{
    elemFire,
    elemIce,
    elemShock,
    elemAcid,
    elemPoison,
    elemWater,
} Element;

// Manual
typedef enum
{
    manIntro,
    manHoroscope,
    manCharacters,
    manAlignment,
    manSkills,
    manWeaponSkills,
    manAttributes,
    manExperience,
    manMagic,
    manMindcraft,
    manCorruptions,
    manGame,
    manDisplay,
    manCommands,
    manDice,
    manInventory,
    manMisc,
    manHowToStart,
    manCredits,
    manCustomizing,
} ManualSection;
const int numManualSections = manCustomizing + 1;

