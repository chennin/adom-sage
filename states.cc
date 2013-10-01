/*---------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * Additional states
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>

#include "adom-sage.h"
#include "states.h"


/*---------------------------------------------------------------------------
 * Full screen state
 * Full-screen displays - inventory, companions list, etc.
 * More generic than info screen - entered when we detect a transition to
 * a full screen
 */

StateFullScreen::StateFullScreen() : need_title(true)
{
}

int StateFullScreen::wclear (WINDOW *win)
{
    pop_state();
    return real_wclear(win);
}

int StateFullScreen::vsprintf (char *str, const char *format, va_list ap)
{
    process(format);
    return real_vsprintf(str, format, ap);
}

int StateFullScreen::vsnprintf (char *str, size_t size, const char *format, va_list ap)
{
    process(format);
    return real_vsnprintf(str, size, format, ap);
}
void StateFullScreen::process(const char *format)
{
    // Special handling for certain full screens
    if (need_title)
    {
        if (strcmp(format, "Mental Powers_") == 0)
        {
            pop_state();
            push_state(new StateMindcraft);
        }

        need_title = false;
    }
}
/*---------------------------------------------------------------------------
 * Info screen state
 * Information screens, such as character background, required exp, etc.
 * Entered when user selects a command that we can't otherwise handle
 */

int StateInfoScreen::wgetch (WINDOW *win)
{
    int result = get_key(win);

    if (result == 'Z' || result == 'z' || result == ' ')
    {
        pop_state();
    }

    return result;
}


/*---------------------------------------------------------------------------
 * Level up state
 * Handles the level up screen
 */

StateLevelUp::StateLevelUp()
{
    done = false;
}

int StateLevelUp::wclear (WINDOW *win)
{
    if (done)
    {
        pop_state();
    }

    return real_wclear(win);
}

int StateLevelUp::vsprintf (char *str, const char *format, va_list ap)
{
    process(format);
    return real_vsprintf(str, format, ap);
}

int StateLevelUp::vsnprintf (char *str, size_t size, const char *format, va_list ap)
{
    process(format);
    return real_vsnprintf(str, size, format, ap);
}

void StateLevelUp::process(const char *format)
{
    if (strcmp(format, "[Press SPACE to continue]") == 0)
    {
        done = true;
    } 
}
/*---------------------------------------------------------------------------
 * Weapon stats state
 * Handles displaying stats for weapons, kicks, missiles
 */

StateWeaponStats::StateWeaponStats()
{
    dice = 1;
    sides = 3;
    plus = 0;
    fraction = 1;
}

int StateWeaponStats::wmove(WINDOW *win, int y, int x)
{
    if (y > 2)
    {
        pop_state();
    }

    return real_wmove(win, y, x);
}

