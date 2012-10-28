/*---------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * Support for spells
 */

#include "adom-sage.h"
#include "states.h"

typedef STRING_HASH(int) SpellMap;
SpellMap *spell_map;

struct SpellInfo
{
    int effectivity;
};
SpellInfo spell_info[numSpells];

void init_spells(void)
{
    spell_map = new SpellMap;
    (*spell_map)["Acid Ball"] = spellAcidBall;
    (*spell_map)["Acid Bolt"] = spellAcidBolt;
    (*spell_map)["Bless"] = spellBless;
    (*spell_map)["Burning Hands"] = spellBurningHands;
    (*spell_map)["Calm Monster"] = spellCalmMonster;
    (*spell_map)["Create Item"] = spellCreateItem;
    (*spell_map)["Cure Critical Wounds"] = spellCureCriticalWounds;
    (*spell_map)["Cure Disease"] = spellCureDisease;
    (*spell_map)["Cure Light Wounds"] = spellCureLightWounds;
    (*spell_map)["Cure Serious Wounds"] = spellCureSeriousWounds;
    (*spell_map)["Darkness"] = spellDarkness;
    (*spell_map)["Death Ray"] = spellDeathRay;
    (*spell_map)["Destroy Undead"] = spellDestroyUndead;
    (*spell_map)["Disarm Trap"] = spellDisarmTrap;
    (*spell_map)["Earthquake"] = spellEarthquake;
    (*spell_map)["Farsight"] = spellFarsight;
    (*spell_map)["Fireball"] = spellFireball;
    (*spell_map)["Fire Bolt"] = spellFireBolt;
    (*spell_map)["Frost Bolt"] = spellFrostBolt;
    (*spell_map)["Greater Identify"] = spellGreaterIdentify;
    (*spell_map)["Heal"] = spellHeal;
    (*spell_map)["Ice Ball"] = spellIceBall;
    (*spell_map)["Identify"] = spellIdentify;
    (*spell_map)["Improved Fireball"] = spellImprovedFireball;
    (*spell_map)["Invisibility"] = spellInvisibility;
    (*spell_map)["Knock"] = spellKnock;
    (*spell_map)["Know Alignment"] = spellKnowAlignment;
    (*spell_map)["Light"] = spellLight;
    (*spell_map)["Lightning Ball"] = spellLightningBall;
    (*spell_map)["Lightning Bolt"] = spellLightningBolt;
    (*spell_map)["Magic Lock"] = spellMagicLock;
    (*spell_map)["Magic Map"] = spellMagicMap;
    (*spell_map)["Magic Missile"] = spellMagicMissile;
    (*spell_map)["Mystic Shovel"] = spellMysticShovel;
    (*spell_map)["Neutralize Poison"] = spellNeutralizePoison;
    (*spell_map)["Petrification"] = spellPetrification;
    (*spell_map)["Remove Curse"] = spellRemoveCurse;
    (*spell_map)["Revelation"] = spellRevelation;
    (*spell_map)["Scare Monster"] = spellScareMonster;
    (*spell_map)["Slow Monster"] = spellSlowMonster;
    (*spell_map)["Slow Poison"] = spellSlowPoison;
    (*spell_map)["Strength of Atlas"] = spellStrengthOfAtlas;
    (*spell_map)["Stun Ray"] = spellStunRay;
    (*spell_map)["Summon Monsters"] = spellSummonMonsters;
    (*spell_map)["Teleportation"] = spellTeleportation;
    (*spell_map)["Web"] = spellWeb;
    (*spell_map)["Wish"] = spellWish;

    (*spell_map)["Baptism of Fire"] = spellBurningHands;
    (*spell_map)["Dispel Undead"] = spellDestroyUndead;
    (*spell_map)["Divine Digger"] = spellMysticShovel;
    (*spell_map)["Divine Favour"] = spellCreateItem;
    (*spell_map)["Divine Intervention"] = spellWish;
    (*spell_map)["Divine Key"] = spellKnock;
    (*spell_map)["Divine Wrath"] = spellLightningBolt;
    (*spell_map)["Enlightenment"] = spellIdentify;
    (*spell_map)["Ethereal Bridge"] = spellTeleportation;
    (*spell_map)["Freezing Fury"] = spellIceBall;
    (*spell_map)["Greater Divine Touch"] = spellDeathRay;
    (*spell_map)["Greater Enlightenment"] = spellGreaterIdentify;
    (*spell_map)["Heavenly Fury"] = spellLightningBall;
    (*spell_map)["Hellish Flames"] = spellFireBolt;
    (*spell_map)["Holy Awe"] = spellScareMonster;
    (*spell_map)["Invoked Devastation"] = spellImprovedFireball;
    (*spell_map)["Knowledge of the Ancients"] = spellMagicMap;
    (*spell_map)["Lesser Divine Touch"] = spellStunRay;
    (*spell_map)["Lordly Might"] = spellStrengthOfAtlas;
    (*spell_map)["Major Punishment"] = spellFireball;
    (*spell_map)["Minor Punishment"] = spellMagicMissile;
    (*spell_map)["Nether Bolt"] = spellFrostBolt;
    (*spell_map)["Rain of Sorrow"] = spellAcidBall;
    (*spell_map)["Seal of the Spheres"] = spellMagicLock;
    (*spell_map)["Veil of the Gods"] = spellInvisibility;
}

