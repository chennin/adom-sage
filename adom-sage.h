/*---------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * Main header file for ADOM Sage
 */

#include <stdio.h>
#include <stdarg.h>
#include <regex.h>

#include "stl.h"

#define SAGE_VERSION "0.9.18"

// curses.h must come after STL stuff - PDCurses defines some macros that
//   break MSVC's STL
#include <curses.h>

#include "const.h"

class State;
class StateCmdProcessor;

int get_version();

/*---------------------------------------------------------------------------
 * System-dependent stuff - library functions that we replace, non-ANSI
 * functions that we use, and directories and filenames used by ADOM
 */

#ifndef _WIN32

#include <unistd.h>

// Linux - Typedefs for shadowed functions
typedef int (*INIT_PAIR)(short pair, short f, short b);
typedef int (*WCLEAR)(WINDOW *win);
typedef int (*WREFRESH)(WINDOW *win);
typedef int (*WMOVE)(WINDOW *win, int y, int x);
typedef int (*WADDCH)(WINDOW *win, chtype ch);
typedef int (*WADDNSTR)(WINDOW *win, const char *str, int n);
typedef chtype (*WINCH)(WINDOW *win);
typedef int (*WGETCH)(WINDOW *win);
typedef int (*WGETNSTR)(WINDOW *win, char *str, int n);
typedef int (*SPRINTF)(char *str, const char *format, ...);
typedef int (*VSPRINTF)(char *str, const char *format, va_list ap);
typedef int (*VSNPRINTF)(char *str, size_t size, const char *format, va_list ap);
typedef int (*UNLINK)(const char *pathname);

// Linux - Pointers to real library functions
extern INIT_PAIR real_init_pair;
extern WCLEAR    real_wclear;
extern WREFRESH  real_wrefresh;
extern WMOVE     real_wmove;
extern WADDCH    real_waddch;
extern WADDNSTR  real_waddnstr;
extern WINCH     real_winch;
extern WGETCH    real_wgetch;
extern WGETNSTR  real_wgetnstr;
extern SPRINTF   real_sprintf;
extern VSPRINTF  real_vsprintf;
extern VSNPRINTF real_vsnprintf;
extern UNLINK    real_unlink;

// Linux - ADOM config directories
#define ADOM_DATA_PATH "/.adom.data"

// Linux - data type definitions
#define ULONG_PTR unsigned
// (Casting pointers to ULONG_PTR silences some warnings on Windows)

#else

#include <direct.h>

// Win32 - functions from libc.cpp and curses.cpp
int libc_unlink(const char *pathname);

// Win32 - #defines for library functions
#define real_init_pair ::init_pair
#define real_wclear ::wclear
#define real_wrefresh ::wrefresh
#define real_wmove ::wmove
#define real_waddch ::waddch
#define real_waddnstr ::waddnstr
#define real_winch ::winch
#define real_wgetch ::wgetch
#define real_wgetnstr ::wgetnstr
#define real_sprintf ::sprintf
#define real_vsprintf ::vsprintf
#define real_vsnprintf ::vsnprintf
#define real_unlink ::libc_unlink

// Win32 - non-ANSI functions
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define getcwd _getcwd

// Win32 - ADOM config directories
#define ADOM_DATA_PATH "\\adom_dat"
#define ADOM_KBD_PATH "\\adom.kbd"

#endif


/*---------------------------------------------------------------------------
 * Logging and debugging setup
 */

extern FILE *log_file;
extern int log_level;
const int log_errors = 0x1;       /* Log internal errors */
const int log_msgs = 0x2;         /* Log messages */
const int log_config = 0x4;       /* Log config file processing */
const int log_statechange = 0x8;  /* Log state changes */
const int log_cmds = 0x10;        /* Log commands */
const int log_game_status = 0x20; /* Log game status changes */
const int log_libcalls = 0x40;    /* Log most library calls */
const int log_waddch = 0x80;      /* Log calls to waddch - very verbose */
const int log_mouse = 0x100;      /* Log mouse activity */

inline void log(int log_flag, const char *str, ...)
{

//	printf("Logging: %s", str);
    if ((log_level & log_flag) && log_file)
    {
        va_list ap;

        va_start(ap, str);
        vfprintf(log_file, str, ap);
        fflush(log_file);
        va_end(ap);
    }
}

int sage_error(const char *msg);
// Displays an error message, goes to passive mode, and returns 0


/*-----------------------------------------------------------------------------
 * Configuration options
 */

extern int adom_version;

struct Config
{
    // Options affecting program operation
    int passive;             // Run passively - just watch

    // Added features
    int auto_swap_neutral;   // Automatically swap places with neutral monsters
    int fast_selling;        // Remove unnecessary messages while selling
    int mindcraft_stats;     // Show mindcraft damage, etc.
    int more_weapon_stats;   // Augment weapon stats display
    int select_starsign;     // Select starsign or not
    int enable_autosave;     // Enable autosave or not
    int enable_reroller;     // Enable char reroller or not
    int short_alchemy;       // Shorten alchemy display
    int short_named_monsters;// Shorten named monsters' titles
    int spell_stats;         // Show spell range, duration, etc.
    int suppress_toef;       // Suppress redundant ToEF messages
    int auto_dump_flg;       // Answer "Y" to "Do you want to create a memorial..."

    // Bugfix features
    int fix_typos;           // Miscellaneous bugfixes, typo corrections, ...
    int fix_flgs;            // Fix flgs under ADOM 1.1.0

