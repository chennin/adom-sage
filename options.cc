/*-----------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * Functions to process config files.
 */

#include <stdio.h>
#include <string>

#include "adom-sage.h"

Config *config;
MsgMap *main_msgmap;
MsgMap *regex_msgmap;
RegexMap *regex_map;

char *cwd_path;
string *config_path;

const char whitespace[] = " \t\r\v\f\n=";

int sage_error (const char *msg)
{
    printf("Unable to load ADOM Sage\n");
    printf("Details: %s\n", msg);
    printf("ADOM will run normally, without any modifications by ADOM Sage\n\n");
    printf("[PRESS ENTER TO CONTINUE]");
    getchar();
    config->passive = 1;
    return 0;
}

int init_config (void)
{
    const char *param;
    config = new Config;

    // Save current working directory
    cwd_path = getcwd(NULL, 0);

    // Locate configuration directory
    param = getenv("HOME");

    if (param == NULL)
    {
        return sage_error("Cannot locate home directory");
    }

    config_path = new string;
    *config_path = getenv("HOME");
    *config_path += ADOM_DATA_PATH;
    return 1;
}

int write_default_config (void)
{
    string configfilename;
    FILE *configfile;

    configfilename = *config_path + "/sage.cfg";
    configfile = fopen(configfilename.c_str(), "w");

    if (configfile == NULL)
    {
        return sage_error("Unable to open or create config file");
    }

    fputs("Auto_Swap_Neutral    = true\n", configfile);
    fputs("Cursor_Style         = default\n", configfile);
    fputs("Draw_Blocks          = true\n", configfile);
    fputs("Draw_Dots            = false\n", configfile);
    fputs("Draw_Lines           = true\n", configfile);
    fputs("Fast_Selling         = false\n", configfile);
    fputs("Fix_Flgs             = true\n", configfile);
    fputs("Fix_Typos            = true\n", configfile);
    fputs("Mark_Spells          = true\n", configfile);
    fputs("Mindcraft_Stats      = true\n", configfile);
    fputs("More_Weapon_Stats    = true\n", configfile);
    fputs("Mouse                = true\n", configfile);
    fputs("Quiet_Macros         = true\n", configfile);
    fputs("Select_Starsign      = true\n", configfile);
    fputs("Enable_Autosave      = true\n", configfile);
    fputs("Enable_Reroller      = true\n", configfile);
    fputs("Short_Alchemy        = false\n", configfile);
    fputs("Short_Named_Monsters = true\n", configfile);
    fputs("Spell_Stats          = false\n", configfile);
    fputs("Suppress_ToEF        = true\n", configfile);
    fclose(configfile);

    return 1;
}

