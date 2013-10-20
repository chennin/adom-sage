/*-----------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * Functions to process commands.
 */

#include <stdio.h>
#include <string>
#include <map>

#include "adom-sage.h"


/*-----------------------------------------------------------------------------
 * Global variables and class definitions
 */

KeyMap *main_keymap, *look_keymap, *target_keymap, *locate_keymap;
int **reverse_keymap;

typedef STRING_HASH(int) CmdCodeMap;


/*-----------------------------------------------------------------------------
 * Command info
 */

struct CommandInfo
{
    const char *code;
    int cmd;   // Should be Command, but typecasting is working oddly
};

CommandInfo cmd_info[] =
{
    // ADOM commands
    {"ACT", cmdActivateTrap},
    {"ASK", cmdApplySkill},
    {"ASC", cmdAscend},
    {"CST", cmdCast},
    {"TLK", cmdChat},
    {"HIL", cmdChangeHighlightMode},
    {"TAC", cmdChangeTactic},
    {"LIT", cmdCheckLiteracy},
    {"CLE", cmdCleanEars},
    {"CLO", cmdClose},
    {"CSE", cmdContinuousSearch},
    {"CSL", cmdCreateShortLogfile},
    {"CVL", cmdCreateVerboseLogfile},
    {"CSS", cmdCreateScreenshot},
    {"DSC", cmdDescend},
    {"DEW", cmdDescribeWeather},
    {"DIP", cmdDip},
    {"DAT", cmdDisplayAvailableTalents},
    {"DSK", cmdDisplaySkills},
    {"BKG", cmdDisplayBackground},
    {"BIL", cmdDisplayBill},
    {"DBL", cmdDisplayBurdenLevels},
    {"CFG", cmdDisplayConfiguration},
    {"CHP", cmdDisplayChaosPowers},
    {"DCI", cmdDisplayCharacterInformation},
    {"DCO", cmdDisplayCompanions},
    {"MNY", cmdDisplayCurrentWealth},
    {"DID", cmdDisplayDeity},
    {"DET", cmdDisplayElapsedGameTime},
    {"IDI", cmdDisplayIdentifiedItems},
    {"KST", cmdDisplayKickStats},
    {"DKM", cmdDisplayKilledMonsters},
    {"DLM", cmdDisplayLevelMap},
    {"MSG", cmdDisplayMessageBuffer},
    {"DMS", cmdDisplayMissileStats},
    {"MWS", cmdDisplayMonsterWoundStatus},
    {"QST", cmdDisplayQuestStatus},
    {"RCP", cmdDisplayRecipes},
    {"EXP", cmdDisplayRequiredExp},
    {"DTL", cmdDisplayTalents},
    {"VER", cmdDisplayVersion},
    {"DWS", cmdDisplayWeaponSkills},
    {"WST", cmdDisplayWeaponStats},
    {"DRK", cmdDrink},
    {"DRO", cmdDrop},
    {"DRC", cmdDropComfortably},
    {"EAT", cmdEat},
    {"EXE", cmdExamine},
    {"EDR", cmdExtDrop},
    {"EPA", cmdExtPay},
    {"EUS", cmdExtUse},
    {"GIV", cmdGive},
    {"HDL", cmdHandle},
    {"INV", cmdInventory},
    {"INM", cmdInvokeMindcraft},
    {"ISO", cmdIssueOrder},
    {"KCK", cmdKick},
    {"LOK", cmdLook},
    {"MSP", cmdMarkSpells},
    {"STF", cmdMiscEquip},
    {"MSW", cmdMoveSW},
    {"MOS", cmdMoveS},
    {"MSE", cmdMoveSE},
    {"MOW", cmdMoveW},
    {"MOE", cmdMoveE},
    {"MNW", cmdMoveNW},
    {"MON", cmdMoveN},
    {"MNE", cmdMoveNE},
    {"BAP", cmdName},
    {"HLP", cmdHelp},
    {"OPN", cmdOpen},
    {"PAY", cmdPay},
    {"PPK", cmdPickUpFast},
    {"PCK", cmdPickUp},
    {"CPC", cmdPickUpComfortably},
    {"PRA", cmdPray},
    {"QIT", cmdQuit},
    {"REA", cmdRead},
    {"RMM", cmdRecall},
    {"RED", cmdRedrawScreen},
    {"SAC", cmdSacrifice},
    {"SAV", cmdSave},
    {"SEA", cmdSearch},
    {"ST0", cmdSetTactics1},
    {"ST1", cmdSetTactics2},
    {"ST2", cmdSetTactics3},
    {"ST3", cmdSetTactics4},
    {"ST4", cmdSetTactics5},
    {"ST5", cmdSetTactics6},
    {"ST6", cmdSetTactics7},
    {"SEV", cmdSetVariable},
    {"SHT", cmdShoot},
    {"SWA", cmdSwapPosition},
    {"CAP", cmdSwitchAutoPickup},
    {"STS", cmdSwitchDynamicDisplay},
    {"CMK", cmdSwitchMoreKey},
    {"ULD", cmdUnlock},
    {"USE", cmdUseItem},
    {"UMP", cmdUseClassPower},
    {"SPA", cmdUseSpecialAbility},
    {"UTO", cmdUseTool},
    {"WAT", cmdWait},
    {"WM1", cmdWalkSW},
    {"WM2", cmdWalkS},
    {"WM3", cmdWalkSE},
    {"WM4", cmdWalkW},
    {"WM5", cmdWalkSpot},
    {"WM6", cmdWalkE},
    {"WM7", cmdWalkNW},
    {"WM8", cmdWalkN},
    {"WM9", cmdWalkNE},
    {"WIF", cmdWipeFace},
    {"ZAP", cmdZap},

    // Sage-specific commands
    {"DDA", cmdDynamicDisplayAmmo},
    {"DDE", cmdDynamicDisplayEnergy},
    {"DDG", cmdDynamicDisplayGold},
    {"DDS", cmdDynamicDisplaySpeed},
    {"DDT", cmdDynamicDisplayTurns},
    {"RPT", cmdRepeat},

    // Macros
    {"MC0", cmdMacro0},
    {"MC1", cmdMacro0 + 1},
    {"MC2", cmdMacro0 + 2},
    {"MC3", cmdMacro0 + 3},
    {"MC4", cmdMacro0 + 4},
    {"MC5", cmdMacro0 + 5},
    {"MC6", cmdMacro0 + 6},
    {"MC7", cmdMacro0 + 7},
    {"MC8", cmdMacro0 + 8},
    {"MC9", cmdMacro0 + 9},
    {"MCA", cmdMacroA},
    {"MCB", cmdMacroA + 1},
    {"MCC", cmdMacroA + 2},
    {"MCD", cmdMacroA + 3},
    {"MCE", cmdMacroA + 4},
    {"MCF", cmdMacroA + 5},
    {"MCG", cmdMacroA + 6},
    {"MCH", cmdMacroA + 7},
    {"MCI", cmdMacroA + 8},
    {"MCJ", cmdMacroA + 9},
    {"MCK", cmdMacroA + 10},
    {"MCL", cmdMacroA + 11},
    {"MCM", cmdMacroA + 12},
    {"MCN", cmdMacroA + 13},
    {"MCO", cmdMacroA + 14},
    {"MCP", cmdMacroA + 15},
    {"MCQ", cmdMacroA + 16},
    {"MCS", cmdMacroA + 17},
    {"MCT", cmdMacroA + 18},
    {"MCU", cmdMacroA + 19},
    {"MCV", cmdMacroA + 21},
    {"MCW", cmdMacroA + 22},
    {"MCX", cmdMacroA + 23},
    {"MCY", cmdMacroA + 24},
    {"MCZ", cmdMacroA + 25},
};