StateCastSpell::StateCastSpell()
{
    max_letter = 'a' - 1;
    current_spell = spellUnknown;
}

int StateCastSpell::waddch(WINDOW *win, chtype ch)
{
    return real_waddch(win, convert_char(ch));
}

int StateCastSpell::waddnstr(WINDOW *win, const char *str, int n)
{
    const char duration[] = "Duration:";
    const char range[]    = "Range:";
    const char diameter[] = "Diameter:";
    const char damage[]   = "Dmg:";
    const char heals[]    = "Heals:";
    const char spacer[]   = "  ";
    int E, L, W, M;

    if (str[0] == '\0' && current_spell != spellUnknown)
    {
        E = spell_info[current_spell].effectivity;
        L = game_status->player_level;
        W = game_status->player_attr[attrWillpower];
        M = game_status->player_attr[attrMana];

        switch (current_spell)
        {

            case spellAcidBolt:
                print_spell_details(win, range, "%i", W / 4 + L / 4 + 2);
                real_waddnstr(win, spacer, -1);
                print_spell_details(win, damage, "%id%i+r",
                                    max(4, (L + E) / 3), 8);
                break;

            case spellAcidBall:
                print_spell_details(win, diameter, "%i", max(2, W / 8));
                real_waddnstr(win, spacer, -1);
                print_spell_details(win, damage, "%id%i+%i",
                                    max(3, L / 4 + 2), 9, L + E);
                break;

            case spellBless:
                print_spell_details(win, duration, "%id%i+%i",
                                    1, W, (M + E) * 2);
                break;

            case spellBurningHands:
                print_spell_details(win, damage, "%id%i+%i",
                                    min(15, L), 3, L + E);
                break;

            case spellCureCriticalWounds:
                print_spell_details(win, heals, "%id%i+%i", 4, 8, 4 + E);
                break;

            case spellCureLightWounds:
                print_spell_details(win, heals, "%id%i+%i", 1, 8, 1 + E);
                break;

            case spellCureSeriousWounds:
                print_spell_details(win, heals, "%id%i+%i", 2, 10, 2 + E);
                break;

            case spellDarkness:
            case spellLight:
                print_spell_details(win, diameter, "%i",
                                    1 + max(4, (W + E) / 4) / 2);
                break;

            case spellDeathRay:
                print_spell_details(win, range, "%i",
                                    max(2, min(16, (W + E) / 4)));
                break;

            case spellDestroyUndead:
                print_spell_details(win, damage, "%id%i+%i", L + 1, 6, W + E);
                break;

            case spellFarsight:
                print_spell_details(win, duration, "%i", (M + E) * 10);
                break;

            case spellFireball:
                print_spell_details(win, diameter, "%i", max(2, W / 8));
                real_waddnstr(win, spacer, -1);
                print_spell_details(win, damage, "%id%i+%i",
                                    max(3, L / 4 + 2), 6, L + E);
                break;

            case spellFireBolt:
            case spellFrostBolt:
            case spellLightningBolt:
                print_spell_details(win, range, "%i", W / 4 + L / 4 + 2);
                real_waddnstr(win, spacer, -1);
                print_spell_details(win, damage, "%id%i+r",
                                    max(4, (L + E) / 3), 6);
                break;

            case spellHeal:
                print_spell_details(win, heals, "%id%i+%i", 10, 6, 10 + E);
                break;

            case spellIceBall:
                print_spell_details(win, diameter, "%i", max(2, W / 8));
                real_waddnstr(win, spacer, -1);
                print_spell_details(win, damage, "%id%i+%i",
                                    max(3, L / 4 + 2), 7, L + E);
                break;

            case spellImprovedFireball:
                print_spell_details(win, diameter, "%i", max(2, W / 8));
                real_waddnstr(win, spacer, -1);
                print_spell_details(win, damage, "%id%i+%i",
                                    2 + (L + E / 5), 8, L);
                break;

            case spellInvisibility:
                print_spell_details(win, duration, "%id%i", L + E + 3, 6);
                break;

            case spellLightningBall:
                print_spell_details(win, diameter, "%i", max(2, W / 8));
                real_waddnstr(win, spacer, -1);
                print_spell_details(win, damage, "%id%i+%i",
                                    max(3, L / 4 + 2), 8, L + E);
                break;

            case spellMagicMissile:
                print_spell_details(win, range, "%i", W / 4 + L / 4 + 2);
                real_waddnstr(win, spacer, -1);
                print_spell_details(win, damage, "%id%i+r",
                                    max(2, (L + E) / 3), 4);
                break;

            case spellMysticShovel:
                print_spell_details(win, range, "%i",
                                    max(2, min(10, (W + E) / 5)));
                break;

            case spellScareMonster:
                print_spell_details(win, duration, "%id%i+%i", L, 4, E);
                break;

            case spellSlowMonster:
                print_spell_details(win, duration, "%id%i", 3, W + E);
                break;

            case spellStrengthOfAtlas:
                print_spell_details(win, duration, "%i", (W + E) * 20);
                break;

            case spellStunRay:
            case spellWeb:
                print_spell_details(win, range, "%i",
                                    max(3, min(12, (W + E) / 4)));
                break;

            default:
                break;
        }
    }

    return real_waddnstr(win, str, n);
}

