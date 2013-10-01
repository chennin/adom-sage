/*---------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * Message handlers
 */

#include "adom-sage.h"
#include "states.h"


#define MSG_HANDLER(handler) void handler (StateCmdProcessor *state, \
        void *data, char *str, const char *format, va_list ap)


/*---------------------------------------------------------------------------
 * Config option tracking
 */

const char *fastmore_msg =
    "[The game will now accept any key at the (more) prompt.]";
MSG_HANDLER(fastmore_handler)
{
    config->fast_more = 1;
    state->ignore_more++;
}

const char *no_fastmore_msg =
    "[The game will now accept only SPACE and ENTER at the (more) prompt.]";
MSG_HANDLER(no_fastmore_handler)
{
    config->fast_more = 0;
    state->ignore_more++;
}


/*---------------------------------------------------------------------------
 * State tracking
 */

const char *levelup_msg =
    "Congratulations! You advance to level %d.";
MSG_HANDLER(levelup_handler)
{
    state->next_state = new StateLevelUp;
}

const char *examine_msg =
    "Use the movement keys to position the cursor. Abort with [SPACE] or [Z].";
MSG_HANDLER(examine_handler)
{
    push_state(new StateExamine);
}

const char *locate_msg =
    "Move the cursor to the desired position and press [SPACE] when done.";
MSG_HANDLER(locate_handler)
{
    push_state(new StateLocate);
}

const char *target_msg =
    "[T]arget --";
MSG_HANDLER(target_handler)
{
    // HACK
    if (strcmp(cur_state()->name(), "target") != 0)
    {
        push_state(new StateTarget);
    }
}

const char *recall_msg =
    "<<<<<--- %s (";
MSG_HANDLER(recall_handler)
{
    push_state(new StateRecall);
}

const char *minstrel_msg
= "The Mad Minstrel sings a song.";
MSG_HANDLER(minstrel_handler)
{
    state->next_state = new StateInfoScreen;
}

const char *game_summary_msg
= "Game Summary for '%s'";
MSG_HANDLER(game_summary_handler)
{
    push_state(new StateGameSummary);
}

const char *memorial_msg
= "Do you want to create a memorial file about your character? [y/N]_";
MSG_HANDLER(memorial_handler)
{
    push_state(new StateMemorial(state));
}

const char *flg_success_msg
= "Final log written to '%s' in directory '%s'.";
MSG_HANDLER(flg_success_handler)
{
    push_state(new StateFlgSuccess(state));
}

/*---------------------------------------------------------------------------
 * Game status tracking
 */

MSG_HANDLER(location_handler)
{
    log(log_game_status, "Game status: location %i\n", (int)(intptr_t) data);
    game_status->loc = (Location) (int)(intptr_t) data;
}

const char *stat_msg =
    "%s:%2d_";
MSG_HANDLER(stat_handler)
{
    char *stat_name;
    int stat_val;
    stat_name = va_arg(ap, char *);
    stat_val = va_arg(ap, int);
    log(log_game_status, "Game status: %s %d\n", stat_name, stat_val);
    game_status->player_attr[(*attr_map)[stat_name]] = stat_val;
}

const char *exp_msg =
    "Exp: %d/%lu__";
const char *exp_msg_2 =
    "Exp: %d/%lld__";
MSG_HANDLER(exp_handler)
{
    int level;
    long unsigned exp;
    level = va_arg(ap, int);
    exp = va_arg(ap, long unsigned);
    game_status->player_level = level;
    log(log_game_status, "Game status: player level %i\n",
        game_status->player_level);
}


/*---------------------------------------------------------------------------
 * ToEF messages
 */

const char *toef_msg =
    "The air is extremely hot!";
const char *ring_ice_msg =
    "Your equipment is protected from the fire by a thin sheet of ice emanating from your hands.";
MSG_HANDLER(toef_handler)
{
    if (config->suppress_toef && game_status->loc == locToEF)
    {
        strcpy(str, "");
    }
}


/*---------------------------------------------------------------------------
 * Resistance messages
 */

static int resisted = 0;
const char *resist_msg =
    "You resist the";
MSG_HANDLER(resist_handler)
{
    resisted = 1;
    strcpy(str, "");
}

const char *resist_fire_msg =
    "searing flames.";
const char *resist_ice_msg =
    "icy blast.";
const char *resist_shock_msg =
    "shock waves.";
const char *resist_acid_msg =
    "acidic fluids.";
const char *resist_water_msg =
    "water blast.";
MSG_HANDLER(resist_elem_handler)
{
    if (resisted)
    {
        if (config->suppress_toef && game_status->loc == locToEF
            && ((int)(intptr_t) data) == elemFire)
        {
            strcpy(str, "");
        }

        else
        {
            sprintf(str, "You resist the %s", format);
        }

        resisted = 0;
    }
}


/*---------------------------------------------------------------------------
 * Fast selling support
 */

static int sale_offer = 0;
const char *sale_offer_msg =
    "Do you want to accept the offer? [Y/n]_";
MSG_HANDLER(sale_offer_handler)
{
    if (config->fast_selling)
    {
        strcpy(str, "Do you accept? [Y/n]_");
        sale_offer = 1;
    }
}

const char *no_sale_msg =
    "\"Then take yer stuff with ye!\"";
MSG_HANDLER(no_sale_handler)
{
    if (config->fast_selling)
    {
        strcpy(str, "");
        sale_offer = 0;
    }
}

const char *drop_msg =
    "You drop the %s.";
MSG_HANDLER(drop_handler)
{
    if (sale_offer && config->fast_selling)
    {
        strcpy(str, "");
        sale_offer = 0;
    }
}