const char *cmdname(int cmd)
{
    for (size_t i = 0; i < sizeof(cmd_info) / sizeof(CommandInfo); i++)
    {
        if (cmd_info[i].cmd == cmd)
        {
            return cmd_info[i].code;
        }
    }

    return NULL;
}


/*-----------------------------------------------------------------------------
 * Functions to read and process keymap files
 */

// Initialize ADOM's command codes.
void init_cmdcodes (CmdCodeMap &cmdcode)
{
    for (size_t i = 0; i < sizeof(cmd_info) / sizeof(CommandInfo); i++)
    {
        cmdcode[cmd_info[i].code] = cmd_info[i].cmd;
    }
}

int write_default_keymap(void)
{
    string mapfilename;
    FILE *mapfile;

    mapfilename = *config_path + "/sage.kbd";
    mapfile = fopen(mapfilename.c_str(), "w");

    if (mapfile == NULL)
    {
        return sage_error("Unable to open or create map file");
    }

    fputs(
        "# Keymap file for ADOM Sage\n"
        "# This file has the same format as adom.kbd or .adom.kbd - please\n"
        "# see that file for details.\n"
        "\n"
        "# Macros 0-9: these keys tell how to activate the macro.\n"
        "# sage.cfg says what each macro does\n"
        "MC0: \\M0\n"
        "MC1: \\M1\n"
        "MC2: \\M2\n"
        "MC3: \\M3\n"
        "MC4: \\M4\n"
        "MC5: \\M5\n"
        "MC6: \\M6\n"
        "MC7: \\M7\n"
        "MC8: \\M8\n"
        "MC9: \\M9\n"
        "# You can also define MCA through MCZ if you need more macros\n"
        "\n"
        "# Repeat last command\n"
        "RPT: '\n"
        "\n"
        "# Set dynamic display to show character's speed\n"
        "DDS: \\Ms\n"
        "\n"
        "# Set dynamic display to show energy cost of last action\n"
        "DDE: \\Me\n"
        "\n"
        "# Set dynamic display to show gold\n"
        "DDG: \\Mg\n"
        "\n"
        "# Set dynamic display to show ammo\n"
        "DDA: \\Ma\n"
        "\n"
        "# Set dynamic display to show game turns/moves passed\n"
        "DDT: \\Mm\n"
        "\n"
        , mapfile);
    fclose(mapfile);
    return 1;
}

