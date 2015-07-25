#include "jaakkos.h"

// fork or die
pid_t try_fork() {
  pid_t forkpid = fork();

  if(forkpid == -1) {
    perror("fork()");
    exit(1);
  }

  return forkpid;
}

// Called after Sage's config is read
void inject_my_starsign(void) {
	uint32_t INJECT_STARSIGN = 0, STAROFF = 0;
	int adom_version = get_version();

	/*
	INJECT_* is the address in memory to overwrite with the location of our function.
	 See http://www.adom.de/forums/showthread.php/1134-Choosing-star-sign?p=72882#post72882
	 for information on finding these offsets.
	*/
	if (adom_version == 111) {
		INJECT_STARSIGN = 0x813ee0a;
	}
	else if (adom_version == 100) {
		INJECT_STARSIGN = 0x813ac6a;
	}
	else if (adom_version == 1203) {
                INJECT_STARSIGN =  0x081387ba;
	}
	else if (adom_version == 1204) {
                INJECT_STARSIGN =  0x0813a09a;
	}
	else if (adom_version == 1205) {
                INJECT_STARSIGN =  0x0813a94a;
        }
	else if (adom_version == 1206) {
                INJECT_STARSIGN =  0x0813c05a;
        }
	else if (adom_version == 1207) {
                INJECT_STARSIGN =  0x814068a;
        }
	else if (adom_version == 1208) {
                INJECT_STARSIGN =  0x81419da;
        }
	else if (adom_version == 1209) {
                INJECT_STARSIGN =  0x814598a;
        }
	else if (adom_version == 12010) {
                INJECT_STARSIGN = 0x814586a;
        }
	else if (adom_version == 12011) {
                INJECT_STARSIGN = 0x81449fa;
        }
	else if (adom_version == 12012) {
                INJECT_STARSIGN = 0x814470a;
        }
	else if (adom_version == 12013) {
                INJECT_STARSIGN = 0x814477a;
        }
	else if (adom_version == 12014) {
                INJECT_STARSIGN = 0x814595a;
        }
	else if (adom_version == 12016) {
                INJECT_STARSIGN = 0x814917a;
        }
	else if (adom_version == 12017) {
                INJECT_STARSIGN = 0x814a8aa;
        }
	else if (adom_version == 12018) {
                INJECT_STARSIGN = 0x814a12a;
        }
        else if (adom_version == 12020) {
                INJECT_STARSIGN = 0x0814a49a;
        }
        else if (adom_version == 12021) {
                INJECT_STARSIGN = 0x8156cda;
        }
        else if (adom_version == 12022) {
                INJECT_STARSIGN = 0x81584fa;
        }
        else if (adom_version == 12023) {
                INJECT_STARSIGN = 0x08158d3a;
        }
        else if (adom_version == 12048) {
                INJECT_STARSIGN = 0x08165aca;
        }
        else if (adom_version == 12049) {
                INJECT_STARSIGN = 0x0816653a;
        }
        else if (adom_version == 12051) {
                INJECT_STARSIGN = 0x08171b2c;
        }
        else if (adom_version == 12055) {
                INJECT_STARSIGN = 0x081723cc;
        }
	else if (adom_version == 12059) {
		INJECT_STARSIGN = 0x0817844c;
	}
	if (INJECT_STARSIGN == 0) {
		printf("Don't know where to inject a function to.  Unknown ADOM version %i ?\n", adom_version);
		return;
	}

	STAROFF = INJECT_STARSIGN + 4;

	if(
			mprotect(PAGEBOUND(INJECT_STARSIGN), getpagesize(), RWX_PROT) /* starsign */
	  ) {
		perror("mprotect()");
		exit(1);
	}

	// inject starsign selector
        *((char**)INJECT_STARSIGN) = ((char*)(&starsign_select)) - STAROFF;
}

