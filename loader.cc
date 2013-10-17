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

int get_version() {
        // Get ADOM version number, set by Sage
        char *version = getenv("ADOM_VERSION");
        int adom_version = 0;

        if (version != NULL)
        {
                sscanf(version, "%i", &adom_version);
        }
	return adom_version;
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
	int adom_version = get_version();
        if(
                        /* For now, autosave only works with 1.1.1. #define out addresses later. */
                        mprotect(PAGEBOUND(0x0808f990), getpagesize(), RWX_PROT) || /* autosave command_hook */
                        mprotect(PAGEBOUND(0x08090735), getpagesize(), RWX_PROT) /* autosave save_hook */
          ) {
                perror("mprotect()");
                exit(1);
        }

        if (adom_version == 111) {
                // inject autosaver - every 1000 turn
                *((char**)0x0808f990) = ((char*)(&command_hook)) - 0x0808f994;

                // inject autosaver - on 'S' command
                *((char**)0x08090735) = ((char*)(&save_hook)) - 0x08090739;
        }
}

void inject_roller(void) {
	int adom_version = get_version();
	// try to load user's saved requirements
	load_requirements();

	// roller's item list
	load_item_list();

        if(
                        /* For now, roller only works with 1.1.1. #define out addresses later. */
			mprotect(PAGEBOUND(0x080756C4), getpagesize(), RWX_PROT) /* roller */
          ) {
                perror("mprotect()");
                exit(1);
        }

        if (adom_version == 111) {
		// inject stat roller
		*((char**)0x080756C4) = ((char*)(&roll_start)) - 0x080756C8;
        }

}