// Read in ADOM's keybindings.
// The official description of the keybindings file:
//   ADOM keybindings are a case-insensitive three-letter command code, a
//   colon, then the list of keys bound to that command.  Comments start
//   with #.  Blank lines are ignored.
// Our take on the keybindings file:
//   We ignore any errors, assuming that ADOM will catch them.
//   ADOM doesn't actually require the third char to be colon, so we don't
//   either.

void read_mapfile (FILE *mapfile, CmdCodeMap &cmdcode)
{
    char buffer[80];
    string cmd, keys, key;
    string::size_type prev_pos, pos;

    while (!feof(mapfile))
    {
        if (fgets(buffer, sizeof(buffer), mapfile) == NULL)
        {
            break;
        }

        if ((strcmp(buffer, "\n") != 0) && (buffer[0] != '#')
            && (strlen(buffer) >= 6))
        {

            buffer[strlen(buffer) - 1] = '\0';  // Remove ending LF
            // Separate into command and keybindings
            buffer[0] = toupper(buffer[0]);
            buffer[1] = toupper(buffer[1]);
            buffer[2] = toupper(buffer[2]);
            buffer[3] = '\0';
            cmd = buffer;
            keys = buffer + 4;

            // Loop through keybindings, adding each one
            prev_pos = 0;

            while (1)
            {
                // Extract next keybinding
                pos = keys.find(' ', prev_pos);

                if (pos == string::npos)
                {
                    key = keys.substr(prev_pos, keys.length() - prev_pos);
                }

                else
                {
                    key = keys.substr(prev_pos, pos - prev_pos);
                }

                // Add keybinding
                if (key.length())
                {
                    (*main_keymap)[strdup(key.c_str())] = cmdcode[cmd.c_str()];

                    if (!reverse_keymap[cmdcode[cmd.c_str()]])
                        reverse_keymap[cmdcode[cmd.c_str()]]
                        = cmdstring_to_ch(key.c_str());

                    log(log_config, "Keymap: %s -> %s (%i)\n",
                        key.c_str(), cmd.c_str(), cmdcode[cmd.c_str()]);
                }

                // Move on to next keybinding
                if (pos == string::npos)
                {
                    break;
                }

                prev_pos = pos + 1;
            }
        }
    }
}

void check_keymaps (void)
{
    KeyMap::iterator iter;
    const char *prev_keys = NULL;
    int prev_cmd = cmdInvalid;
    bool error = false;

    for (iter = main_keymap->begin(); iter != main_keymap->end(); iter++)
    {
        if (prev_keys != NULL)
        {
            if (strncmp(prev_keys, iter->first, strlen(prev_keys)) == 0)
            {
                printf("Key binding for %s (%s) blocks key binding for %s (%s)\n",
                       cmdname(prev_cmd), prev_keys, cmdname(iter->second),
                       iter->first);
                error = true;
            }
        }

        prev_keys = iter->first;
        prev_cmd = iter->second;
    }

    if (error)
    {
        printf("You will probably encounter problems while playing ADOM.\n");
        printf("Please exit and edit adom.kbd (or .adom.kbd) and sage.kbd\n");
        printf("to fix these problems\n");
        printf("\n");
        printf("[PRESS ENTER TO CONTINUE]");
        getchar();
    }
}