void inject_autosaver(void) {
    uint32_t COMMAND_ADDR = 0, SAVE_ADDR = 0;
	int adom_version = get_version();

    if (adom_version == 111) {
		COMMAND_ADDR = 0x0808F990;
		SAVE_ADDR = 0x08090735;
	}
	else if (adom_version == 12018) {
		COMMAND_ADDR = 0x0808A6FE;
		SAVE_ADDR = 0x0808AEA8;
	}
        else if (adom_version == 12020) {
                COMMAND_ADDR = 0x0808a77e;
                SAVE_ADDR = 0x0808af28;
        }
        else if (adom_version == 12021) {
                COMMAND_ADDR = 0x0808e1c0;
                SAVE_ADDR = 0x0808e9b5;
        }
        else if (adom_version == 12022) {
                COMMAND_ADDR = 0x0808ea28;
                SAVE_ADDR = 0x0808f21b;
        }
        else if (adom_version == 12023) {
                COMMAND_ADDR = 0x0808ecf8;
                SAVE_ADDR = 0x0808f4eb;
        }
        else if (adom_version == 12048) {
                COMMAND_ADDR = 0x080931a0;
                SAVE_ADDR = 0x08093be4;
        }
        else if (adom_version == 12049) {
                COMMAND_ADDR = 0x0809331d;
                SAVE_ADDR = 0x08093d64;
        }
        else if (adom_version == 12051) {
                COMMAND_ADDR = 0x0809B70E;
                SAVE_ADDR = 0x0809CD90;
        }
	else {
		printf("Don't know where to inject autosaver. Unknown ADOM version %i ?\n", adom_version);
		return;
	}

    if(
        mprotect(PAGEBOUND(COMMAND_ADDR), getpagesize(), RWX_PROT) || /* autosave command_hook */
        mprotect(PAGEBOUND(SAVE_ADDR), getpagesize(), RWX_PROT) /* autosave save_hook */
    ) {
        perror("mprotect()");
        exit(1);
    }

    // inject autosaver - every 1000 turn
    *((char**)COMMAND_ADDR) = ((char*)(&command_hook)) - COMMAND_ADDR - 4;

    // inject autosaver - on 'S' command
    *((char**)SAVE_ADDR) = ((char*)(&save_hook)) - SAVE_ADDR - 4;

}

void inject_roller(void) {
    uint32_t ROLL_ADDR = 0;
	int adom_version = get_version();

    if (adom_version == 111) {
		ROLL_ADDR = 0x080756C4;
	}
	else if (adom_version == 12018) {
		ROLL_ADDR = 0x0807DB03;
	}
        else if (adom_version == 12020) {
                ROLL_ADDR = 0x0807db83;
        }
        else if (adom_version == 12021) {
                ROLL_ADDR = 0x080800e5;
        }
        else if (adom_version == 12022) {
                ROLL_ADDR = 0x0808052d;
        }
        else if (adom_version == 12023) {
                ROLL_ADDR = 0x080806cd;
        }
        else if (adom_version == 12048) {
                ROLL_ADDR = 0x08082ced;
        }
        else if (adom_version == 12049) {
                ROLL_ADDR = 0x08082e07;
        }
        else if (adom_version == 12051) {
                ROLL_ADDR = 0x0808ab1b;
        }
        else if (adom_version == 12055) {
                ROLL_ADDR = 0x0808afcb;
        }
	else if (adom_version == 12059) {
		ROLL_ADDR = 0x0808d7aa;
	}
	else {
		printf("Don't know where to inject roller. Unknown ADOM version %i ?\n", adom_version);
		return;
	}

	// try to load user's saved requirements
	load_requirements();

	// roller's item list
	load_item_list();

    if(
        mprotect(PAGEBOUND(ROLL_ADDR), getpagesize(), RWX_PROT) /* roller */
    ) {
        perror("mprotect()");
        exit(1);
    }

    // inject stat roller
    *((char**)ROLL_ADDR) = ((char*)(&roll_start)) - ROLL_ADDR - 4;

}
