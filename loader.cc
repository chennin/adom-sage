#include "jaakkos.h"

// Called after Sage's config is read
void inject_my_starsign(void) {
	uint32_t INJECT_STARSIGN = 0, STAROFF = 0;
//	int INJECT_STAT = 0, STATOFF = 0;

	// Get ADOM version number, set by Sage
	char *version = getenv("ADOM_VERSION");
	int adom_version = 0;

	if (version != NULL)
	{
		sscanf(version, "%i", &adom_version);
	}

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
	if (INJECT_STARSIGN == 0) {
		printf("Don't know where to inject a function to.  Unknown ADOM version %i ?\n", adom_version);
		return;
	}

	STAROFF = INJECT_STARSIGN + 4;
//	STATOFF = INJECT_STAT + 4;
	
	// try to load user's saved requirements
	//  load_requirements();

	// roller's item list
	//  load_item_list();
	if(
			//     mprotect(PAGEBOUND(0x080756C4), getpagesize(), RWX_PROT) ||
			mprotect(PAGEBOUND(INJECT_STARSIGN), getpagesize(), RWX_PROT) // ||
			//     mprotect(PAGEBOUND(0x0808f990), getpagesize(), RWX_PROT) ||
			//     mprotect(PAGEBOUND(0x08090735), getpagesize(), RWX_PROT)
	  ) {
		perror("mprotect()");
		exit(1);
	}

	// inject stat roller
	//  *((void**)0x080756C4) = &roll_start - 0x080756C8;

	// inject starsign selector
        *((char**)INJECT_STARSIGN) = ((char*)(&starsign_select)) - STAROFF;

	// inject autosaver - every 1000 turn
	//  *((void**)0x0808f990) = &command_hook - 0x0808f994;

	// inject autosaver - on 'S' command
	//  *((void**)0x08090735) = &save_hook - 0x08090739;
}

