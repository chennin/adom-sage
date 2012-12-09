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

int adom_version;

Config *config;
MsgMap *main_msgmap;

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

    for (int i = 0; i < numMacros; i++)
    {
        config->macro[i] = NULL;
    }

    // Read configuration from config file
    configfilename = *config_path + "/sage.cfg";
    configfile = fopen(configfilename.c_str(), "r");

    if (configfile == NULL)
    {
        return write_default_config();
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
                macro_num = ch - 'a';
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
    char *version = getenv("ADOM_VERSION");
    int adom_version = 0;

    if (version != NULL)
    {
        sscanf(version, "%i", &adom_version);
    }

    if (adom_version != 100)
    {
        config->spell_stats = 0;
    }

    if (adom_version != 110)
    {
        config->fix_flgs = 0;
    }

    return 1;
}

int read_msg_maps (void)
{
    string mapfilename;
    FILE *mapfile;
    char line[255];
    int no_skip = 0;
    attr_t attr = A_NORMAL;
    short color = COLOR_WHITE;
    int suppress = 0;
    MsgMap::iterator iter;

    mapfilename = *config_path + "/sage.msg";
    mapfile = fopen(mapfilename.c_str(), "r");

    if (mapfile == NULL)
    {
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

        // Process directives: color names, skip or noskip, suppress
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

            else
            {
                return sage_error("Invalid directive in message map\n");
            }
        }

        else
        {

            // Process messages.
            log(log_config, "Msgmap: %i %i %i %i %s\n", no_skip,
                (int) attr, (int) color, (int) suppress, line);
            iter = main_msgmap->find(line);

            if (iter != main_msgmap->end())
            {
                MsgInfo *mi = iter->second;
                mi->no_skip = no_skip;
                mi->attr = attr;
                mi->color = color;
                mi->subst = suppress ? "" : NULL;
            }

            else
                (*main_msgmap)[strdup(line)] =
                    new MsgInfo(no_skip, attr, color, suppress ? "" : NULL);
        }
    }

    fclose(mapfile);
    return 1;
}