int StateWeaponStats::sprintf(char *str, const char *format, va_list ap)
{
    const char *dice_format = "%dd%d";
    const char *hand_format = "\x03\xC6%s_hand:_\x03\xCE%+d\x03\xC6_bonus to hit,_%s\x03\xCE%s%+d%s\x03\xC6_damage%s";
    const char *kick_format = "\x03\xCE%+d\x03\xC6_bonus to hit,_\x03\xCE" "1d%d%+d\x03\xC6_damage";
    const char *missile_dice_format = "%s%+d";
    const char *missile_fraction_dice_format = "[(%s)/%d]%+d";
    const char *missile_format = "\x03\xC6" "Ammunition:_\x03\xCE%ld\x03\xC6, base range:_\x03\xCE%d\x03\xC6,_\x03\xCE%+d\x03\xC6_bonus to hit,_\x03\xCE%s\x03\xC6_damage";

    if (strcmp(format, dice_format) == 0)
    {
        dice = va_arg(ap, int);
        sides = va_arg(ap, int);
        fraction = 1;
        return real_sprintf(str, format, dice, sides);
    }

    else if (strcmp(format, missile_dice_format) == 0)
    {
        char *dicestr;
        dicestr = va_arg(ap, char *);
        plus = va_arg(ap, int);
        fraction = 1;
        return real_sprintf(str, format, dicestr, plus);
    }

    else if (strcmp(format, missile_fraction_dice_format) == 0)
    {
        char *dicestr;
        dicestr = va_arg(ap, char *);
        fraction = va_arg(ap, int);
        plus = va_arg(ap, int);
        return real_sprintf(str, format, dicestr, fraction, plus);
    }

    else if (strcmp(format, hand_format) == 0)
    {
        char *hand, *dicestr, *open_paren, *unk, *close_paren;
        int tohit;
        hand = va_arg(ap, char *);
        tohit = va_arg(ap, int);
        open_paren = va_arg(ap, char *);
        dicestr = va_arg(ap, char *);
        plus = va_arg(ap, int);
        unk = va_arg(ap, char *);
        close_paren = va_arg(ap, char *);
        return real_sprintf(str,
                            "%s hand: %+d bonus to hit, %s%dd%d%+d%s damage%s (%i-%i, avg %.1f)",
                            hand, tohit, open_paren, dice, sides, plus, unk, close_paren,
                            (dice + plus) / fraction, (dice * sides + plus) / fraction,
                            ((double) dice * (sides + 1) / 2 + plus) / fraction);
    }

    else if (strcmp(format, kick_format) == 0)
    {
        int tohit;
        tohit = va_arg(ap, int);
        sides = va_arg(ap, int);
        plus = va_arg(ap, int);
        return real_sprintf(str,
                            "%+d bonus to hit, 1d%d+%d damage (%i-%i, avg %.1f)",
                            tohit, sides, plus,
                            1 + plus, sides + plus,
                            (double) (sides + 1) / 2 + plus);
    }

    else if (strcmp(format, missile_format) == 0)
    {
        char *dmgstr;
        int ammo, range, tohit;
        ammo = va_arg(ap, int);
        range = va_arg(ap, int);
        tohit = va_arg(ap, int);
        dmgstr = va_arg(ap, char *);
        return real_sprintf(str,
                            "Ammo: %i, base range: %i, %+d to hit, %s damage (%i-%i, avg %.1f)",
                            ammo, range, tohit, dmgstr,
                            dice / fraction + plus, (dice * sides) / fraction + plus,
                            (double) dice * (sides + 1) / 2 / fraction + plus);
    }

    else
    {
        return real_vsprintf(str, format, ap);
    }
}

int StateWeaponStats::vsprintf(char *str, const char *format, va_list ap)
{
	int ret = process(str, format, ap);
	if (ret == -200)
	{
		return real_vsprintf(str, format, ap);
	}
	return ret;
}
int StateWeaponStats::process(char *str, const char *format, va_list ap)
{
    const char *fraction_format = "\x03\xC6)/\x03\xCE%d";
    const char *hand_dice_format = "1d%d";

    if (strcmp(format, fraction_format) == 0)
    {
        fraction = va_arg(ap, int);
        return real_sprintf(str, format, fraction);
    }

    else if (strcmp(format, hand_dice_format) == 0)
    {
        dice = 1;
        sides = va_arg(ap, int);
        fraction = 1;
        return real_sprintf(str, format, dice, sides);
    }

    else
    {
        return -200;
    }
}

int StateWeaponStats::vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	int ret = process(str, format, ap);
	if (ret == -200) 
	{
		return real_vsnprintf(str, size, format, ap);
	}
	return ret;
}


/*---------------------------------------------------------------------------
 * Extended drop state
 * Handles the extended drop command
 */

int StateExtDrop::wclear(WINDOW *win)
{
    pop_state();
    return real_wclear(win);
}


/*---------------------------------------------------------------------------
 * Recall state
 * Handles the recall command (monster memory)
 * Monster memory screens exit on space, or on Z and space, depending on
 * whether they're activated by the recall command or by choosing [M]ore
 * while looking or targeting.  Huh?  So we can't use InfoScreen...
 */

int StateRecall::wgetch(WINDOW *win)
{
    int result = get_key(win);

    if (result == 'z' || result == 'Z' || result == ' ' || result == '\r')
    {
        pop_state();
        return ' ';
    }

    else
    {
        return result;
    }
}


/*---------------------------------------------------------------------------
 * Swap state
 * Used to implement auto-swap-with-neutrals.  The swap command is sometimes
 * disabled (for example, while swimming), so we take this approach:
 *   Say 'no' to attack/backstab command
 *   Issue swap command
 *   If prompted with a direction, issue that direction
 *   If instead asked for a command, give up.  Make a note of the initial
 *     number of entries in the key queue, and if we go back to that number
 *     of entries without being asked for a direction, then give up.
 * Just checking for a "can't swap now" message would be easier, but we don't
 * want to have to exhaustively check for all such messages.
 */

StateSwap::StateSwap(StateCmdProcessor *cmd_processor, Command new_dir)
    : StateDecorator(cmd_processor)
{
    dir = new_dir;

    queue_size = key_queue->size();
    key_queue->push_back('n');
    key_queue->push_cmd(cmdSwapPosition);
}