int read_keymaps (void)
{
    string mapfilename;
    FILE *adommap, *sagemap;

    CmdCodeMap cmdcode;

    // Initialize keymaps
    main_keymap = new KeyMap;
    reverse_keymap = new int*[numCommands];

    for (int i = 0; i < numCommands; i++)
    {
        reverse_keymap[i] = NULL;
    }

    init_cmdcodes(cmdcode);

    int adom_version = get_version();

    string kbdfile = "/.adom.kbd";
    if (adom_version >= 12011) {
        kbdfile = "/adom.kbd";
    }

    // Open ADOM keymap, check that ADOM has been run
    mapfilename = *config_path + kbdfile;
    log(log_config, "Attempting to open %s\n", mapfilename.c_str());
    adommap = fopen(mapfilename.c_str(), "r");

    if (adommap == NULL)
    {
        printf("\nWARNING:\n"
               "Unable to read ADOM's settings.  This is apparently the first time that you\n"
               "have run ADOM.  After ADOM displays its welcome screen, please exit the game\n"
               "and rerun it so that ADOM Sage can properly initialize.\n\n"
	       "Sage is now entering PASSIVE MODE.\n\n");
        printf("\n[PRESS ENTER TO CONTINUE]\n");
	config->passive = 1;
        getchar();
        return 0;
    }

    // Open Sage keymap, create if needed
    mapfilename = *config_path + "/sage.kbd";
    sagemap = fopen(mapfilename.c_str(), "r");

    if (sagemap == NULL)
    {
        log(log_config, "Config: sage.kbd not found, writing default keymap\n");
        if (!write_default_keymap())
        {
            return 0;
        }

        sagemap = fopen(mapfilename.c_str(), "r");

        if (sagemap == NULL)
        {
            return sage_error("Something very strange happened - still can't read key map");
        }
    }

    // Read and check keymaps.  Read Sage's first so that, in case of
    // duplicates, ADOM will override.  (Is this the best approach possible?)
    read_mapfile(sagemap, cmdcode);
    read_mapfile(adommap, cmdcode);

    fclose(sagemap);
    fclose(adommap);

    check_keymaps();

    // Set up keymaps for secondary modes (look, locate, target)
    // TODO: Doesn't act like ADOM if conflicts with movement cmds. Do we care?
    look_keymap = new KeyMap(*main_keymap);
    (*look_keymap)["z"] = cmdCancel;
    (*look_keymap)["Z"] = cmdCancel;
    (*look_keymap)["m"] = cmdMore;
    (*look_keymap)["M"] = cmdMore;

    locate_keymap = new KeyMap(*main_keymap);
    (*locate_keymap)["z"] = cmdCancel;
    (*locate_keymap)["Z"] = cmdCancel;
    (*locate_keymap)[" "] = cmdConfirm;

    target_keymap = new KeyMap(*main_keymap);
    (*target_keymap)["z"] = cmdCancel;
    (*target_keymap)["Z"] = cmdCancel;
    (*target_keymap)["m"] = cmdMore;
    (*target_keymap)["M"] = cmdMore;
    (*target_keymap)["t"] = cmdTarget;
    (*target_keymap)["T"] = cmdTarget;
    (*target_keymap)["+"] = cmdNextTarget;
    (*target_keymap)["-"] = cmdPrevTarget;

    reverse_keymap[cmdCancel]     = cmdstring_to_ch("Z");
    reverse_keymap[cmdConfirm]    = cmdstring_to_ch(" ");
    reverse_keymap[cmdMore]       = cmdstring_to_ch("M");
    reverse_keymap[cmdTarget]     = cmdstring_to_ch("T");
    reverse_keymap[cmdNextTarget] = cmdstring_to_ch("+");
    reverse_keymap[cmdPrevTarget] = cmdstring_to_ch("-");

    return 1;
}


/*-----------------------------------------------------------------------------
 * Functions to manage commands and keys
 */