int write_default_msg_map(void)
{
    string mapfilename;
    FILE *mapfile;

    mapfilename = *config_path + "/sage.msg";
    mapfile = fopen(mapfilename.c_str(), "w");

    if (mapfile == NULL)
    {
        return sage_error("Unable to open or create map file");
    }

    fputs(
        "# Player accomplishments\n"
        ":no_skip\n"
        ":yellow\n"
        "just have completed your current quest.\n"
        "Congratulations! You advance to level %d.\n"
        "\n"
        "# Benefical date effects\n"
        "You feel that starting your adventure on Creator's Day was a good idea.\n"
        "A lone star leads you to this remote valley.\n"
        "You feel well prepared for the coming year.\n"
        "\n"
        "# Stat gain\n"
        ":no_skip\n"
        ":light green\n"
        "Your muscles feel stronger.\n"
        "Your muscles feel stronger!\n"
        "You feel studious.\n"
        "You feel studious!\n"
        "Your will seems inflexible.\n"
        "Your will seems inflexible!\n"
        "Your movements are getting swifter.\n"
        "Your movements are getting swifter!\n"
        "Your health increases.\n"
        "Your health increases!\n"
        "You feel very self-confident.\n"
        "You feel very self-confident!\n"
        "Your looks improve.\n"
        "Your looks improve!\n"
        "You feel more in touch with the world.\n"
        "You feel more in touch with the world!\n"
        "Your senses sharpen.\n"
        "Your senses sharpen!\n"
        "feel good about your %s potential.\n"
        "feel very good about your %s level.\n"
        "\n"
        "# Stat loss\n"
        ":no_skip\n"
        ":red\n"
        "You feel as if this was not a good idea!\n"
        "You seem to get less perceptive.\n"
        "You are getting out of touch with everything.\n"
        "You are growing a wart.\n"
        "You feel reserved.\n"
        "It seems that you are getting a cold.\n"
        "You are getting shaky.\n"
        "You feel soft-hearted.\n"
        "Thinking seems to get tougher.\n"
        "Your muscles soften.\n"
        "You feel weak!\n"
        "You feel exhausted!\n"
        "You suddenly age!\n"
        "\n"
        "# Skill gain\n"
        ":no_skip\n"
        ":light blue\n"
        "Your '%s' skill increases by %+d\n"
        "to %d.\n"
        "improve in the area of '%s' (from %s to\n"
        "%s).\n"
        "improve your skill in the shield group to level %d.\n"
        "improve your skill in the '%s' weapon group to level %d.\n"
        "improve your skill with the '%s' weapon category to level %d.\n"
        "become more efficient in using the '%s' spell (now %+d).\n"
        "\n"
        "# Equipment damage/destruction\n"
        ":no_skip\n"
        ":red\n"
        "Your %s are destroyed.\n"
        "Your %s are damaged.\n"
        "Your %s corrodes!\n"
        "Your %s corrode!\n"
        "They are destored.\n"
        "Your weapon breaks!\n"
        "Your %s %s damaged.\n"
        "Your %s %s destroyed!\n"
        "Your %s rust%s!\n"
        "%ld %sof your %s %s!\n"
        "Your %s %s!\n"
        "Your %s is finally damaged beyond repair!\n"
        "Something on the floor melts down into a puddle of metal!\n"
        "Something on the ground %s!\n"
        "The %s %s transformed into a rusty heap!\n"
        "The %s rust%s!\n"
        "They are annihilated!\n"
        "It is annihilated!\n"
        "\n"
        "# Detrimental date effects\n"
        "You feel unlucky.\n"
        "You feel slightly exhausted today.\n"
        "\n"
        "# Food rotting\n"
        ":no_skip\n"
        ":brown\n"
        "Your equipment suddenly seems to weigh less.\n"
        "Your %s rots away!\n"
        "Your %s starts to rot!\n"
        "Your %s slowly rots away!\n"
        "notice %s %s rotting away.\n"
        "\n"
        "# Spell effect messages\n"
        ":no_skip\n"
        ":cyan\n"
        "You feel your spirits rise!\n"
        "You feel your spirits soar!\n"
        "Your mood darkens.\n"
        "Your eyes itch.\n"
        "Your eyesight adjusts to normality once more.\n"
        "You feel like Atlas himself.\n"
        "You feel your great strength waning.\n"
        "Suddenly your extraordinary carrying capacity fails you.\n"
        "Suddenly everything around you seems to speed up!\n"
        "Suddenly everything around you seems to slow down.\n"
        "Suddenly you can't see yourself!\n"
        "For a moment you were gone... then your eyes adjust.\n"
        "Suddenly you can see yourself again!\n"
        "\n"
        "# Clerical to arcane spellbook translations\n"
        ":replace\n"
        " of Rain of Sorrow| of Rain of Sorrow (Acid Ball)\n"
        " of Baptism of Fire| of Baptism of Fire (Burning Hands)\n"
        " of Divine Favour| of Divine Favour (Create Item)\n"
        " of Greater Divine Touch| of Greater Divine Touch (Death Ray)\n"
        " of Dispel Undead| of Dispel Undead (Destroy Undead)\n"
        " of Hellish Flames| of Hellish Flames (Fire Bolt)\n"
        " of Major Punishment| of Major Punishment (Fireball)\n"
        " of Nether Bolt| of Nether Bolt (Frost Bolt)\n"
        " of Greater Enlightenment| of Greater Enlightenment (Greater Identify)\n"
        " of Freezing Fury| of Freezing Fury (Ice Ball)\n"
        " of Spellbook of Enlightenment| of Spellbook of Enlightenment (Identify)\n"
        " of Invoked Devastation| of Invoked Devastation (Improved Fireball)\n"
        " of Veil of the Gods| of Veil of the Gods (Invisibility)\n"
        " of Divine Key| of Divine Key (Knock)\n"
        " of Heavenly Fury| of Heavenly Fury (Lightning Ball)\n"
        " of Divine Wrath| of Divine Wrath (Lightning Bolt)\n"
        " of Seal of the Spheres| of Seal of the Spheres (Magic Lock)\n"
        " of Knowledge of the Ancients| of Knowledge of the Ancients (Magic Map)\n"
        " of Minor Punishment| of Minor Punishment (Magic Missile)\n"
        " of Divine Digger| of Divine Digger (Mystic Shovel)\n"
        " of Holy Awe| of Holy Awe (Scare Monster)\n"
        " of Lordly Might| of Lordly Might (Strength of Atlas)\n"
        " of Lesser Divine Touch| of Lesser Divine Touch (Stun Ray)\n"
        " of Ethereal Bridge| of Ethereal Bridge (Teleport)\n"
        "\n"
        "# Unnecessary messages\n"
        ":suppress\n"
        "Select the companion you want to issue an order to.\n"
        "Select the slave to command.\n"
        "Extended drop command.\n"
        "#(easily cutting through %s armor with the %s)\n"
        "#You lust for blood!\n"
        "#The %s returns!\n"
        "#You manage to catch it.\n"
        "#You fail to catch it.\n"
        "\n"
        , mapfile);
    fclose(mapfile);
    return 1;
}