int StateSwap::wgetch(WINDOW *win)
{
    int y, x;
    getyx(win, y, x);

    if (key_queue->size() == queue_size && y > 1)
    {
        pop_state();
    }

    return component->wgetch(win);
}

int StateSwap::vsprintf(char *str, const char *fmt, va_list ap)
{
    if (process(str, fmt) == 0)
    {
       return 0;
    }
    else
    {
        return component->vsprintf(str, fmt, ap);
    }
}

int StateSwap::vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
    if (process(str, fmt) == 0)
    {
        return 0;
    }
    else
    {
        return component->vsnprintf(str, size, fmt, ap);
    }
}

int StateSwap::process(char *str, const char *fmt)
{
    if (strcmp(fmt, "Which direction [%s, Z = ABORT]?") == 0)
    {
        key_queue->push_cmd(dir);
        strcpy(str, "");
        return 0;
    }
  return 1;
}
/*---------------------------------------------------------------------------
 * Alchemy state
 * Handles the recipes command
 */

StateAlchemy::StateAlchemy()
{
    just_cleared = 0;
}

int StateAlchemy::wclear (WINDOW *win)
{
    just_cleared = 1;
    return real_wclear(win);
}

int StateAlchemy::wmove (WINDOW *win, int y, int x)
{
    if (just_cleared)
    {
        pop_state();
    }

    return real_wmove(win, y, x);
}

int StateAlchemy::waddch (WINDOW *win, chtype ch)
{
    just_cleared = 0;
    return real_waddch(win, convert_char(ch));
}
int StateAlchemy::vsprintf(char *str, const char *format, va_list ap)
{
	return StateAlchemy::vsnprintf(str, 65535, format, ap);
}
int StateAlchemy::vsnprintf (char *str, size_t size, const char *format, va_list ap)
{
    just_cleared = 0;

    if (strcmp(format, "don't know any recipes yet.") == 0)
    {
        pop_state();
        return real_vsnprintf(str, size, format, ap);
    }

    else if (config->short_alchemy
             && strcmp(format, "\x03\xC6Recipe #\x03\xCE%d\x03\xC6:") == 0)
    {
        return real_vsnprintf(str, size,  "\x03\xC6#\x03\xCE%2d\x03\xC6:", ap);
    }

    else if (config->short_alchemy
             && strncmp(format, "To mix %s", strlen("To mix %s")) == 0)
    {
        char *article, *a, *b, *c;
        article = va_arg(ap, char *);
        a = va_arg(ap, char *);
        article = va_arg(ap, char *);
        b = va_arg(ap, char *);
        article = va_arg(ap, char *);
        c = va_arg(ap, char *);
        return real_sprintf(str, "\x03\xCE %s\x03\xC6 =\x03\xCE %s\x03\xC6 +\x03\xCE %s\x03\xC6\n\n", a, b, c);
    }

    else
    {
        return real_vsnprintf(str, size, format, ap);
    }
}


/*---------------------------------------------------------------------------
 * Dynamic display state
 * Handles the set-dynamic-display-to-x commands
 */

StateDynamicDisplay::StateDynamicDisplay(StateCmdProcessor *cmd_processor,
        Command cmd_new_display) : StateDecorator(cmd_processor)
{
    cmd_display = cmd_new_display;

    queue_size = key_queue->size();
    key_queue->push_cmd(cmdSwitchDynamicDisplay);
}

