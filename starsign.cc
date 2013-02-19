#include "jaakkos.h"

void starsign_select() {
	int month;
	uint32_t BIRTHSIGN_ADDR = 0;
	uint32_t JUMP_TO = 0;

	// Get ADOM version number, set by Sage
	char *version = getenv("ADOM_VERSION");
	int adom_version = 0;

	if (version != NULL)
	{
		sscanf(version, "%i", &adom_version);
	}

	/*
	BIRTHSIGN_ADDR is the place in memory where the day of birth is stored.
	Its value in memory will be the in-game displayed value minus one.

	JUMP_TO is where to resume execution in the ADOM executable.

         See http://www.adom.de/forums/showthread.php/1134-Choosing-star-sign?p=72882#post72882
         for information on finding these offsets.
	*/
	if (adom_version == 111) {
		BIRTHSIGN_ADDR = 0x82b61f0;
		JUMP_TO = 0x813ee80;
	}
	else if (adom_version == 100) {
		BIRTHSIGN_ADDR = 0x82a66e4;
		JUMP_TO = 0x0813ace0;
	}
	else if (adom_version == 1203) {
		BIRTHSIGN_ADDR = 0x8283d40;
		JUMP_TO = 0x081386d0;
	}
	else if (adom_version == 1204) {
		BIRTHSIGN_ADDR = 0x8285d40;
		JUMP_TO = 0x8139fb0;
	}
	else if (adom_version == 1205) {
		BIRTHSIGN_ADDR = 0x8286948;
		JUMP_TO = 0x813a860;
	}
	else if (adom_version == 1206) {
		BIRTHSIGN_ADDR = 0x828acbc;
		JUMP_TO = 0x813bf70;
	}
	else if (adom_version == 1207) {
		BIRTHSIGN_ADDR = 0x82921fc;
		JUMP_TO = 0x81405a0;
	}
	else if (adom_version == 1208) {
		BIRTHSIGN_ADDR = 0x8295a60;
		JUMP_TO = 0x81418f0;
	}
	else if (adom_version == 1209) {
		BIRTHSIGN_ADDR = 0x82a7d40;
		JUMP_TO = 0x81458a0;
	}
	else if (adom_version == 12010) {
		BIRTHSIGN_ADDR = 0x82a7e40;
		JUMP_TO = 0x8145780;
  }
	else if (adom_version == 12011) {
		BIRTHSIGN_ADDR = 0x82a6d20;
		JUMP_TO = 0x8144910;
	}
	if ((BIRTHSIGN_ADDR == 0) || (JUMP_TO == 0)) {
		printf("Don't know where to put the birth date or jump to.  Unknown ADOM version %i ?\n", adom_version);
		return;
	}

	do {
		printf("\033[2J\033[1;1H");

		printf("Select the month of your birth:\r\n"
				"\r\n"
				"  ? - random\r\n"
				"\r\n"
				"  A - Raven\r\n"
				"  B - Book\r\n"
				"  C - Wand\r\n"
				"  D - Unicorn\r\n"
				"  E - Salamander\r\n"
				"  F - Dragon\r\n"
				"  G - Sword\r\n"
				"  H - Falcon\r\n"
				"  I - Cup\r\n"
				"  J - Candle\r\n"
				"  K - Wolf\r\n"
				"  L - Tree\r\n"
				"\r\n"
				"> ");

		fflush(stdout);
		month = toupper(fgetc(stdin));
	} while(month != '?' && (month < 'A' || month > 'L'));

	printf("\033[2J");
	fflush(stdout);

	month = month == '?' ? 0 : month-'A'+1;

	if(!month) month = rand() % 12;
	else month--;

	*((uint16_t*)BIRTHSIGN_ADDR) = (unsigned int)(30*month + (rand()%30));
	((void(*)())JUMP_TO)();
}

