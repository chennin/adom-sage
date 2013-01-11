/*---------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * Main file for ADOM Sage - contains game status tracking, state tracking,
 * and the core states.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#include <vector>

#include "adom-sage.h"
#include "states.h"


/*---------------------------------------------------------------------------
 * Global variables
 */

GameStatus *game_status;
AttrMap *attr_map;

/* Debugging setup */
FILE *log_file;
int log_level = 1;


/*---------------------------------------------------------------------------
 * Game status functions
 */

void init_game_status(void)
{
    int i;

    attr_map = new AttrMap;
    (*attr_map)["St"] = attrStrength;
    (*attr_map)["Le"] = attrLearning;
    (*attr_map)["Wi"] = attrWillpower;
    (*attr_map)["Dx"] = attrDexterity;
    (*attr_map)["To"] = attrToughness;
    (*attr_map)["Ch"] = attrCharisma;
    (*attr_map)["Ap"] = attrAppearance;
    (*attr_map)["Ma"] = attrMana;
    (*attr_map)["Pe"] = attrPerception;

    game_status = new GameStatus;
    game_status->player_name[0] = '\0';
    game_status->loc = locWilderness;
    game_status->player_level = 0;

    for (i = 0; i < numAttrs; i++)
    {
        game_status->player_attr[i] = 0;
    }
}


/*---------------------------------------------------------------------------
 * State-changing functions
 */

typedef vector<State *> StateStack;
vector<State *> state_stack, cleanup_stack;

void push_state (State *state)
{
    log(log_statechange, "Enter state: %s\n", state->name());
    state_stack.push_back(state);
}

void pop_state(void)
{
    log(log_statechange, "Leave state: %s\n", cur_state()->name());
    cleanup_stack.push_back(state_stack.back());
    state_stack.pop_back();
}

State *cur_state(void)
{
    return state_stack.back();
}

void cleanup_states(void)
{
    StateStack::iterator iter;

    for (iter = cleanup_stack.begin(); iter != cleanup_stack.end(); iter++)
    {
        delete *iter;
    }

    cleanup_stack.clear();
}


/*---------------------------------------------------------------------------
 * Drawable state
 * Virtual base class adding line-drawing support
 */

int StateDrawable::waddnstr(WINDOW *win, const char *str, int n)
{
    if (!strstr(str, "--"))
    {
        return real_waddnstr(win, str, n);
    }

    else
    {
        // Semi-HACK:Only want to convert multiple -'s, but don't check for that
        for (const char *s = str; *s; s++)
        {
            real_waddch(win, convert_char(*s));
        }

        return OK;
    }
}


/*---------------------------------------------------------------------------
 * Startup state
 * Determine when we enter game via new character or game restore
 */

int StateStartup::waddch(WINDOW *win, chtype ch)
{
    if (ch == '^')
    {
        // Drawing mountains for a new character's wilderness map
        push_state(new StateRunning);
    }

    return StateDrawable::waddch(win, ch);
}

int StateStartup::waddnstr(WINDOW *win, const char *str, int n)
{
    if (strcmp(str, "Loading...") == 0)
    {
        // Restoring saved game
        push_state(new StateRunning);
    }

    return StateDrawable::waddnstr(win, str, n);
}


/*---------------------------------------------------------------------------
 * Command processor state
 * Virtual base class to handle receiving commands, printing messages, etc.
 */

StateCmdProcessor::StateCmdProcessor(MsgMap *msgmap_to_use,
                                     KeyMap *keymap_to_use)
{
    just_cleared = 0;
    next_state = NULL;
    at_more = skip_more = ignore_more = reset_colors = 0;
    msg_attr = A_NORMAL;
    msg_color = COLOR_WHITE;
    msgmap = msgmap_to_use;
    keymap = keymap_to_use;
    skip_next_msg = 0;
    mouse = false;
    allow_repeat = false;
}

StateCmdProcessor::~StateCmdProcessor()
{
    if (mouse)
    {
        log(log_mouse, "Mouse: setting back to old state %x\n", old_mouse_status);
        mouse_restore(old_mouse_status);
    }
}