int StateDynamicDisplay::wgetch(WINDOW *win)
{
    if (queue_size == key_queue->size())
    {
        key_queue->push_cmd(cmdSwitchDynamicDisplay);
    }

    return component->wgetch(win);
}
int StateDynamicDisplay::vsprintf(char *str, const char *format, va_list ap)
{
	return StateDynamicDisplay::vsnprintf(str, 65535, format, ap);
}
int StateDynamicDisplay::vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    int result;
    const char *dyn_display_msg = "[The dynamic display now shows the";
    result = component->vsnprintf(str, size, format, ap);

    if (strcmp(format, dyn_display_msg) == 0)
    {
        strcpy(str, "");
    }

    else if (strcmp(format, "energy cost of your last action.]") == 0)
    {
        if (cmd_display == cmdDynamicDisplayEnergy)
        {
            real_sprintf(str, "%s %s", dyn_display_msg, format);
            pop_state();
        }

        else
        {
            strcpy(str, "");
        }
    }

    else if (strcmp(format, "amount of gold pieces your character is carrying.]") == 0)
    {
        if (cmd_display == cmdDynamicDisplayGold)
        {
            real_sprintf(str, "%s %s", dyn_display_msg, format);
            pop_state();
        }

        else
        {
            strcpy(str, "");
        }
    }

    else if (strcmp(format, "amount of ammunition your character has readied.]") == 0)
    {
        if (cmd_display == cmdDynamicDisplayAmmo)
        {
            real_sprintf(str, "%s %s", dyn_display_msg, format);
            pop_state();
        }

        else
        {
            strcpy(str, "");
        }
    }

    else if (strcmp(format, "number of game turns that have passed.]") == 0)
    {
        if (cmd_display == cmdDynamicDisplayTurns)
        {
            real_sprintf(str, "%s %s", dyn_display_msg, format);
            pop_state();
        }

        else
        {
            strcpy(str, "");
        }
    }

    else if (strcmp(format, "the speed of your character.]") == 0)
    {
        if (cmd_display == cmdDynamicDisplaySpeed)
        {
            real_sprintf(str, "%s %s", dyn_display_msg, format);
            pop_state();
        }

        else
        {
            strcpy(str, "");
        }
    }

    return result;
}


/*---------------------------------------------------------------------------
 * Mindcraft state
 * Handles the invoke mindcraft command
 */

int StateMindcraft::wclear (WINDOW *win)
{
    pop_state();
    return real_wclear(win);
}
int StateMindcraft::vsprintf(char *str, const char *format, va_list ap)
{
	return StateMindcraft::vsnprintf(str, 65535, format, ap);
}
int StateMindcraft::vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    if (!config->mindcraft_stats)
    {
        int result = real_vsnprintf(str, size, format, ap);
        char *s;

        if (config->fix_typos && (s = strstr(str, "Eyes of the mind")))
        {
            strncpy(s, "Eyes of the Mind", strlen("Eyes of the Mind"));
        }

        return result;
    }

    // Normal length of a mindcraft line is 54 characters + cost in PP.  We
    // add nonprintable characters to put power details in bold, which means
    // that we may overflow the buffer by putting 54 printable characters.
    // So reduce by 4*(max number of added bold sequences).
    const int pad_length = 50;

    // I'm not using a hash_map here, for once.  Not worth the effort.
    const char *mindcraft_name[] =
    {
        "A.__Teleport Control_",
        "B.__Confusion Blast_",
        "%c.__Confusion Wave_",
        "%c.__Mind Blast_",
        "%c.__Mental Shield_",
        "%c.__Mind Wave_",
        "%c.__Telekinetic Blast_",
        "%c.__Eyes of the mind_",
        "%c.__Greater Mental Blast_",
        "%c.__Greater Telekinetic Blast_",
        "%c.__Regeneration_",
        "%c.__Teleport Self_",
        "%c.__Teleport Other_",
        "%c.__Greater Mental Wave_"
    };

    const char *caption1 = "Id__Power_";
    const char *caption2 = "--__----------------------_";

    const char *dmg_format = "(\x03\xCE%dd%d\x03\xC6)";
    const char *armor_format = "[\x03\xCE%+d,%+d\x03\xC6] %s";
    int mind_power;

    for (mind_power = 0; mind_power < numMindcrafts; mind_power++)
        if (strncmp(format, mindcraft_name[mind_power],
                    strlen(mindcraft_name[mind_power])) == 0)
        {
            break;
        }

    if (mind_power != numMindcrafts)
    {
        char *inuse, *cost;
        int W, L;

        W = game_status->player_attr[attrWillpower];
        L = game_status->player_level;

        // Copy letter and power name.
        // First two powers have hard-coded letters.  How annoying.
        if (mind_power == mindTeleportControl
            || mind_power == mindConfusionBlast)
        {
            strcpy(str, mindcraft_name[mind_power]);
        }

        else
        {
            char letter = va_arg(ap, int);

            if (config->fix_typos
                && mind_power == mindEyesOfMind)   // Correct typo
            {
                real_sprintf(str, "%c.__Eyes of the Mind_", letter);
            }

            else
            {
                real_sprintf(str, mindcraft_name[mind_power], letter);
            }
        }

        // Add any necessary details.
        switch (mind_power)
        {
            case mindTeleportControl:
            case mindRegeneration:
                inuse = va_arg(ap, char *);
                strcat(str, inuse);
                break;
            case mindMindBlast:
            case mindMindWave:
                real_sprintf(str + strlen(str), dmg_format, (W + L) / 6 + 1, 5);
                break;
            case mindMentalShield:
                inuse = va_arg(ap, char *);
                real_sprintf(str + strlen(str), armor_format,
                             (W + L) / 5, (W + L) / 8, inuse);
                break;
            case mindTeleBlast:
                real_sprintf(str + strlen(str), dmg_format, (W / 5) + L, 3);
                break;
            case mindGreaterMentalBlast:
            case mindGreaterMentalWave:
                real_sprintf(str + strlen(str), dmg_format, (W + L) / 8 + 1, 9);
                break;
            case mindGreaterTeleBlast:
                real_sprintf(str + strlen(str), dmg_format, (W / 5) + L, 6);
                break;
        }

        // Pad string and add cost to end.
        pad_string(str, pad_length);
        cost = va_arg(ap, char *);
        strcat(str, "\x03\xCE");
        strcat(str, cost);
        strcat(str, "\x03\xC6\n");
        return (int) strlen(str);

        // Captions
    }

    else if (strncmp(format, caption1, strlen(caption1)) == 0)
    {
        strcpy(str, caption1);
        pad_string(str, pad_length);
        strcat(str, "Cost\n");
        return (int) strlen(str);

    }

    else if (strncmp(format, caption2, strlen(caption2)) == 0)
    {
        strcpy(str, caption2);
        pad_string(str, pad_length);
        strcat(str, "----\n");
        return (int) strlen(str);

        // Default case
    }

    else
    {
        return real_vsnprintf(str, size, format, ap);
    }
}