// TODO: Config file processing is really weak here
int read_config (void)
{
    const char *param;
    string configfilename;
    FILE *configfile;
    char line[255], *param_name, *param_value;

    // Read configuration from environment
    param = getenv("SAGE_PASSIVE");
    config->passive = (param != NULL);

    // Setup defaults for config file values
    config->auto_swap_neutral = 1;
    config->fast_selling = 0;
    config->fast_more = 0;
    config->mindcraft_stats = 1;
    config->fix_flgs = 1;
    config->fix_typos = 1;
    config->more_weapon_stats = 1;
    config->select_starsign = 1;
    config->enable_autosave = 1;
    config->enable_reroller = 1;
    config->short_alchemy = 0;
    config->short_named_monsters = 1;
    config->spell_stats = 1;
    config->suppress_toef = 1;
    config->mouse = 1;
    config->draw_blocks = 1;
    config->draw_lines = 1;
    config->draw_dots = 0;
    config->cursor_visibility = 1;
    config->cursor_attr = A_NORMAL;
    config->quiet_macros = 1;
    config->auto_dump_flg = 1;

    for (int i = 0; i < numMacros; i++)
    {
        config->macro[i] = NULL;
    }

    // Read configuration from config file
    configfilename = *config_path + "/sage.cfg";
    configfile = fopen(configfilename.c_str(), "r");

    if (configfile == NULL)
    {
        log(log_config, "Config: sage.cfg not found, writing defaults\n");
        write_default_config();
        configfile = fopen(configfilename.c_str(), "r");
    }
    if (configfile == NULL)
    {
        return sage_error("Something went very wrong, still can't read config file!\n");
    }
    while (!feof(configfile))
    {

        // Read line.  Remove trailing CR/LFs.
        if (fgets(line, sizeof(line), configfile) == NULL)
        {
            break;
        }

        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }

        if (line[strlen(line) - 1] == '\r')
        {
            line[strlen(line) - 1] = '\0';
        }

        // Skip blank lines and comments.
        if (line[0] == '#' || strlen(line) == 0)
        {
            continue;
        }

        // Locate parameter name and value.
        param_value = strchr(line, '=');

        if (param_value == NULL)
        {
            return sage_error("Bad line in config file\n");
        }

        param_value++;
        param_value += strspn(param_value, whitespace);
        param_name = line;
        param_name[strcspn(param_name, whitespace)] = '\0';

        // Process parameter.
        log(log_config, "Config: %s = %s\n", param_name, param_value);

        if (strcasecmp(param_name, "auto_swap_neutral") == 0)
        {
            config->auto_swap_neutral = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "auto_dump_flg") == 0)
        {
            config->auto_dump_flg = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "fast_more") == 0)
        {
            config->fast_more = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "fast_selling") == 0)
        {
            config->fast_selling = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "mindcraft_stats") == 0)
        {
            config->mindcraft_stats = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "fix_flgs") == 0)
        {
            config->fix_flgs = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "fix_typos") == 0)
        {
            config->fix_typos = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "more_weapon_stats") == 0)
        {
            config->more_weapon_stats = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "select_starsign") == 0)
        {
            config->select_starsign = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "enable_autosave") == 0)
        {
            config->enable_autosave = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "enable_reroller") == 0)
        {
            config->enable_reroller = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "spell_stats") == 0)
        {
            config->spell_stats = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "suppress_toef") == 0)
        {
            config->suppress_toef = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "short_alchemy") == 0)
        {
            config->short_alchemy = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "short_named_monsters") == 0)
        {
            config->short_named_monsters = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "quiet_macros") == 0)
        {
            config->quiet_macros = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "mouse") == 0)
        {
            config->mouse = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "draw_blocks") == 0)
        {
            config->draw_blocks = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "draw_lines") == 0)
        {
            config->draw_lines = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "draw_dots") == 0)
        {
            config->draw_dots = (strcasecmp(param_value, "true") == 0);
        }

        if (strcasecmp(param_name, "cursor_style") == 0)
        {
            if (strcasecmp(param_value, "hide") == 0)
            {
                config->cursor_visibility = 0;
            }

            else if (strcasecmp(param_value, "block") == 0)
            {
                config->cursor_visibility = 2;
            }

            else if (strcasecmp(param_value, "underline") == 0)
            {
                config->cursor_visibility = 0;
                config->cursor_attr = A_UNDERLINE;
            }

            else if (strcasecmp(param_value, "reverse") == 0)
            {
                config->cursor_visibility = 0;
                config->cursor_attr = A_REVERSE;
            }
        }

        if (strncasecmp(param_name, "macro_", strlen("macro_")) == 0 &&
            strlen(param_name) == strlen("macro_") + 1)
        {
            char ch = tolower(*(param_name + strlen("macro_")));
            int macro_num = 0;

            if (ch >= '0' && ch <= '9')
            {
                macro_num = ch - '0';
            }

            else if (ch >= 'a' && ch <= 'z')
            {
                macro_num = ch - 'a' + 10;
            }

            if (config->macro[macro_num])
            {
                free(config->macro[macro_num]);
            }

            config->macro[macro_num] = strdup(param_value);
        }
    }

    fclose(configfile);

    // Additional processing for certain config options
    if (config->fix_typos)
    {
        (*main_msgmap)["You feel absolutly mad!"] =
            new MsgInfo("You feel absolutely mad!");
        (*main_msgmap)["You feel absolutly lunatic!"] =
            new MsgInfo("You feel absolutely lunatic!");
    }

    // Version-specific processing
    int adom_version = get_version();
    if ((adom_version != 100) && (config->spell_stats != 0))
    {
        config->spell_stats = 0;
    }

    if ((adom_version != 110) && (config->fix_flgs != 0))
    {
        log(log_config, "Config: ADOM version not 1.1.0, disabling flg fix.\n");
        config->fix_flgs = 0;
    }

    if ((adom_version != 111) && (adom_version != 12018) && (config->enable_autosave != 0)) {
	log(log_config, "Config: ADOM version not 1.1.1 or 1.2.0p18, disabling autosaver.\n");
	config->enable_autosave = 0;
    }

    if ((adom_version != 111) && (adom_version != 12018) && (config->enable_reroller != 0)) {
	log(log_config, "Config: ADOM version not 1.1.1 or 1.2.0p18, disabling char reroller.\n");
	config->enable_reroller = 0;
    }

    if ((adom_version == 12017) && (config->auto_swap_neutral != 0)) {
        log(log_config, "Config: ADOM version 1.2.0p17 detected, disabling auto swap with neutrals.\n");
        config->auto_swap_neutral = 0;
    }

    if ((adom_version >= 12017) && (config->mindcraft_stats != 0)) {
        log(log_config, "Config: ADOM version 1.2.0p17 or greater detected, disabling mindcraft stats.\n");
        config->mindcraft_stats = 0;
    }

    return 1;
}

