/*---------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley - see LICENSE for details
 *
 * Functions to shadow those in ncurses and libc
 */


/*---------------------------------------------------------------------------
 * System-independent globals and preprocessor directives
 */
#include "adom-sage.h"
#include "jaakkos.h"
#include "states.h"
#include "config.h"

int in_wgetnstr;           // If in wgetnstr, ignore other library calls
int initialized = 0;


/*---------------------------------------------------------------------------
 * Globals and statics from ADOM's address space
 */

char ***adom_log_text = (char ** *) 0x82adbd8;
int *adom_log_pos = (int *) 0x82995c4;

char **local_log_text;
int local_log_pos;


/*---------------------------------------------------------------------------
 * Linux-only globals and preprocessor directives
 */
#ifndef _WIN32

#include <dlfcn.h>

#define LOG_PATH "/tmp/sage.log"

// Functions shadowed by this library
INIT_PAIR real_init_pair;
WCLEAR    real_wclear;
WREFRESH  real_wrefresh;
WMOVE     real_wmove;
WADDCH    real_waddch;
WADDNSTR  real_waddnstr;
WINCH     real_winch;
WGETCH    real_wgetch;
WGETNSTR  real_wgetnstr;
SPRINTF   real_sprintf;
VSPRINTF  real_vsprintf;
VSNPRINTF real_vsnprintf;
UNLINK    real_unlink;

#define INTERCEPTOR(x) x

#endif


/*---------------------------------------------------------------------------
 * Win32-only globals and preprocessor directives
 */
#ifdef _WIN32

#include <windows.h>
#include "libraries.h"

#define LOG_PATH "sage.log"

#define INTERCEPTOR(x) sage_##x

#endif


/*---------------------------------------------------------------------------
 * Initialization functions
 */

#ifndef _WIN32
/* Linux-specific initialization; Windows-specific stuff is at end of file */
void load_dynamic_libraries()
{
    const char *libc_path, *curses_path;
    void *handle;

    // Locate libraries to use
    libc_path = getenv("LIBC_PATH");

    if (libc_path == NULL)
    {
        libc_path = LIBC;
    }

    curses_path = getenv("CURSES_PATH");

    if (curses_path == NULL)
    {
        curses_path = LIBNCURSES;
    }

    // Import functions from ncurses
    handle = (void *) dlopen(curses_path, 1);

    if (!handle)
    {
        printf("ADOM Sage: Unable to load\n");
        exit(128);
    }

    real_init_pair = (INIT_PAIR) dlsym(handle, "init_pair");
    real_wclear    =    (WCLEAR) dlsym(handle, "wclear");
    real_wrefresh  =  (WREFRESH) dlsym(handle, "wrefresh");
    real_wmove     =     (WMOVE) dlsym(handle, "wmove");
    real_waddch    =    (WADDCH) dlsym(handle, "waddch");
    real_waddnstr  =  (WADDNSTR) dlsym(handle, "waddnstr");
    real_winch     =     (WINCH) dlsym(handle, "winch");
    real_wgetch    =    (WGETCH) dlsym(handle, "wgetch");
    real_wgetnstr  =  (WGETNSTR) dlsym(handle, "wgetnstr");

    // Import functions from libc
    handle = (void *) dlopen(libc_path, 1);

    if (!handle)
    {
        printf("ADOM Sage: Unable to load\n");
        exit(128);
    }

    real_sprintf  = (SPRINTF)  dlsym(handle, "sprintf");
    real_vsprintf = (VSPRINTF) dlsym(handle, "vsprintf");
    real_vsnprintf = (VSNPRINTF) dlsym(handle, "vsnprintf");
    real_unlink   = (UNLINK)   dlsym(handle, "unlink");

    // Don't reload Sage for any of our children (if we ever get children)
    unsetenv("LD_PRELOAD");
}
#endif

void initialize()
{
    int result;

    if (initialized)
    {
        return;
    }

#ifndef _WIN32
    load_dynamic_libraries();
#endif

    // Initialize debugging
    char *debug_flags = getenv("SAGE_DEBUG");

    if (debug_flags)
    {
        sscanf(debug_flags, "%i", &log_level);
    }

    if (log_level)
    {
        const char *log_path = getenv("SAGE_LOGFILE");

        if (log_path == NULL)
        {
            log_path = LOG_PATH;
        }

        log_file = fopen(log_path, "w");

        if (!log_file)
        {
            printf("Unable to open log file; ADOM Sage will run without logging.\n\n");
            printf("[PRESS ENTER TO CONTINUE]");
            getchar();
        }
    }

    // Read configuration files; perform other initialization
    result = 1;
    init_io();
    init_spells();
    init_msg_maps();
    init_game_status();
    result = result && init_config();
    result = result && read_keymaps();
    result = result && read_config();
    result = result && read_msg_maps();
    if (config->select_starsign) {
      inject_my_starsign();
    }
    if (config->enable_autosave) {
      inject_autosaver();
    }
    if (config->enable_reroller) {
      inject_roller();
    }
    //init_help();
    // read_keymaps checks that ADOM is configured; we want to ensure that
    // before checking on our own configuration

    // Initialize state variables
    push_state(new StatePassive());

    if (!config->passive)
    {
        push_state(new StateStartup());
    }

    initialized = 1;
}


/*---------------------------------------------------------------------------
 * Library functions
 */

int INTERCEPTOR(init_pair) (short pair, short f, short b)
{
    log(log_libcalls, "init_pair %i %i %i\n",
        (int) pair, (int) f, (int) b);

    if (b == COLOR_BLACK)
    {
        colorpair[f] = pair;
    }

    else if (b == COLOR_WHITE)
    {
        highlight_colorpair[f] = pair;
    }

    return real_init_pair(pair, f, b);
}

