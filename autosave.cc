#include "jaakkos.h"

/* For now, autosave only works with 1.1.1. #define out addresses later. */

int (*adom_msg)(const char *format, ...) = (int(*)(const char*,...))0x8049fc0;

#define TURNCOUNTER (*((unsigned int*)0x082b16e0))
#define YESNO_CODE 0x0809072A
unsigned char ignore_yesno[] = {0x33, 0xc0, 0x40, 0x90,
                                0x90, 0x90, 0x90, 0x90,
                                0x90, 0x90, 0x90, 0x90,
                                0x90, 0x90, 0x90};

static int autosave(int patch_really_save) {
  // child
  if(!try_fork()) {
    if(mprotect(PAGEBOUND(0x0809072A), getpagesize(), RWX_PROT) ||
       mprotect(PAGEBOUND(0x0804A690), getpagesize(), RWX_PROT) ||
       mprotect(PAGEBOUND(0x08125C20), getpagesize(), RWX_PROT)) {
      perror("mprotect()");
      exit(1);
    }
    
    close(STDOUT_FILENO);
    close(STDIN_FILENO);
    
    if(patch_really_save)
      memcpy((void*)0x0809072A, ignore_yesno, sizeof(ignore_yesno)); // no "really save"
      
    *((int*)0x082b665c) = 60; // command = save
    *((unsigned char*)0x0804A690) = 0xC3; // no hit spacebar to continue
    *((unsigned int*)0x08125c20) = 0x00c3c031; // don't remove temp files
    
    return 0;
  }

  // parent
  else {
    wait(NULL);
    
    char* (*get_svg_name)() = (char*(*)())0x080D6050;
    char svg_path[2048];
    char new_path[2048];
    
    snprintf(svg_path, 2048, "%s/.adom.data/savedg/%s", getpwuid(getuid())->pw_dir, get_svg_name());
    // TODO: change 111 when support for other versions is added
    snprintf(new_path, 2048, "%s/backup-111/%s", getpwuid(getuid())->pw_dir, get_svg_name());    
    
    if(!rename(svg_path, new_path))
      adom_msg("NOTICE: backup .svg created!");
    
    else adom_msg("WARNING: Automatic backup failed to move the .svg (%s)",
	    sys_errlist[errno]);
	    
    return 1;
  }
}

void command_hook() {
  static unsigned int last_save_turn = 0;

  // time for autobackup?
  if(TURNCOUNTER >= last_save_turn + 1000)
    if(autosave(1))
      last_save_turn = TURNCOUNTER; // in parent
    
  ((void(*)())0x0804a500)();
}

int save_hook(char *msg, int a, char b) {  
  if(!autosave(0))
    return 1; // child - answer 'y'

  // parent - ask  
  return ((int(*)(char*,int,char))0x0804B800)(msg,a,b);
}