    // I/O options
    int cursor_visibility;   // Cursor visibility
    int cursor_attr;         // Attribute to use in place of normal cursor
    int draw_blocks;         // Use linedrawing characters for blocks
    int draw_lines;          // Use linedrawing characters for lines
    int draw_dots;           // Use linedrawing characters for dots
    int mouse;               // Enable mouse support

    // In-game options
    int fast_more;           // Accept any character for (more) prompt

    // Sage options
    int quiet_macros;        // Disable screen updates while running macros

    // Macros
    char *macro[numMacros];
};

typedef void (*MsgHandler)(StateCmdProcessor *state, void *data, char *str,
                           const char *format, va_list ap);

struct MsgInfo
{
    // Constructors: just copy a bunch of variables
    MsgInfo(MsgHandler new_handler, void *new_handler_data)
        : handler(new_handler), handler_data(new_handler_data),
          no_skip(0), attr(A_NORMAL), color(COLOR_WHITE), subst(NULL) {};
    MsgInfo(int new_no_skip, attr_t new_attr, short new_color, const char *new_subst)
        : handler(NULL), handler_data(NULL), no_skip(new_no_skip),
          attr(new_attr), color(new_color), subst(new_subst) {};
    MsgInfo(const char *new_subst)
        : handler(NULL), handler_data(NULL), no_skip(0), attr(A_NORMAL),
          color(COLOR_WHITE), subst(new_subst) {};

    // Member variables
    MsgHandler handler;
    void *handler_data;
    int no_skip;
    attr_t attr;
    short color;
    const char *subst;
};

extern char *cwd_path;
extern string *config_path;

typedef STRING_HASH(MsgInfo *) MsgMap;
typedef STRING_HASH(regex_t *) RegexMap;
extern MsgMap *main_msgmap;
extern MsgMap *regex_msgmap;
extern RegexMap *regex_map;

extern Config *config;

void init_msg_maps(void);
int init_config(void);
int read_config(void);
int read_msg_maps(void);


/*-----------------------------------------------------------------------------
 * Commands
 */

typedef map<const char *, int, ltstr> KeyMap;

extern KeyMap *main_keymap;   // Key map for normal operation
extern KeyMap *look_keymap;   // For 'l'ook command
extern KeyMap *target_keymap; // For targeting, ordering pets, etc.
extern KeyMap *locate_keymap; // For 'n'ame command, teleport control
extern int **reverse_keymap; // Maps commands to key sequences

/* Command-processing functions */
int read_keymaps(void);
const char *ch_to_cmdstring(int ch);
int *cmdstring_to_ch(const char *cmdstring);
Command lookup_command(string &cmd, KeyMap *keymap);

/* Key queue */
class KeyQueue
{
    public:
        KeyQueue();
        bool empty() const
        {
            return v.empty();
        };
        size_t size() const
        {
            return v.size();
        };
        void clear()
        {
            v.clear();
        };
        int front()
        {
            return v[i];
        };
        void pop_front();
        void push_back(int ch)
        {
            v.push_back(ch);
        };
        void push_cmd(Command cmd);
        void push_string(const char *str);
        void push_queue(KeyQueue &q);
    protected:
        vector<int> v;
        size_t i;
};

extern KeyQueue *key_queue, *macro_queue, *current_input, *current_keys,
       *prev_keys;
// key_queue is global input queue (input may come from mouse, features, etc.)
// macro_queue is input queue just for macros (since parts of macro may need
//   to append stuff to key_queue)
// current_input is keys that make up current command, to feed to ADOM after
//   we're done with them
// current_keys is all of the keys that make up the current command; used for
//   "repeat last command"
// prev_keys is all of the keys that made up the previous command; used for
//   "repeat last command"


/*---------------------------------------------------------------------------
 * Game status
 */

struct GameStatus
{
    char player_name[20];
    Location loc;
    int player_attr[numAttrs];
    int player_level;
};

extern GameStatus *game_status;
typedef STRING_HASH(int) AttrMap;
extern AttrMap *attr_map;

void init_game_status(void);


/*---------------------------------------------------------------------------
 * Spells
 */

void init_spells(void);


/*---------------------------------------------------------------------------
 * Graphics and I/O
 */

extern int colorpair[8];
extern int highlight_colorpair[8];

extern bool no_wrefresh;

void init_io(void);
int morewait (WINDOW *win, int skip_more);
void show_msg (WINDOW *win, const char *msg);
int get_key (WINDOW *win, int curs_visibility = 1, attr_t curs_attr = A_NORMAL,
             bool allow_mouse = false);
int get_string (WINDOW *win, char *str, int n);
chtype convert_char(chtype ch);
chtype reverse_convert_char(chtype ch);
int get_color (char *colorname, attr_t *attr, short *color);
void pad_string (char *str, int pad_length);
void fill_char (WINDOW *win, chtype ch, int y, int x, int len);

bool find_ch(WINDOW *win, chtype ch, int &y, int &x);
void move_cursor(WINDOW *win, int y, int x);

typedef mmask_t MouseCookie;
MouseCookie mouse_enable();
void mouse_restore(MouseCookie cookie);
void get_mouse_event(int *y, int *x, int *button);


/*---------------------------------------------------------------------------
 * Global and static variables in ADOM's address space, and a few variables
 * that we need ourselves
 */

extern char ***adom_log_text;
extern int *adom_log_pos;

extern char **local_log_text;
extern int local_log_pos;


/*---------------------------------------------------------------------------
 * Online help and pager
 */
/*
   void init_help();
   void show_manual(int section);
   void pager(const char *title, int linecount, const char * const line[],
   int linelen[] = NULL, int start_line = 0);
   */
