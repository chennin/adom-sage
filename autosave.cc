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
    uint32_t REALLY_SAVE_ADDR = 0, SAVE_ADDR = 0, SPACEBAR_ADDR = 0, TEMP_FILES_ADDR = 0, QUIT_ADDR = 0, RETURN_ADDR = 0;
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
      RETURN_ADDR = 0x709c2a08;
    }
    else if (adom_version == 12020) {
      REALLY_SAVE_ADDR = 0x0808af18;
      SAVE_ADDR = 0x082e2bf0;
      SPACEBAR_ADDR = 0x0804b320;
      TEMP_FILES_ADDR = 0x0812c720;
      QUIT_ADDR = 0x0807d810;
      RETURN_ADDR = 0x90a72a08;
    }
    else if (adom_version == 12021) {
      REALLY_SAVE_ADDR = 0x0808e9a5;
      SAVE_ADDR = 0x08309668;
      SPACEBAR_ADDR = 0x0804b650;
      TEMP_FILES_ADDR = 0x08136350;
      QUIT_ADDR = 0x0807fe90;
      RETURN_ADDR = 0x70ed2c08;
    }
    else if (adom_version == 12022) {
      REALLY_SAVE_ADDR = 0x0808f20b;
      SAVE_ADDR = 0x0830b690;
      SPACEBAR_ADDR = 0x0804b650;
      TEMP_FILES_ADDR = 0x081378e0;
      QUIT_ADDR = 0x08080320;
      RETURN_ADDR = 0x900d2d08;
    }
    else if (adom_version == 12023) {
      REALLY_SAVE_ADDR = 0x0808f4db;
      SAVE_ADDR = 0x0830c6f0;
      SPACEBAR_ADDR = 0x0804b650;
      TEMP_FILES_ADDR = 0x08138030;
      QUIT_ADDR = 0x080804c0;
      RETURN_ADDR = 0xd01d2d08;
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
      else if ((adom_version == 12018) || (adom_version >= 12020)) {
        memcpy((void*)REALLY_SAVE_ADDR, ignore_yesno12018, sizeof(ignore_yesno12018));
      }
    }

    *((int*)SAVE_ADDR) = 60; // command = save
    *((unsigned char*)SPACEBAR_ADDR) = 0xC3; // no hit spacebar to continue
    *((unsigned int*)TEMP_FILES_ADDR) = 0x00c3c031; // don't remove temp files

    if ((adom_version == 12018) || (adom_version >= 12020)) {
      if(mprotect(PAGEBOUND(QUIT_ADDR), getpagesize(), RWX_PROT)) {
        perror("mprotect()");
        exit(1);
      }
      memcpy((void*)QUIT_ADDR, press_q12018, sizeof(press_q12018)); //pre18 quits to main menu after save
      *((unsigned int*)(QUIT_ADDR + 2)) = RETURN_ADDR;
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
    else if (adom_version == 12020) {
      SVG_ADDR = 0x080cdd30;
      MSG_ADDR = 0x0804b8a0;
    }
    else if (adom_version == 12021) {
      SVG_ADDR = 0x080d4890;
      MSG_ADDR = 0x0804c640;
    }
    else if (adom_version == 12022) {
      SVG_ADDR = 0x080d5560;
      MSG_ADDR = 0x0804c640;
    }
    else if (adom_version == 12023) {
      SVG_ADDR = 0x080d5a20;
      MSG_ADDR = 0x0804c720;
    }
    else {
      printf("Don't know where to inject autosaver. Unknown ADOM version %i ?\n", adom_version);
      return 2;
    }
    int (*adom_msg)(const char *format, ...) = (int(*)(const char*,...))MSG_ADDR;

    wait(NULL);

    char* (*get_svg_name)() = (char*(*)())SVG_ADDR;
    char svg_path[2048];
    char new_path[2048];

    snprintf(svg_path, 2048, "%s/.adom.data/savedg/%s", getpwuid(getuid())->pw_dir, get_svg_name());
    snprintf(new_path, 2048, "%s/.adom.data/backup/%s", getpwuid(getuid())->pw_dir, get_svg_name());

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
    TURN_ADDR = 0x082DBA18;
  }
  else if (adom_version == 12020) {
    REFRESH_ADDR = 0x0804b440;
    TURN_ADDR = 0x082dc538;
  }
  else if (adom_version == 12021) {
    REFRESH_ADDR = 0x0804b800;
    TURN_ADDR = 0x08302c38;
  }
  else if (adom_version == 12022) {
    REFRESH_ADDR = 0x0804b800;
    TURN_ADDR = 0x08304c58;
  }
  else if (adom_version == 12023) {
    REFRESH_ADDR = 0x0804b800;
    TURN_ADDR = 0x08305cb8;
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
  else if (adom_version == 12020) {
    PASK_ADDR = 0x0804c020;
  }
  else if (adom_version == 12021) {
    PASK_ADDR = 0x0804c970;
  }
  else if (adom_version == 12022) {
    PASK_ADDR = 0x0804c970;
  }
  else if (adom_version == 12023) {
    PASK_ADDR = 0x0804ca50;
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