/*---------------------------------------------------------------------------
 * Game summary state
 * Handles the game summary screen.  (Game over, man.  Game over!)
 */

StateGameSummary::StateGameSummary() : clear_count(0)
{
    // If needed, copy log in order to fix flgs
    if (config->fix_flgs)
    {
        local_log_text = new char*[*adom_log_pos];

        for (int i = 0; i < *adom_log_pos; i++)
        {
            local_log_text[i] = strdup((*adom_log_text)[i]);
            log(log_game_status, "Final log: %s\n", local_log_text[i]);
        }

        local_log_pos = *adom_log_pos;
    }
}

int StateGameSummary::wclear(WINDOW *win)
{
    clear_count++;

    if (clear_count == 2)
    {
        pop_state();
    }

    return real_wclear(win);
}


/*---------------------------------------------------------------------------
 * Memorial state
 * Handles writing flgs for ADOM 1.1.0
 */

StateMemorial::StateMemorial (StateCmdProcessor *cmd_processor)
    : StateDecorator (cmd_processor)
{
    do_log = false;
}

int StateMemorial::wgetch(WINDOW *win)
{
    int result;
    if (config->auto_dump_flg == 1)
    {
        key_queue->push_back('y');
	pop_state();
    }
    else {
        result = get_key(win);

        if (result == 'y' || result == 'Y')
        {
            do_log = true;
        }
    }

    return result;
}

int StateMemorial::unlink(const char *pathname)
{
    if (do_log && config->fix_flgs)
    {
        string logpath;
        FILE *log_file;
        int i;

        logpath = cwd_path;
        logpath += "/";
        logpath += game_status->player_name;
        logpath += ".flg";

        log_file = fopen(logpath.c_str(), "w");

        if (log_file == NULL)
        {
            log(log_errors, "Error: Unable to open %s for writing\n",
                logpath.c_str());
        }

        else
        {
            log(log_game_status, "Writing full log to %s\n", logpath.c_str());

            for (i = 0; i < *adom_log_pos; i++)
            {
                fprintf(log_file, "%s\n", (*adom_log_text)[i]);
            }

            for (i = 0; i < local_log_pos; i++)
            {
                fprintf(log_file, "%s\n", local_log_text[i]);
            }

            fclose(log_file);
        }

        do_log = false;
    }
    pop_state();
    return real_unlink(pathname);
}

/*
	StateFlgSuccess
	Accept [nN ] at [Press SPACE to Continue]
*/
StateFlgSuccess::StateFlgSuccess (StateCmdProcessor *cmd_processor)
    : StateDecorator (cmd_processor)
{
}

int StateFlgSuccess::wgetch(WINDOW *win)
{
    int result = get_key(win);
    if ((config->auto_dump_flg == 1) && (result == 'n' || result == 'N' || result == ' '))
    {
        key_queue->push_back(' ');
    }

    return result;
}

