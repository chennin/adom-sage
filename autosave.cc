#include "jaakkos.h"

unsigned char ignore_yesno111[] = {0x33, 0xc0, 0x40, 0x90,
                                   0x90, 0x90, 0x90, 0x90,
                                   0x90, 0x90, 0x90, 0x90,
                                   0x90, 0x90, 0x90};
unsigned char ignore_yesno12018[] = {0x33, 0xC0, 0x40, 0x90,
                                     0x90, 0x90, 0x90, 0x90,
                                     0x90, 0x90, 0x90, 0x90,
                                     0x90, 0x90, 0x90, 0x90,
                                     0x90, 0x90, 0x90, 0x90};
unsigned char press_q12018[] = {0xC7, 0x05, 0x70, 0x9C,
                                0x2A, 0x08, 0x02, 0x00,
                                0x00, 0x00, 0xC3};

static int autosave(int patch_really_save) {

  // child
  if(!try_fork()) {
    uint32_t REALLY_SAVE_ADDR = 0, SAVE_ADDR = 0, SPACEBAR_ADDR = 0, TEMP_FILES_ADDR = 0, QUIT_ADDR = 0;
    int adom_version = get_version();
    if (adom_version == 111) {
      REALLY_SAVE_ADDR = 0x0809072A;
      SAVE_ADDR = 0x082b665c;
      SPACEBAR_ADDR = 0x0804A690;
      TEMP_FILES_ADDR = 0x08125c20;
    }
    else if (adom_version == 12018) {
      REALLY_SAVE_ADDR = 0x0808AE98;
      SAVE_ADDR = 0x082E20CC;
      SPACEBAR_ADDR = 0x0804B320;
      TEMP_FILES_ADDR = 0x0812C400;
      QUIT_ADDR = 0x0807D790;
    }
    else {
      printf("Don't know where to inject autosaver. Unknown ADOM version %i ?\n", adom_version);
      return 2;
    }

    if(mprotect(PAGEBOUND(REALLY_SAVE_ADDR), getpagesize(), RWX_PROT) ||
       mprotect(PAGEBOUND(SPACEBAR_ADDR), getpagesize(), RWX_PROT) ||
       mprotect(PAGEBOUND(TEMP_FILES_ADDR), getpagesize(), RWX_PROT) ) {
      perror("mprotect()");
      exit(1);
    }

    close(STDOUT_FILENO);
    close(STDIN_FILENO);

    if(patch_really_save) {
      if (adom_version == 111) {
        memcpy((void*)REALLY_SAVE_ADDR, ignore_yesno111, sizeof(ignore_yesno111)); // no "really save"
      }
      else if (adom_version == 12018) {
        memcpy((void*)REALLY_SAVE_ADDR, ignore_yesno12018, sizeof(ignore_yesno12018));
      }
    }

    *((int*)SAVE_ADDR) = 60; // command = save
    *((unsigned char*)SPACEBAR_ADDR) = 0xC3; // no hit spacebar to continue
    *((unsigned int*)TEMP_FILES_ADDR) = 0x00c3c031; // don't remove temp files

    if (adom_version == 12018) {
      if(mprotect(PAGEBOUND(QUIT_ADDR), getpagesize(), RWX_PROT)) {
        perror("mprotect()");
        exit(1);
      }
      memcpy((void*)QUIT_ADDR, press_q12018, sizeof(press_q12018)); //pre18 quits to main menu after save
    }

    return 0;
  }

  // parent
  else {
    uint32_t SVG_ADDR = 0, MSG_ADDR = 0;
    int adom_version = get_version();
    if (adom_version == 111) {
      SVG_ADDR = 0x080D6050;
      MSG_ADDR = 0x8049fc0;
    }
    else if (adom_version == 12018) {
      SVG_ADDR = 0x080CDBC0;
      MSG_ADDR = 0x0804B8A0;
    }
    else {
      printf("Don't know where to inject autosaver. Unknown ADOM version %i ?\n", adom_version);
      return 2;
    }
    int (*adom_msg)(const char *format, ...) = (int(*)(const char*,...))MSG_ADDR;

    wait(NULL);

    char* (*get_svg_name)() = (char*(*)())SVG_ADDR;
    char svg_path[2048];
    char bk_dir[1024];
    char new_path[2048];

    snprintf(svg_path, 2048, "%s/.adom.data/savedg/%s", getpwuid(getuid())->pw_dir, get_svg_name());
    snprintf(bk_dir, 1024, "%s/.adom.data/backup/", getpwuid(getuid())->pw_dir);
    if (mkdir(bk_dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IXOTH|S_IROTH) != 0) {
      if (errno != EEXIST) { perror("mkdir() error"); }
    }
    snprintf(new_path, 2048, "%s/%s", bk_dir, get_svg_name());

    if(!rename(svg_path, new_path)) {
      adom_msg("NOTICE: backup .svg created!");
    }
    else {
      adom_msg("WARNING: Automatic backup failed to move the .svg (%s)", sys_errlist[errno]);
    }

    return 1;
  }
}

void command_hook() {
  static unsigned int last_save_turn = 0;
  uint32_t REFRESH_ADDR = 0, TURN_ADDR = 0;
  int adom_version = get_version();

  if (adom_version == 111) {
    REFRESH_ADDR = 0x0804a500;
    TURN_ADDR = 0x082b16e0;
  }
  else if (adom_version == 12018) {
    REFRESH_ADDR = 0x0804B440;
    TURN_ADDR = 0x82dba18;
  }
  else {
    printf("Don't know where to inject autosaver. Unknown ADOM version %i ?\n", adom_version);
    return;
  }

  // time for autobackup?
  if((*((unsigned int*)TURN_ADDR)) >= last_save_turn + 1000)
    if(autosave(1))
      last_save_turn = (*((unsigned int*)TURN_ADDR)); // in parent

  ((void(*)())REFRESH_ADDR)();
}

int save_hook(char *msg, int a, char b) {
  uint32_t PASK_ADDR = 0;
  int adom_version = get_version();

  if (adom_version == 111) {
    PASK_ADDR = 0x0804B800;
  }
  else if (adom_version == 12018) {
    PASK_ADDR = 0x0804C020;
  }
  else {
    printf("Don't know where to inject autosaver. Unknown ADOM version %i ?\n", adom_version);
    return 2;
  }

  if(!autosave(0))
    return 1; // child - answer 'y'

  // parent - ask
  return ((int(*)(char*,int,char))PASK_ADDR)(msg,a,b);
}