void StateCmdProcessor::enable_mouse (Command btn1_cmd,
                                      Command btn2_cmd, Command btn3_cmd, Command btn4_cmd)
{
    mouse = true;
    old_mouse_status = mouse_enable();
    log(log_mouse, "Mouse: enabled, old state is %x\n", old_mouse_status);
    mouse_x = mouse_y = -1;
    btn1 = btn1_cmd;
    btn2 = btn2_cmd;
    btn3 = btn3_cmd;
    btn4 = btn4_cmd;
}

int StateCmdProcessor::wclear(WINDOW *win)
{
    if (next_state)
    {
        reset_colors = 1;
        push_state(next_state);
        next_state = NULL;
    }

    else
    {
        just_cleared = 1;
    }

    return real_wclear(win);
}

int StateCmdProcessor::wmove(WINDOW *win, int y, int x)
{
    just_cleared = 0;

    return real_wmove(win, y, x);
}

int StateCmdProcessor::waddch(WINDOW *win, chtype ch)
{
    if (just_cleared && (ch == '-' || ch == '#' || ch == 10)) // 10 == LF
    {
        push_state(new StateFullScreen);
    }

    return real_waddch(win, convert_char(ch));
}

int StateCmdProcessor::waddnstr(WINDOW *win, const char *str, int n)
{
    attr_t old_attr;
    short old_color;
    int result;

    //just_cleared = 0; // fix 'freeze' on skillup after 50
    reset_colors = 1;  // Reset colors at next vsprintf

    // Make note of player's name
    int y, x;
    getyx(win, y, x);

    if (y == 22 && x == 0)
    {
        log(log_game_status, "Game status: player name %s\n", str);
        strcpy(game_status->player_name, str);
    }

    // Handle (more) prompt
    if (strcmp(str, " (more)") == 0 || strcmp(str, "(more)") == 0)
    {
        if (ignore_more)
        {
            ignore_more--;
        }

        else
        {
            at_more = 1;
            return real_waddnstr(win, str, n);
        }
    }

    // Colorize messages as needed
    if (msg_color != COLOR_WHITE || msg_attr != A_NORMAL)
    {
        wattr_get(win, &old_attr, &old_color, NULL);
        wattr_set(win, msg_attr, colorpair[msg_color], NULL);
        result = real_waddnstr(win, str, n);
        wattr_set(win, old_attr, old_color, NULL);

        return result;
    }

    return real_waddnstr(win, str, n);
}

chtype StateCmdProcessor::winch(WINDOW *win)
{
    return reverse_convert_char(real_winch(win));
}

int StateCmdProcessor::wgetch(WINDOW *win)
{
    int y, x, result;
    Command cmd;

    if (at_more)
    {
        skip_more = morewait(win, skip_more);
        at_more = 0;

        return (int) ' ';
    }

    skip_more = 0;

    // Check for aborted next_state - if we're receiving a command, then
    // we ended up not using next state after all.
    if (next_state)
    {
        delete next_state;
        next_state = NULL;
    }

    // If we're in the message window, just get a key and return it.
    getyx(win, y, x);

    if (y <= 2)
    {
        return get_key(win);
    }

    // Get and process a command.
    while (1)
    {
        cmd = cmdIncomplete;

        // If this state supports repeatable stuff, prepare for that.
        if (allow_repeat)
        {
            *prev_keys = *current_keys;
            current_keys->clear();
        }

        while (cmd == cmdIncomplete)
        {
            result = get_key(win, config->cursor_visibility,
                             config->cursor_attr, mouse);

            if (mouse && result == KEY_MOUSE)
            {
                // Mouse handling
                int button;

                get_mouse_event(&mouse_y, &mouse_x, &button);
                mouse_y = max(2, min(mouse_y, LINES - 3));

                move_cursor(win, mouse_y, mouse_x);

                Command btn_cmd = cmdInvalid;

                switch (button)
                {
                    case 1:
                        btn_cmd = btn1;
                        break;

                    case 2:
                        btn_cmd = btn2;
                        break;

                    case 3:
                        btn_cmd = btn3;
                        break;

                    case 4:
                        btn_cmd = btn4;
                        break;
                }

                if (btn_cmd != cmdInvalid)
                {
                    key_queue->push_cmd(btn_cmd);
                }

                // Hack: Clear out current working command and restart
                // processing with the keys generated from the mouse event
                cmdstring = "";
                current_input->clear();

            }

            else

            {
                // See if we have a complete command yet
                current_input->push_back(result);
                cmdstring += ch_to_cmdstring(result);
                cmd = lookup_command(cmdstring, keymap);
            }
        }

        // Process the command
        log(log_cmds, "Command: %s -> %i\n", cmdstring.c_str(), (int) cmd);
        cmdstring.erase();
        handle_cmd(win, cmd);
        last_cmd = cmd;

        if (!is_sage_cmd(cmd))
        {
            result = current_input->front();
            current_input->pop_front();

            return result;
        }

        else
        {
            current_input->clear();
        }
    }
}