/*---------------------------------------------------------------------------
 * Auto-swap-with-neutrals support
 */

const char *confirm_msg
= "Really %s? [%s]_";
MSG_HANDLER(confirm_handler)
{
    if (config->auto_swap_neutral && (state->last_cmd >= cmdMoveSW)
        && state->last_cmd <= cmdMoveNE
        && reverse_keymap[cmdSwapPosition])
    {
        char *s = va_arg(ap, char *);

        if (strncmp(s, "attack ", strlen("attack ")) == 0
            || strncmp(s, "backstab ", strlen("backstab ")) == 0)
        {

            // If we're asked, "Really attack/backstab the foo?", then
            // attempt to swap places instead.
            push_state(new StateSwap(state, state->last_cmd));
            str[0] = '\0';
        }
    }
}

// Version messages
const char *flg_version_msg = "Version 1.2.0";

const char *version_msg_111 = "Ancient Domains of Mystery__ Version 1.1.1";
const char *version_msg_100 = "Ancient Domains of Mystery__ Version 1.0.0";
const char *version_msg_120 = "Ancient Domains of Mystery__ Version 1.2.0";

/*---------------------------------------------------------------------------
 * Initialization
 */

void init_msg_maps(void)
{
    main_msgmap = new MsgMap;
    regex_msgmap = new MsgMap;

    // Tracking state changes
    (*main_msgmap)[levelup_msg] = new MsgInfo(levelup_handler, NULL);
    (*main_msgmap)[examine_msg] = new MsgInfo(examine_handler, NULL);
    (*main_msgmap)[locate_msg] = new MsgInfo(locate_handler, NULL);
    (*main_msgmap)[target_msg] = new MsgInfo(target_handler, NULL);
    (*main_msgmap)[recall_msg] = new MsgInfo(recall_handler, NULL);
    (*main_msgmap)[minstrel_msg] = new MsgInfo(minstrel_handler, NULL);
    (*main_msgmap)[game_summary_msg] = new MsgInfo(game_summary_handler, NULL);
    (*main_msgmap)[memorial_msg] = new MsgInfo(memorial_handler, NULL);
    (*main_msgmap)[flg_success_msg] = new MsgInfo(flg_success_handler, NULL);

    // Tracking configuration changes
    (*main_msgmap)[fastmore_msg] = new MsgInfo(fastmore_handler, NULL);
    (*main_msgmap)[no_fastmore_msg] = new MsgInfo(no_fastmore_handler, NULL);

    // Tracking game status changes
    (*main_msgmap)["DrCh_"]  = new MsgInfo(location_handler,
                                           (void *) locWilderness);
    (*main_msgmap)["TF: 1_"] = new MsgInfo(location_handler, (void *) locToEF);
    (*main_msgmap)["TF: 2_"] = new MsgInfo(location_handler, (void *) locToEF);
    (*main_msgmap)["TF: 3_"] = new MsgInfo(location_handler, (void *) locToEF);
    (*main_msgmap)["TF: 4_"] = new MsgInfo(location_handler, (void *) locToEF);
    (*main_msgmap)[stat_msg] = new MsgInfo(stat_handler, NULL);
    (*main_msgmap)[exp_msg] = new MsgInfo(exp_handler, NULL);
    (*main_msgmap)[exp_msg_2] = new MsgInfo(exp_handler, NULL);

    // Tower of Eternal Flames messages
    (*main_msgmap)[toef_msg] = new MsgInfo(toef_handler, NULL);
    (*main_msgmap)[ring_ice_msg] = new MsgInfo(toef_handler, NULL);

    // Resistance messages
    (*main_msgmap)[resist_msg] = new MsgInfo(resist_handler, NULL);
    (*main_msgmap)[resist_fire_msg] = new MsgInfo(resist_elem_handler,
            (void *) elemFire);
    (*main_msgmap)[resist_ice_msg] = new MsgInfo(resist_elem_handler,
            (void *) elemIce);
    (*main_msgmap)[resist_shock_msg] = new MsgInfo(resist_elem_handler,
            (void *) elemShock);
    (*main_msgmap)[resist_acid_msg] = new MsgInfo(resist_elem_handler,
            (void *) elemAcid);
    (*main_msgmap)[resist_water_msg] = new MsgInfo(resist_elem_handler,
            (void *) elemWater);

    // Selling messages
    (*main_msgmap)[sale_offer_msg] = new MsgInfo(sale_offer_handler, NULL);
    (*main_msgmap)[no_sale_msg] = new MsgInfo(no_sale_handler, NULL);
    (*main_msgmap)[drop_msg] = new MsgInfo(drop_handler, NULL);

    // Auto swap with neutrals
    (*main_msgmap)[confirm_msg] = new MsgInfo(confirm_handler, NULL);

    // Version info
    attr_t attr = A_NORMAL;
    short color = COLOR_WHITE;
    get_color("yellow", &attr, &color);
    (*main_msgmap)[flg_version_msg] = new MsgInfo("Version 1.2.0 with ADOM Sage " SAGE_VERSION);
    (*main_msgmap)[version_msg_111] = new MsgInfo(0, attr, color, "Ancient Domains of Mystery__ Version 1.1.1 with ADOM Sage " SAGE_VERSION);
    (*main_msgmap)[version_msg_100] = new MsgInfo(0, attr, color, "Ancient Domains of Mystery__ Version 1.0.0 with ADOM Sage " SAGE_VERSION);
    (*main_msgmap)[version_msg_120] = new MsgInfo(0, attr, color, "Ancient Domains of Mystery__ Version 1.2.0 with ADOM Sage " SAGE_VERSION);
}

