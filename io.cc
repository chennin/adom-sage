/*---------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * Functions dealing with I/O.
 */

#include "adom-sage.h"


/*---------------------------------------------------------------------------
 * Globals and type definitions
 */

typedef pair<attr_t, short> Color;
typedef STRING_CASE_HASH(Color) ColorMap;

int colorpair[8];   // Curses color pairs corresponding to basic colors
int highlight_colorpair[8];
ColorMap *colormap;

KeyQueue *key_queue;
KeyQueue *macro_queue;
KeyQueue *current_input;
KeyQueue *current_keys;
KeyQueue *prev_keys;

bool no_wrefresh;


/*---------------------------------------------------------------------------
 * Initialization
 */

void init_io(void)
{
    // I wish I knew enough about C++ to know if this code was really OK.
    colormap = new ColorMap;
    (*colormap)["blue"]          = Color(A_NORMAL, COLOR_BLUE);
    (*colormap)["green"]         = Color(A_NORMAL, COLOR_GREEN);
    (*colormap)["cyan"]          = Color(A_NORMAL, COLOR_CYAN);
    (*colormap)["red"]           = Color(A_NORMAL, COLOR_RED);
    (*colormap)["magenta"]       = Color(A_NORMAL, COLOR_MAGENTA);
    (*colormap)["brown"]         = Color(A_NORMAL, COLOR_YELLOW);
    (*colormap)["light gray"]    = Color(A_NORMAL, COLOR_WHITE);
    (*colormap)["dark gray"]     = Color(A_BOLD, COLOR_BLACK);
    (*colormap)["light blue"]    = Color(A_BOLD, COLOR_BLUE);
    (*colormap)["light green"]   = Color(A_BOLD, COLOR_GREEN);
    (*colormap)["light cyan"]    = Color(A_BOLD, COLOR_CYAN);
    (*colormap)["light red"]     = Color(A_BOLD, COLOR_RED);
    (*colormap)["light magenta"] = Color(A_BOLD, COLOR_MAGENTA);
    (*colormap)["yellow"]        = Color(A_BOLD, COLOR_YELLOW);
    (*colormap)["white"]         = Color(A_BOLD, COLOR_WHITE);

    key_queue = new KeyQueue;
    macro_queue = new KeyQueue;
    current_input = new KeyQueue;
    current_keys = new KeyQueue;
    prev_keys = new KeyQueue;
}


/*---------------------------------------------------------------------------
 * General I/O functions
 */

// To be called before waddch; converts to line-drawing chars if enabled
chtype convert_char (chtype in)
{
    if (in == '#' && config->draw_blocks)
    {
        return ACS_CKBOARD;
    }

    else if (in == '-' && config->draw_lines)
    {
        return ACS_HLINE;
    }

    else if (in == '|' && config->draw_lines)
    {
        return ACS_VLINE;
    }

    else if (in == '.' && config->draw_dots)
    {
        return ACS_BULLET;
    }

    else
    {
        return in;
    }
}

// To be called with winch; converts from line-drawing chars
// Hack - ncurses markes line-drawing chars with A_ALTCHARSET, but PDCurses
// just uses ASCII values above 127.  We design our code to be able to handle
// both.
chtype reverse_convert_char (chtype in)
{
    if ((in & A_ALTCHARSET) || ((in & A_CHARTEXT) > 127))
    {
        attr_t attr = (in & A_ATTRIBUTES) & !A_ALTCHARSET;
        chtype ch = in & (A_CHARTEXT | A_ALTCHARSET);

        if (ch == ACS_BULLET)
        {
            return attr | '.';
        }

        else if (ch == ACS_HLINE)
        {
            return attr | '-';
        }

        else if (ch == ACS_VLINE)
        {
            return attr | '|';
        }

        else if (ch == ACS_CKBOARD)
        {
            return attr | '#';
        }

        else
        {
            log(log_errors, "Error: unknown altcharset character %c\n",
                (char) (in & A_CHARTEXT));
            return in;
        }
    }

    else
    {
        return in;
    }
}

// Looks up a colorname and puts its attr and color in *attr and *color.
// Returns 1 if a valid colorname, 0 otherwise.
int get_color (char *colorname, attr_t *attr, short *color)
{
    ColorMap::iterator iter;
    iter = colormap->find(colorname);

    if (iter == colormap->end())
    {
        return 0;
    }

    else
    {
        *attr = iter->second.first;
        *color = iter->second.second;
        return 1;
    }
}

int morewait(WINDOW *win, int skip_more)
{
    int ch;

    if (skip_more)
    {
        return 1;
    }

    if (config->fast_more)
    {
        ch = real_wgetch(win);
    }

    else
    {
        do
        {
            ch = real_wgetch(win);
        }
        while (ch != ' ' && ch != '\r');
    }

    return (ch == '\r');
}

void show_msg(WINDOW *win, const char *msg)
{
    int y, x;
    getyx(win, y, x);
    real_wmove(win, 1, 0);
    wclrtoeol(win);
    real_wmove(win, 0, 0);
    wclrtoeol(win);
    real_waddnstr(win, msg, -1);
    real_waddnstr(win, " (more)", -1);
    morewait(win, 0);
    real_wmove(win, 0, 0);
    wclrtoeol(win);
    real_wmove(win, y, x);
}

