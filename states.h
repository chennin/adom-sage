/*---------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * State classes
 */

class State
{
    public:
        virtual ~State() {};
        virtual const char *name(void) = 0;
        virtual int wclear(WINDOW *win)
        {
            return real_wclear(win);
        };
        virtual int wmove(WINDOW *win, int y, int x)
        {
            return real_wmove(win, y, x);
        };
        virtual int waddch(WINDOW *win, chtype ch)
        {
            return real_waddch(win, ch);
        };
        virtual int waddnstr(WINDOW *win, const char *str, int n)
        {
            return real_waddnstr(win, str, n);
        };
        virtual chtype winch(WINDOW *win)
        {
            return real_winch(win);
        }
        virtual int wgetch(WINDOW *win)
        {
            return get_key(win);
        };
        virtual int wgetnstr(WINDOW *win, char *str, int n)
        {
            return get_string(win, str, n);
        };
        virtual int sprintf(char *str, const char *format, va_list ap)
        {
            return real_vsprintf(str, format, ap);
        };
        virtual int vsprintf(char *str, const char *format, va_list ap)
        {
            return real_vsprintf(str, format, ap);
        };
        virtual int vsnprintf(char *str, size_t size, const char *format, va_list ap)
        {
	    int i = real_vsnprintf(str, size, format, ap);
            return i;
        };
        virtual int unlink(const char *pathname)
        {
            return real_unlink(pathname);
        }
};

class StatePassive : public State
{
    public:
        const char *name(void)
        {
            return "passive";
        };
};

class StateDrawable : public State
{
    public:
        int waddch(WINDOW *win, chtype ch)
        {
            return real_waddch(win, convert_char(ch));
        };
        int waddnstr(WINDOW *win, const char *str, int n);
        chtype winch(WINDOW *win)
        {
            return reverse_convert_char(real_winch(win));
        };
};

class StateCmdProcessor : public State
{
    public:
        StateCmdProcessor(MsgMap *msgmap_to_use,
                          MsgMap *regex_msgmap_to_use,
                          KeyMap *keymap_to_use);
        ~StateCmdProcessor();

        int wclear(WINDOW *win);
        int wmove(WINDOW *win, int y, int x);
        int waddch(WINDOW *win, chtype ch);
        int waddnstr(WINDOW *win, const char *str, int n);
        chtype winch(WINDOW *win);
        int wgetch(WINDOW *win);
        int vsprintf(char *str, const char *format, va_list ap);
	int vsnprintf(char *str, size_t size, const char *format, va_list ap);

        void enable_mouse(Command btn1_cmd = cmdInvalid,
                          Command btn2_cmd = cmdInvalid,
                          Command btn3_cmd = cmdInvalid,
                          Command btn4_cmd = cmdInvalid);

        // Public variables for access by message handlers
        int skip_next_msg;
        int ignore_more;
        State *next_state;
        Command last_cmd;

    protected:
        // State tracking
        int just_cleared;

        // Mouse handling
        bool mouse;
        Command btn1, btn2, btn3, btn4;
        MouseCookie old_mouse_status;
        int mouse_x, mouse_y;

        // Message handling
        int at_more, skip_more, reset_colors;
        short msg_color;
        attr_t msg_attr;
        MsgMap *msgmap;
        MsgMap *re_msgmap;
        

        // Command processing
        string cmdstring;
        KeyMap *keymap;
        bool allow_repeat;
        virtual void handle_cmd(WINDOW *win, Command cmd) = 0;
};

class StateDecorator : public State
{
    public:
        StateDecorator(StateCmdProcessor *new_component)
            : component(new_component) {};
        int wclear(WINDOW *win)
        {
            return component->wclear(win);
        };
        int wmove(WINDOW *win, int y, int x)
        {
            return component->wmove(win, y, x);
        };
        int waddch(WINDOW *win, chtype ch)
        {
            return component->waddch(win, ch);
        }
        int waddnstr(WINDOW *win, const char *str, int n)
        {
            return component->waddnstr(win, str, n);
        };
        chtype winch(WINDOW *win)
        {
            return component->winch(win);
        }
        int wgetch(WINDOW *win)
        {
            return component->wgetch(win);
        }
        int wgetnstr(WINDOW *win, char *str, int n)
        {
            return component->wgetnstr(win, str, n);
        };
        int vsprintf(char *str, const char *format, va_list ap)
        {
            return component->vsprintf(str, format, ap);
        };
        int vsnprintf(char *str, size_t size, const char *format, va_list ap)
        {
            return component->vsnprintf(str, size, format, ap);
        };
    protected:
        StateCmdProcessor *component;
};

class StateStartup : public StateDrawable
{
    public:
        const char *name(void)
        {
            return "startup";
        };
        int waddch(WINDOW *win, chtype ch);
        int waddnstr(WINDOW *win, const char *str, int n);
};

class StateRunning : public StateCmdProcessor
{
    public:
        StateRunning();
        const char *name(void)
        {
            return "running";
        };
    protected:
        virtual void handle_cmd(WINDOW *win, Command cmd);
};