int INTERCEPTOR(wclear) (WINDOW *win)
{
    int result = 0;

    log(log_libcalls, "wclear %x\n", (ULONG_PTR)(intptr_t) win);

    result = cur_state()->wclear(win);
    cleanup_states();
    return result;
}

int INTERCEPTOR(wrefresh) (WINDOW *win)
{
    log(log_libcalls, "wrefresh %x\n", (ULONG_PTR)(intptr_t) win);

    if (!no_wrefresh)
    {
        return real_wrefresh(win);
    }

    else
    {
        return OK;
    }
}

int INTERCEPTOR(wmove) (WINDOW *win, int y, int x)
{
    int result;

    if (in_wgetnstr)
    {
        return real_wmove(win, y, x);
    }

    log(log_libcalls, "wmove %x %i %i\n", (ULONG_PTR)(intptr_t) win, y, x);

    result = cur_state()->wmove(win, y, x);
    cleanup_states();
    return result;
}

int INTERCEPTOR(waddch) (WINDOW *win, chtype ch)
{
    int result;

    if (in_wgetnstr)
    {
        return real_waddch(win, ch);
    }

    log(log_libcalls, "waddch %x %i %c\n",
        (ULONG_PTR)(intptr_t) win, (int) ch, (char) ch);

    result = cur_state()->waddch(win, ch);
    cleanup_states();
    return result;
}

int INTERCEPTOR(waddnstr) (WINDOW *win, const char *str, int n)
{
    // Assumption: ADOM always uses -1 for n and NULL-terminated strings
    int result;

    if (in_wgetnstr)
    {
        return real_waddnstr(win, str, n);
    }

    log(log_libcalls, "waddnstr %x %i %s\n", (ULONG_PTR)(intptr_t) win, n, str);

    result = cur_state()->waddnstr(win, str, n);
    cleanup_states();
    return result;
}

chtype INTERCEPTOR(winch) (WINDOW *win)
{
    chtype result;
    result = cur_state()->winch(win);
    cleanup_states();
    log(log_libcalls, "winch %i %c %i %i %i\n", (int) result, (char) result,
        (int) result & A_CHARTEXT, (int) result & A_ATTRIBUTES,
        (int) result & A_COLOR);
    return result;
}

int INTERCEPTOR(wgetch) (WINDOW *win)
{
    int result;

    if (in_wgetnstr)
    {
        return real_wgetch(win);
    }

    if (current_input->empty())
    {
        result = cur_state()->wgetch(win);
    }

    else
    {
        result = current_input->front();
        current_input->pop_front();
    }

    log(log_libcalls, "wgetch %x %i %s\n",
        (ULONG_PTR)(intptr_t) win, result, keyname(result));

    cleanup_states();
    return result;
}

int INTERCEPTOR(wgetnstr) (WINDOW *win, char *str, int n)
{
    // Assumption: ADOM leaves enough space for str to be NULL-terminated
    int result;

    in_wgetnstr = 1;
    result = cur_state()->wgetnstr(win, str, n);
    in_wgetnstr = 0;

    log(log_libcalls, "wgetnstr %x %i %s\n", (ULONG_PTR)(intptr_t) win, n, str);

    cleanup_states();
    return result;
}

int INTERCEPTOR(sprintf) (char *str, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    int result = cur_state()->sprintf(str, format, ap);
    va_end(ap);

    log(log_libcalls, "sprintf %s %s\n", str, format);

    return result;
}

// pre-1.2.0p4:
int INTERCEPTOR(vsprintf) (char *str, const char *format, va_list ap)
{
    initialize();
    int result = cur_state()->vsprintf(str, format, ap);

    log(log_libcalls, "vsprintf %s %s\n", str, format);

    return result;
}

// 1.2.0p4+
int INTERCEPTOR(vsnprintf) (char *str, size_t size, const char *format, va_list ap)
{
    initialize();
    int result = cur_state()->vsnprintf(str, size, format, ap);
	
    log(log_libcalls, "vsnprintf %s %s\n", str, format);

    return result;
} 

int INTERCEPTOR(unlink) (const char *pathname)
{
    log(log_libcalls, "unlink %s\n", pathname);

    return cur_state()->unlink(pathname);
}


/*---------------------------------------------------------------------------
 * Win32-specific initialization functions
 */
#ifdef _WIN32

#ifdef _DEBUG

void INTERCEPTOR(signal) (void)
{
    // Disables signal handling so MSVC++'s debugger will be called
}

#endif

void sageSymbols()
{
    codesym("init_pair", sage_init_pair);
    codesym("wclear", sage_wclear);
    codesym("wrefresh", sage_wrefresh);
    codesym("wmove", sage_wmove);
    codesym("waddch", sage_waddch);
    codesym("waddnstr", sage_waddnstr);
    codesym("winch", sage_winch);
    codesym("wgetch", sage_wgetch);
    codesym("wgetnstr", sage_wgetnstr);
    codesym("sprintf", sage_sprintf);
    codesym("vsprintf", sage_vsprintf);
    codesym("vsnprintf", sage_vsnprintf);
    codesym("unlink", sage_unlink);
#ifdef _DEBUG
    codesym("__sysv_signal", sage_signal);
#endif
}

void sageInit()
{
    SetConsoleTitle("ADOM");
    // Could call initialize function here, if we cared to.
    // But let's keep system-dependent stuff to a minimum for now.
}

#endif