int get_key(WINDOW *win, int cursor_visibility, attr_t cursor_attr,
            bool allow_mouse)
{
    int result;
    KeyQueue *queue = !key_queue->empty() ? key_queue : macro_queue;

    if (!queue->empty())
    {
        result = queue->front();
        current_keys->push_back(result);
        queue->pop_front();
        no_wrefresh = config->quiet_macros && !queue->empty();
    }

    else
    {
        int y, x, old_visibility;
        chtype ch;
        attr_t attrs;
        short pair;

        // Handle cursor - redraw or hide if desired.
        if (cursor_visibility != 1)
        {
            old_visibility = curs_set(cursor_visibility);

            if (cursor_attr != A_NORMAL)
            {
                // If ADOM set A_BOLD earlier, it will get or'ed in here unless
                // we reset to A_NORMAL.  Curses library can be a bit of a pain.
                getyx(win, y, x);
                wattr_get(win, &attrs, &pair, NULL);
                attrset(A_NORMAL);
                ch = real_winch(win);
                real_waddch(win, ch | cursor_attr);
                real_wmove(win, y, x);
            }
        }

        // Get key; make sure it's not a mouse event if asked to do so
        do
        {
            result = real_wgetch(win);
        }
        while (!allow_mouse && result == KEY_MOUSE);

        current_keys->push_back(result);

        // Restore cursor to normal
        if (cursor_visibility != 1)
        {
            curs_set(old_visibility);

            if (cursor_attr != A_NORMAL)
            {
                real_waddch(win, ch);
                real_wmove(win, y, x);
                wattr_set(win, attrs, pair, NULL);
            }
        }
    }

    return result;
}

// Replacement for wgetnstr that takes input off of the input queue if available
int get_string(WINDOW *win, char *str, int n)
{
    int ch;
    vector<int> v;
    KeyQueue *queue = !key_queue->empty() ? key_queue : macro_queue;

    // ungetch prepends to beginning of queue, so copy input to a temporary
    // vector so we can easily reverse it
    while (!queue->empty())
    {
        ch = queue->front();
        v.push_back(ch);
        queue->pop_front();

        if (ch == '\n')
        {
            break;
        }
    }

    while (!v.empty())
    {
        ungetch(v.back());
        v.pop_back();
    }

    no_wrefresh = config->quiet_macros && !queue->empty();
    return real_wgetnstr(win, str, n);
}

// Pads a string to the desired length with forced spaces.  ADOM uses
// underscores as forced spaces.
void pad_string(char *str, int pad_length)
{
    int pad_count;
    size_t i;

    for (i = 0, pad_count = pad_length; str[i] != '\0'; i++)
        if (str[i] != '\x03' && (str[i] <= '\xC0' || str[i] >= '\xCF'))
        {
            pad_count--;
        }

    for (i = strlen(str); pad_count > 0; i++, pad_count--)
    {
        str[i] = '_';
    }

    str[i] = '\0';
}

void fill_char(WINDOW *win, chtype ch, int y, int x, int len)
{
    real_wmove(win, y, x);

    for (int i = 0; i < len; i++)
    {
        real_waddch(win, ch);
    }
}


/*---------------------------------------------------------------------------
 * Mouse functions - system specific
 */

bool find_ch(WINDOW *win, chtype ch, int &y, int &x)
{
    int cury, curx, starty, startx;
    getyx(win, starty, startx);
    cury = starty;
    curx = startx;

    while (1)
    {
        // Advance to next position
        curx++;

        if (curx == COLS)
        {
            curx = 0;
            cury++;

            if (cury == LINES - 3)
            {
                cury = 2;
            }
        }

        if (cury == starty && curx == startx)
        {
            return false;
        }

        // Check position
        real_wmove(win, cury, curx);

        if ((reverse_convert_char(real_winch(win)) & A_CHARTEXT) == ch)
        {
            y = cury;
            x = curx;
            real_wmove(win, starty, startx);
            log(log_libcalls, "Found at %i, %i\n", y, x);
            return true;
        }
    }
}

void move_cursor(WINDOW *win, int y, int x)
{
    int cury, curx, dir;
    getyx(win, cury, curx);

    while (curx != x || cury != y)
    {
        log(log_libcalls, "From %i, %i to %i, %i\n", cury, curx, y, x);
        dir = (y < cury ? 2 : (y > cury ? 0 : 1)) * 3;
        dir += (x < curx ? 0 : (x > curx ? 2 : 1));
        key_queue->push_cmd((Command) (cmdMoveSW + dir));
        cury += (y < cury ? -1 : (y > cury ? 1 : 0));
        curx += (x < curx ? -1 : (x > curx ? 1 : 0));
    }
}


/*---------------------------------------------------------------------------
 * Mouse functions - system specific
 */

#ifndef _WIN32

MouseCookie mouse_enable(void)
{
    MouseCookie result;
    mousemask(ALL_MOUSE_EVENTS, &result);
    return result;
}

void mouse_restore(MouseCookie cookie)
{
    mousemask(cookie, NULL);
}

void get_mouse_event(int *y, int *x, int *button)
{
    MEVENT mevent;
    getmouse(&mevent);
    *y = mevent.y;
    *x = mevent.x;

    switch (mevent.bstate)
    {
        case BUTTON1_CLICKED:
            *button = 1;
            break;
        case BUTTON2_CLICKED:
            *button = 2;
            break;
        case BUTTON3_CLICKED:
            *button = 3;
            break;
        case BUTTON4_CLICKED:
            *button = 4;
            break;
        default:
            *button = 0;
    }
}

#else

MouseCookie mouse_enable(void)
{
    MouseCookie result;
    result = getmouse();
    mouse_set(ALL_MOUSE_EVENTS);
    return result;
}

void mouse_restore(MouseCookie cookie)
{
    mouse_set(cookie);
}

void get_mouse_event(int *y, int *x, int *button)
{
    request_mouse_pos();
    *y = MOUSE_Y_POS;
    *x = MOUSE_X_POS;
    *button = 0;

    for (int i = 1; i <= 4; i++)
        if (BUTTON_STATUS(i))
        {
            *button = i;
            break;
        }
}

#endif