int StateCmdProcessor::vsprintf(char *str, const char *format, va_list ap)
{
    int result;
    MsgMap::const_iterator iter;
    va_list handler_ap, subst_ap;

    if (reset_colors)
    {
        log(log_msgs, "New message\n", format);
        msg_color = COLOR_WHITE;
        msg_attr = A_NORMAL;
        reset_colors = 0;
    }

    log(log_msgs, "Message  in: %s\n", format);

    if (skip_next_msg)  // Should be moved earlier in func?
    {
        log(log_msgs, "Message skipped\n");
        skip_next_msg--;
        str[0] = '\0';

        return 0;
    }

    // Call message handler
    va_copy(handler_ap, ap);
    va_copy(subst_ap, ap);
    result = real_vsprintf(str, format, ap);

    // Generic handling for various messages - look up color to use
    iter = msgmap->find(format);

    if (iter == msgmap->end())
    {
        iter = msgmap->find(str);
    }
    
    if (iter == msgmap->end())
    {
        for (MsgMap::const_iterator nstrict_iter = msgmap->begin(); 
                                    nstrict_iter != msgmap->end(); 
                                    nstrict_iter ++)
        {
            if (strstr(str, (*nstrict_iter).first) != NULL)
            {
                iter = nstrict_iter;
                break;
            }
        }
    }

    if (iter != msgmap->end())
    {
        if (iter->second->color != COLOR_WHITE ||
            iter->second->attr != A_NORMAL)
        {
            msg_color = iter->second->color;
            msg_attr = iter->second->attr;
        }

        if (iter->second->no_skip)
        {
            skip_more = 0;
        }

        if (iter->second->handler)
        {
            (*iter->second->handler)
            (this, iter->second->handler_data, str, format, handler_ap);
        }

        if (iter->second->subst)
        {
            result = real_vsprintf(str, iter->second->subst, subst_ap);
        }
    }
    va_end(subst_ap);
    va_end(handler_ap);

    log(log_msgs, "Message out: %s\n", str);

    // Hack - shorten named monsters if desired
    if (config->short_named_monsters &&
        strcmp(format, "This is %s. %s %s.") != 0 &&
        strcmp(format, "This is %s%s. %s %s.") != 0 &&
        strcmp(format, "was killed %sby %s") != 0)
    {
        char *start, *stop;

        while ((stop = strchr(str, '~')) != NULL)
        {
            for (start = stop; start > str && *start != ','; start--)
            {
                ;
            }

            memmove(start, stop + 1, strlen(str) - (stop - str));
        }
    }

    return result;
}

int StateCmdProcessor::vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	return StateCmdProcessor::vsprintf(str, format, ap);
}

/*---------------------------------------------------------------------------
 * Running state
 * Running, accepting normal commands, doing normal messages, etc.
 */

StateRunning::StateRunning() : StateCmdProcessor(main_msgmap, main_keymap)
{
    allow_repeat = true;
}