const char *ch_to_cmdstring (int ch)
{
    static char cmdstring[5];

    switch (ch)
    {

            // Keys recognized by ADOM
        case KEY_UP:
            strcpy(cmdstring, "\\U");
            break;
        case KEY_DOWN:
            strcpy(cmdstring, "\\D");
            break;
        case KEY_LEFT:
            strcpy(cmdstring, "\\L");
            break;
        case KEY_RIGHT:
            strcpy(cmdstring, "\\R");
            break;
        case ' ':
            strcpy(cmdstring, "\\S");
            break;
        case '\\':
            strcpy(cmdstring, "\\\\");
            break;
        case 27:    // Escape key; shows up in Meta-key combos
            strcpy(cmdstring, "\\M");
            break;

            // Keys recognized only by Sage
            // Don't work under Linux or Windows - what's the point?
            /*case KEY_A1: // Home (keypad)
              strcpy(cmdstring, "\\H");
              break;
              case KEY_A3: // PgUp (keypad)
              strcpy(cmdstring, "\\+");
              break;
              case KEY_B2: // Center of keypad
              strcpy(cmdstring, "\\.");
              break;
              case KEY_C1: // End (keypad)
              strcpy(cmdstring, "\\E");
              break;
              case KEY_C3: // PgDown (keypad)
              strcpy(cmdstring, "\\-");
              break;
              case KEY_F(10):
              strcpy(cmdstring, "\\0");
              break;
              case KEY_F(11):
              strcpy(cmdstring, "\\A");
              break;
              case KEY_F(12):
              strcpy(cmdstring, "\\B");
              break;*/

            // Control keys, function keys, and everything else
        default:

            // General handling for control keys
            if (ch < 27)
            {
                strcpy(cmdstring, "\\Cx");
                cmdstring[2] = (char) ch - 1 + 'a';

                // General handling for function keys
            }

            else if (ch >= KEY_F0 && ch <= KEY_F(64))
            {
                real_sprintf(cmdstring, "\\%i", ch - KEY_F0);

                // General handling for (hopefully) printable, invalid keys
            }

            else if (ch > 256 || !isprint(ch))
            {
                log(log_errors, "Error: unknown character %i %c\n",
                    (int) ch, (char) ch);
                strcpy(cmdstring, " ");
            }

            // General handling for normal, printable keys
            else
            {
                cmdstring[0] = (char) ch;
                cmdstring[1] = '\0';
            }
    }

    return cmdstring;
}

Command lookup_command(string &cmd, KeyMap *keymap)
{
    KeyMap::iterator iter;
    iter = keymap->find(cmd.c_str());

    if (iter == keymap->end())
    {
        iter = keymap->upper_bound(cmd.c_str());

        if (iter == keymap->end()
            || (strncmp(cmd.c_str(), iter->first, cmd.length()) != 0))
        {
            return cmdInvalid;
        }

        else
        {
            return cmdIncomplete;
        }
    }

    else
    {
        return (Command) iter->second;
    }
}

int *cmdstring_to_ch (const char *cmdstring)
{
    int *ch_seq, i;
    ch_seq = new int[strlen(cmdstring) + 1];
    i = 0;

    while (*cmdstring)
    {
        if (*cmdstring != '\\')
        {
            ch_seq[i] = *cmdstring;
        }

        else
        {
            cmdstring++;

            switch (*cmdstring)
            {
                case 'U':
                    ch_seq[i] = KEY_UP;
                    break;
                case 'D':
                    ch_seq[i] = KEY_DOWN;
                    break;
                case 'L':
                    ch_seq[i] = KEY_LEFT;
                    break;
                case 'R':
                    ch_seq[i] = KEY_RIGHT;
                    break;
                case 'M':
                    ch_seq[i] = 27;
                    break;
                case 'S':
                    ch_seq[i] = ' ';
                    break;
                case '\\':
                    ch_seq[i] = '\\';
                    break;
                case 'C':
                    cmdstring++;

                    if (*cmdstring < 'a' && *cmdstring > 'z')
                    {
                        delete ch_seq;
                        return NULL;
                    }

                    ch_seq[i] = *cmdstring - 'a' + 1;
                    break;
                default:

                    if (*cmdstring >= '1' && *cmdstring <= '9')
                    {
                        ch_seq[i] = *cmdstring - '0' + KEY_F0;
                    }

                    else
                    {
                        delete ch_seq;
                        return NULL;
                    }
            }
        }

        i++;
        cmdstring++;
    }

    ch_seq[i] = *cmdstring;
    return ch_seq;
}


/*-----------------------------------------------------------------------------
 * KeyQueue class definition
 */

KeyQueue::KeyQueue() : i(0)
{
}

void KeyQueue::pop_front()
{
    i++;

    if (i == v.size())
    {
        i = 0;
        v.clear();
    }
}

void KeyQueue::push_cmd(Command cmd)
{
    int *i = reverse_keymap[cmd];

    while (*i)
    {
        push_back(*i);
        i++;
    }
}

void KeyQueue::push_string(const char *str)
{
    int *ch = cmdstring_to_ch(str), *i;
    i = ch;

    while (*i)
    {
        push_back(*i);
        i++;
    }

    delete ch;
}

void KeyQueue::push_queue(KeyQueue &q)
{
    vector<int>::iterator iter_front, iter_back;
    iter_front = q.v.begin();
    iter_back = q.v.end();
    v.insert(v.end(), iter_front, iter_back);
}