class StateExamine : public StateCmdProcessor
{
    public:
        StateExamine();
        const char *name(void)
        {
            return "examine";
        };
    protected:
        virtual void handle_cmd(WINDOW *win, Command cmd);
};

class StateLocate : public StateCmdProcessor
{
    public:
        StateLocate();
        const char *name(void)
        {
            return "locate";
        };
    protected:
        virtual void handle_cmd(WINDOW *win, Command cmd);
};

class StateTarget : public StateCmdProcessor
{
    public:
        StateTarget();
        const char *name(void)
        {
            return "target";
        };
    protected:
        virtual void handle_cmd(WINDOW *win, Command cmd);
};

class StateFullScreen : public StateDrawable
{
    public:
        StateFullScreen();
        const char *name(void)
        {
            return "full screen";
        };
        int wclear(WINDOW *win);
        int vsprintf(char *str, const char *format, va_list ap);
	int vsnprintf(char *str, size_t size, const char *format, va_list ap);
	void process(const char *format);
    private:
        bool need_title;
};

class StateInfoScreen : public StateDrawable
{
    public:
        const char *name(void)
        {
            return "info screen";
        };
        int wgetch(WINDOW *win);
};

class StateLevelUp : public StateDrawable
{
    public:
        StateLevelUp();
        const char *name(void)
        {
            return "level up";
        };
        int wclear(WINDOW *win);
        int vsprintf(char *str, const char *format, va_list ap);
	int vsnprintf(char *str, size_t size, const char *format, va_list ap);
	void process(const char *format);
    private:
        bool done;
};

class StateWeaponStats : public State
{
    public:
        const char *name(void)
        {
            return "weapon stats";
        };
        StateWeaponStats();
        int wmove(WINDOW *win, int y, int x);
        int sprintf(char *str, const char *format, va_list ap);
        int vsprintf(char *str, const char *format, va_list ap);
	int vsnprintf(char *str, size_t size, const char *format, va_list ap);
	int process(char *str, const char *format, va_list ap);
    private:
        int dice, sides, plus, fraction;
};

class StateExtDrop : public State
{
    public:
        const char *name(void)
        {
            return "extended drop";
        };
        int wclear(WINDOW *win);
};

class StateRecall : public StateDrawable
{
    public:
        const char *name(void)
        {
            return "recall";
        };
        int wgetch(WINDOW *win);
};

class StateSwap : public StateDecorator
{
    public:
        StateSwap(StateCmdProcessor *cmd_processor, Command new_dir);
        const char *name(void)
        {
            return "swap";
        };
        int wgetch(WINDOW *win);
        int vsprintf(char *str, const char *fmt, va_list ap);
	int vsnprintf(char *str, size_t size, const char *fmt, va_list ap);
	int process(char *str, const char *fmt);
    private:
        Command dir;
        unsigned queue_size;
};

class StateAlchemy : public State
{
    public:
        StateAlchemy();
        const char *name(void)
        {
            return "alchemy";
        };
        int wclear(WINDOW *win);
        int wmove(WINDOW *win, int y, int x);
        int waddch(WINDOW *win, chtype ch);
        int vsprintf(char *str, const char *format, va_list ap);
	int vsnprintf(char *str, size_t size, const char *format, va_list ap);
    private:
        int just_cleared;
};

class StateDynamicDisplay : public StateDecorator
{
    public:
        StateDynamicDisplay(StateCmdProcessor *cmd_processor, Command
                            cmd_new_display);
        const char *name(void)
        {
            return "dynamic display";
        };
        int wgetch(WINDOW *win);
        int vsprintf(char *str, const char *format, va_list ap);
	int vsnprintf(char *str, size_t size, const char *format, va_list ap);
    private:
        size_t queue_size;
        Command cmd_display;
};

class StateCastSpell : public State
{
    public:
        StateCastSpell();
        const char *name(void)
        {
            return "cast spell";
        };
        int waddch(WINDOW *win, chtype ch);
        int waddnstr(WINDOW *win, const char *str, int n);
        int wgetch(WINDOW *win);
        int vsprintf(char *str, const char *format, va_list ap);
	int vsnprintf(char *str, size_t size, const char *format, va_list ap);
    private:
        char max_letter;
        Spell current_spell;
        void print_spell_details(WINDOW *win, const char *label,
                                 const char *format, ...);
};

class StateMindcraft : public StateDrawable
{
    public:
        const char *name(void)
        {
            return "mindcraft";
        };
        int wclear(WINDOW *win);
        int vsprintf(char *str, const char *format, va_list ap);
	int vsnprintf(char *str, size_t size, const char *format, va_list ap); 
//	int process(char *str, const char *format, va_list ap); 
};

class StateGameSummary : public StateDrawable
{
    public:
        StateGameSummary();
        const char *name(void)
        {
            return "game summary";
        };
        int wclear(WINDOW *win);
    private:
        int clear_count;
};

class StateMemorial : public StateDecorator
{
    public:
        StateMemorial(StateCmdProcessor *cmd_processor);
        const char *name(void)
        {
            return "memorial";
        };
        int wgetch(WINDOW *win);
        int unlink(const char *pathname);
    private:
        bool do_log;
};

void push_state(State *state);
void pop_state(void);
State *cur_state(void);
void cleanup_states(void);