void StateRunning::handle_cmd(WINDOW *win, Command cmd)
{
    switch (cmd)
    {
        case cmdCast:

            if (config->spell_stats)
            {
                push_state(new StateCastSpell);
            }

            break;

        case cmdDisplayCharacterInformation:

        case cmdDisplayWeaponSkills:
            push_state(new StateInfoScreen);
            break;

        case cmdDisplayRequiredExp:

            if (game_status->player_level < 50)
            {
                push_state(new StateInfoScreen);
            }

            break;

        case cmdDisplayKickStats:

        case cmdDisplayMissileStats:

        case cmdDisplayWeaponStats:
            push_state(new StateWeaponStats);
            break;

        case cmdDisplayRecipes:
            push_state(new StateAlchemy);
            break;

        case cmdExtDrop:
            next_state = new StateExtDrop;
            break;

            // Sage-specific commands
        case cmdRepeat:
            key_queue->push_queue(*prev_keys);
            break;

        case cmdDynamicDisplaySpeed:

        case cmdDynamicDisplayEnergy:

        case cmdDynamicDisplayGold:

        case cmdDynamicDisplayAmmo:

        case cmdDynamicDisplayTurns:

            if (reverse_keymap[cmdSwitchDynamicDisplay])
            {
                push_state(new StateDynamicDisplay(this, cmd));
            }

            else
            {
                show_msg(win, "Keymaps are broken, can't set dynamic display.");
            }

            break;

        default:

            // Macro commands
            if (cmd >= cmdMacro0 &&
                cmd <= cmdMacroZ &&
                config->macro[cmd - cmdMacro0] &&
                macro_queue->empty())      // Avoid recursive macros
            {
                macro_queue->push_string(config->macro[cmd - cmdMacro0]);
            }

            break;
    }
}


/*---------------------------------------------------------------------------
 * Examine state
 * Handles the examine command ('l'ook)
 */

StateExamine::StateExamine() : StateCmdProcessor(main_msgmap, look_keymap)
{
}

void StateExamine::handle_cmd(WINDOW *win, Command cmd)
{
    if (cmd == cmdCancel || cmd == cmdInvalid)
    {
        pop_state();
    }

    // Handle macros
    else if (cmd >= cmdMacro0 &&
             cmd <= cmdMacroZ &&
             config->macro[cmd - cmdMacro0] &&
             macro_queue->empty())      // Avoid recursive macros
    {
        macro_queue->push_string(config->macro[cmd - cmdMacro0]);
    }
}


/*---------------------------------------------------------------------------
 * Locate state
 * Handles selecting a location - for 'n'ame command, teleport control
 */

StateLocate::StateLocate() : StateCmdProcessor(main_msgmap, locate_keymap)
{
    if (config->mouse)
    {
        enable_mouse(cmdConfirm);
    }
}

void StateLocate::handle_cmd(WINDOW *win, Command cmd)
{
    if (cmd == cmdInvalid || cmd == cmdCancel || cmd == cmdConfirm)
    {
        pop_state();
    }

    // Handle macros
    else if (cmd >= cmdMacro0 &&
             cmd <= cmdMacroZ &&
             config->macro[cmd - cmdMacro0] &&
             macro_queue->empty())      // Avoid recursive macros
    {
        macro_queue->push_string(config->macro[cmd - cmdMacro0]);
    }

    else if (cmd == cmdAscend || cmd == cmdDescend)
    {
        int y, x;
        chtype ch;

        if (cmd == cmdAscend)
        {
            ch = '<';
        }

        else
        {
            ch = '>';
        }

        if (find_ch(win, ch, y, x))
        {
            move_cursor(win, y, x);
        }
    }
}


/*---------------------------------------------------------------------------
 * Target state
 * Handles selecting a target - shooting, ordering companions, etc.
 */

StateTarget::StateTarget() : StateCmdProcessor(main_msgmap, target_keymap)
{
    if (config->mouse)
    {
        enable_mouse(cmdTarget);
    }
}

void StateTarget::handle_cmd(WINDOW *win, Command cmd)
{
    if (cmd == cmdTarget || cmd == cmdCancel)
    {
        pop_state();
    }

    // Handle macros
    else if (cmd >= cmdMacro0 &&
             cmd <= cmdMacroZ &&
             config->macro[cmd - cmdMacro0] &&
             macro_queue->empty())      // Avoid recursive macros
    {
        macro_queue->push_string(config->macro[cmd - cmdMacro0]);
    }
}