int StateCastSpell::wgetch(WINDOW *win)
{
    int result;
    result = get_key(win);

    if ((result >= 'A' && result <= max_letter) ||
        (result >= 'a' && result <= max_letter - 'A' + 'a') ||
        (result == 'z' || result == 'Z' || result == ' '))
    {
        pop_state();
    }

    return result;
}
int StateCastSpell::vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    return StateCastSpell::vsprintf(str, format, ap);
}
int StateCastSpell::vsprintf(char *str, const char *format, va_list ap)
{
    char *spell_name;
    SpellMap::iterator iter;
    va_list ap_copy;
    va_copy(ap_copy, ap);

    // Test if we can cast spells after all
    if ((strcmp(format, "Not while in the wilderness.") == 0) ||
        (strcmp(format, "can't focus your thoughts enough to cast a spell.") == 0))
    {
        pop_state();
    }

    // Allowed spells
    else if (strcmp(format, "%c\x03\xC6-\x03\xCE%c") == 0)
    {
        max_letter = va_arg(ap_copy, int);  // Discard first letter
        max_letter = va_arg(ap_copy, int);  // Save last letter
    }

    else if (strcmp(format, "SELECT A SPELL TO CAST_") == 0)
    {
        current_spell = spellUnknown;
    }

    // Spell name
    else if (strcmp(format, "\x03\xCE%s\x03\xC6") == 0)
    {
        spell_name = va_arg(ap_copy, char *);
        iter = spell_map->find(spell_name);

        if (iter == spell_map->end())
        {
            log(log_errors, "Error: unknown spell %s\n", spell_name);
        }

        else
        {
            current_spell = (Spell) iter->second;
        }
    }
    va_end(ap_copy);

    // Spell castings and cost
    if (strcmp(format,
                    " \x03\xCE%5ld\x03\xC6, \x03\xCE%3d\x03\xC6pp ") == 0)
    {
        return real_vsprintf(str,
                             "\x03\xCE%5ld\x03\xC6,\x03\xCE%4d\x03\xC6pp ", ap);
    }

    // Spell effectivity
    else if (strcmp(format, "____(Effectivity:_\x03\xCE%+d\x03\xC6)") == 0)
    {
        spell_info[current_spell].effectivity = va_arg(ap_copy, int);
        return real_vsprintf(str, "_(\x03\xCE%+3d\x03\xC6)", ap);
    }

    // Full spell entry
    else if (strcmp(format, "\x01%s\x03\xC6%c - %s\x01\n") == 0)
    {

        // Get arguments
        va_copy(ap_copy, ap);
        char *spell_details = va_arg(ap_copy, char *);
        char spell_letter = va_arg(ap_copy, int);
        spell_details = va_arg(ap_copy, char *);
        va_end(ap_copy);

        // Extract spell name
        spell_name = strdup(spell_details);
        char *s = strchr(spell_name, ':');

        do
        {
            s--;
        }
        while (*s == ' ');

        s -= 1;  // Trim trailing bold sequence
        *s = '\0';

        // Look up matching spell
        iter = spell_map->find(spell_name + 2); // Trim leading bold sequence

        if (iter == spell_map->end())
        {
            log(log_errors, "Error: unknown spell %s\n", spell_name);
        }

        else
        {
            current_spell = (Spell) iter->second;
        }

        free(spell_name);
        return real_sprintf(str, "\x01   \x03\xC6%c - %s\x01\n", spell_letter,
                            spell_details);
    }

    return real_vsprintf(str, format, ap);
}

void StateCastSpell::print_spell_details(WINDOW *win, const char *label,
        const char *format, ...)
{
    char details[255];
    va_list ap;
    va_start(ap, format);
    real_waddnstr(win, label, -1);
    real_waddch(win, ' ');
    real_vsprintf(details, format, ap);
//    real_vsnprintf(details, size, format, ap);
    attron(A_BOLD);
    real_waddnstr(win, details, -1);
    attroff(A_BOLD);
    va_end(ap);
}