int read_msg_maps (void)
{
    string mapfilename;
    FILE *mapfile;
    string adommsgname;
    FILE *adommsg;
    char line[255];
    char *delim_pos;
    char *subst;
    int no_skip = 0;
    attr_t attr = A_NORMAL;
    short color = COLOR_WHITE;
    int suppress = 0;
    MsgMap::iterator iter;
    MsgMap* cur_msgmap = main_msgmap;

    regex_map = new RegexMap;

    // Look for sage.msg
    mapfilename = *config_path + "/sage.msg";
    mapfile = fopen(mapfilename.c_str(), "r");

    // Look for adom.msg (1.2.0p17+)
    adommsgname = *config_path + "/adom.msg";
    adommsg = fopen(adommsgname.c_str(), "r");

    if (mapfile == NULL) // No existing sage.msg ...
    {
	if (adommsg == NULL) // ... And no adom.msg: create sage.msg
	{
          // Write out default sage.msg
          if (!write_default_msg_map())
          {
              return 0;
          }

          mapfile = fopen(mapfilename.c_str(), "r");

          if (mapfile == NULL)
          {
              return sage_error ("Something very strange happened-still can't read message map");
          }
	}
	else // ... And yes adom.msg: don't create default map
	{
	  log(log_config, "Msgmap: adom.msg exists, not creating default sage.msg\n");
	  return 1;
	}
    }
    else // Existing sage.msg ...
    {
      if ((adommsg != NULL) && (get_version() >= 12017)) // ... And existing adom.msg and version > 1.2.0p17
      {
	log(log_config, "Msgmap: adom.msg and sage.msg exist, beware of conflicts\n");
        printf("WARNING: sage.msg and adom.msg both exist. Since 1.2.0p17, ADOM supports the\n"
               "same features Sage does in sage.msg. Be careful of duplicate directives in\n"
               "the .msg files.\n");
      }
    }

    while (!feof(mapfile))
    {

        // Read line.  Remove trailing CR/LFs.
        if (fgets(line, sizeof(line), mapfile) == NULL)
        {
            break;
        }

        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }

        if (line[strlen(line) - 1] == '\r')
        {
            line[strlen(line) - 1] = '\0';
        }

        // Skip blank lines and comments.
        if (line[0] == '#' || strlen(line) == 0)
        {
            continue;
        }

        // Process directives: color names, skip or noskip, suppress, replace, regex
        if (line[0] == ':')
        {
            if (strcasecmp(line, ":suppress") == 0)
            {
                attr = A_NORMAL;
                color = COLOR_WHITE;
                suppress = 1;
            }

            else if (strcasecmp(line, ":no_skip") == 0)
            {
                suppress = 0;
                no_skip = 1;
            }

            else if (strcasecmp(line, ":allow_skip") == 0)
            {
                suppress = 0;
                no_skip = 0;
            }

            else if (get_color(line + 1, &attr, &color))
            {
                suppress = 0;
            }

            else if (strcasecmp(line, ":regex") == 0)
            {
                cur_msgmap = regex_msgmap;
            }

            else if (strcasecmp(line, ":no_regex") == 0)
            {
                cur_msgmap = main_msgmap;
            }

            else if (strcasecmp(line, ":replace") == 0)
            {
                attr = A_NORMAL;
                color = COLOR_WHITE;
                suppress = 0;
            }

            else
            {
                return sage_error("Invalid directive in message map\n");
            }
        }

        else
        {

            subst = NULL;
            // Process messages.
            delim_pos = strchr(line, '|');
            if ((delim_pos != NULL) && !suppress)
            {
                subst = strdup(delim_pos + 1);
                *delim_pos = '\0';
            }

            log(log_config, "Msgmap: %i %i %i %i %s\n", no_skip,
                (int) attr, (int) color, (int) suppress, line);
            iter = cur_msgmap->find(line);

            if (cur_msgmap == regex_msgmap)
            {
                regex_t *target_regex = new regex_t;
                int comperr = regcomp(target_regex, line, REG_NOSUB);
                if (comperr > 0) {
                        size_t length = regerror(comperr, target_regex, NULL, 0);
                        char *buffer = (char*)malloc(length);
                        if (buffer == NULL) { perror("Unable to allocate memory to display error."); }
                        regerror(comperr, target_regex, buffer, length);
                        return sage_error(buffer);
                }
                (*regex_map)[strdup(line)] = target_regex;
            }

            if (iter != cur_msgmap->end())
            {
                MsgInfo *mi = iter->second;
                mi->no_skip = no_skip;
                mi->attr = attr;
                mi->color = color;
                if (subst != NULL)
                {
                    mi->subst = subst;
                }
                else
                {
                    mi->subst = suppress ? "" : NULL;
                }
            }

            else
                (*cur_msgmap)[strdup(line)] =
                    new MsgInfo(no_skip, attr, color, suppress ? "" : subst);
        }
    }

    fclose(mapfile);
    return 1;
}

