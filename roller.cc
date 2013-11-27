#include "jaakkos.h"

extern char item_list[ITEMS][ITEMSTRSZ];

unsigned char patch_3_instructions[] = {0xB0, 0x52, 0x90, 0x90, 0x90};
unsigned char talent_instructions[] = {0x56, 0x90, 0x90};

#define MAX_ROLLS 1000
#define BEST_ROLLS 16

struct RollResult {
  unsigned char rng[0x100];
  int stats[0x9];
  int items[0x2b9+0x2f];
  unsigned int talents;
};

struct RollerShared {
  int stats[0x9];
  int items[0x2b9+0x2f];
  unsigned int talents;
};

int stat_requirements[0x9] = {10, 10, 10, 10, 10, 10, 10, 10, 10};
int item_requirements = 0x0;// 0x0 for nothing, 0x1...0x2b9 for items, 0x2ba...0x2e8 for spellbooks (0x16 for iron ration)

RollerShared *shm;

const char stat_names[][3] = {"St", "Le", "Wi", "Dx", "To", "Ch", "Ap", "Ma", "Pe"};

static void reseed_rng() {
  uint16_t *rng111 = (uint16_t *)0x082ada40;
  uint16_t *rng12018 = (uint16_t *)0x082CEBA0;
  int adom_version = get_version();
  if (adom_version == 111) {
    for (int i=0; i < (0x100/2); i++)
      rng111[i] = rand();
  }
  else if (adom_version == 12018) {
    for (int i=0; i < (0x100/2); i++)
      rng12018[i] = rand();
  }
  else {
    printf("Don't know where to inject roller. Unknown ADOM version %i ?\n", adom_version);
    return;
  }
}

void load_requirements() {
  char roller_cfg[1024];
  snprintf(roller_cfg, 1024, "%s/roller.cfg", getpwuid(getuid())->pw_dir);

  FILE *fp = fopen(roller_cfg, "r");
  if (fp) {
    fread(stat_requirements, sizeof(int), 0x9, fp);
    fclose(fp);
  }
}

static void save_requirements() {
  char roller_cfg[1024];
  snprintf(roller_cfg, 1024, "%s/roller.cfg", getpwuid(getuid())->pw_dir);

  FILE *fp = fopen(roller_cfg, "w");
  if (fp) {
    fwrite(stat_requirements, 4, 0x9, fp);
    fclose(fp);
  }
}

static void draw_ui(int stat_sel, unsigned char failed,
                    unsigned int max_rolls, RollResult *best_rolls) {
  if (failed == 2) {
    printf("The initial roll finished. Ignore the roller by hitting 'a'.\r\n");
    printf("\r\n");
  }

  else if (failed) {
    printf("The roll finished. The character you asked for WAS NOT FOUND.\r\n");
    printf("\r\n");
  }

  else {
    printf("The roll finished. The character you asked for WAS FOUND.\r\n");
    printf("\r\n");
  }

  printf("Your character has the following attributes.\r\n");

  for (int i=0; i < 0x9; i++)
    printf("%s - rolled %2d %s required %2d%s\r\n", stat_names[i],
      best_rolls[0].stats[i], (best_rolls[0].stats[i] >= stat_requirements[i]) ? "and" : "but", stat_requirements[i], (stat_sel == i) ? " (selected)" : "");

  printf("S - rolled %d total\r\n", +best_rolls[0].stats[0]
                                    +best_rolls[0].stats[1]
                                    +best_rolls[0].stats[2]
                                    +best_rolls[0].stats[3]
                                    +best_rolls[0].stats[4]
                                    +best_rolls[0].stats[5]
                                    +best_rolls[0].stats[6]
                                    +best_rolls[0].stats[7]
                                    +best_rolls[0].stats[8]);
  printf("I - item #%i from group #%i is %s\r\n", item_requirements%(0x2b9+1)+item_requirements/(0x2b9+1), item_requirements/(0x2b9+1), (item_requirements > 0) ? ((best_rolls[0].items[item_requirements-1]) ? "found" : "not found") : "not an item");
  printf("T - potentially %d talents to be learned\r\n", best_rolls[0].talents);
  printf("\r\n");
  printf("The roller will generate a maximum of %d characters\r\n", max_rolls);
    char *item_requirements_buffer;
    item_requirements_buffer = (char *)malloc(0x100*sizeof(char));
    sprintf(item_requirements_buffer, "require item #%i from group #%i (%.36s%s)", item_requirements%(0x2b9+1)+item_requirements/(0x2b9+1), item_requirements/(0x2b9+1), item_list[item_requirements-1], (strlen(item_list[item_requirements-1]) > 36) ? "..." : "");
  printf("and %s.\r\n", (item_requirements > 0) ? item_requirements_buffer : "not require any items");
  printf("\r\n");
  printf("a/r - accept/refuse the current character\r\n");
  printf("1-9 - select the attribute to modify\r\n");
  printf("p/n - decrease/increase the item index required by 1\r\n");
  printf("b/f - decrease/increase the item index required by 100\r\n");
  printf("-/+ - decrease/increase the selected attribute by 1\r\n");
  printf("</> - decrease/increase the maximum rolls by 100");
  fflush(stdout);
}

static int config_roller(RollResult *best_rolls, unsigned char failed,
                         unsigned int *max_rolls) {
  int input;
  int stat_sel = 0;

  while (1) {
    printf("\033[2J\033[0;0H");
    draw_ui(stat_sel, failed, *max_rolls, best_rolls);
    input = tolower(fgetc(stdin));

    if (isdigit(input)) {
      int no = input-'0';
      if (no < 1 || no > 0x9) continue;
      stat_sel = no-1;
    }

    else {
      int ret;

      switch(input) {
      case 'a':
	    ret = 1;
		break;

      case 'r':
	    ret = 0;
		break;

      case '+':
        if (stat_requirements[stat_sel] < 99)
          stat_requirements[stat_sel]++;
        continue;

      case '-':
        if (stat_requirements[stat_sel] > 0)
          stat_requirements[stat_sel]--;
        continue;

      case '>':
        if (*max_rolls < 100000)
          *max_rolls += 100;
        continue;

      case '<':
        if (*max_rolls > 100)
          *max_rolls -= 100;
        continue;

      case 'n':
        if (item_requirements < 0x2b9+0x2f)
          item_requirements += 1;
        continue;

      case 'p':
        if (item_requirements > 0)
          item_requirements -= 1;
        continue;

      case 'f':
        if (item_requirements < 0x2b9+0x2f-100)
          item_requirements += 100;
        continue;

      case 'b':
        if (item_requirements > 0+100)
          item_requirements -= 100;
        continue;

      default:
	    continue;
      }

      printf("\033[2J\033[0;0H");
      fflush(stdout);

      save_requirements();
      return ret;
    }
  }
}

#define P2(x) ((x)*(x))

static int sqerr(int *data) {
  int sqerr = 0;

  for (int i=0; i < 0x9; i++)
    if (stat_requirements[i] > data[i])
      sqerr += P2(stat_requirements[i]-data[i]);

  return sqerr;
}

// children end up here
static void roll_end(unsigned int talents18, void *b, void *c, unsigned int ntalents) {
  uint32_t STATS_ADDR = 0, ITEMS_ADDR = 0, BOOKS_ADDR = 0;
  int adom_version = get_version();

  if (adom_version == 111) {
    STATS_ADDR = 0x082b1728;
    ITEMS_ADDR = 0x082a5980;
    BOOKS_ADDR = 0x082a7e00;
  }
  else if (adom_version == 12018) {
    STATS_ADDR = 0x082DBA7C;
    ITEMS_ADDR = 0x082C6080;
    BOOKS_ADDR = 0x082C7B60;
  }
  else {
    printf("Don't know where to inject roller. Unknown ADOM version %i ?\n", adom_version);
    return;
  }

  memcpy(shm->stats, (void*)STATS_ADDR, 0x9*sizeof(int));
  memcpy(shm->items, (void*)ITEMS_ADDR, 0x2b9*sizeof(int));
  memcpy(shm->items+0x2b9, (void*)BOOKS_ADDR, 0x2f*sizeof(int));
  if (adom_version == 12018) {
    shm->talents = talents18;
  }
  else {
    shm->talents = ntalents;
  }
  exit(EXIT_SUCCESS);
}

static void child() {
  uint32_t PATCH1_ADDR = 0, PATCH2_ADDR = 0, PATCH3_ADDR = 0, CNT_ADDR = 0, TALENT_ADDR = 0, ROLLEND_ADDR = 0, RESUME_ADDR = 0;
  int adom_version = get_version();

  if (adom_version == 111) {
    PATCH1_ADDR = 0x080756e1;
    PATCH2_ADDR = 0x0807571b;
    PATCH3_ADDR = 0x08075728;
    CNT_ADDR = 0x0814a86e;
    ROLLEND_ADDR = 0x0814ec49;
    RESUME_ADDR = 0x08073970;
  }
  else if (adom_version == 12018) {
    PATCH1_ADDR = 0x0807DB25;
    PATCH2_ADDR = 0x0807DB5A;
    PATCH3_ADDR = 0x0807DB66;
    CNT_ADDR = 0x0807BB95;
    TALENT_ADDR = 0x0815CBE1;
    ROLLEND_ADDR = 0x0815CBE5;
    RESUME_ADDR = 0x0807C7B0;
  }
  else {
    printf("Don't know where to inject roller. Unknown ADOM version %i ?\n", adom_version);
    return;
  }

  close(STDOUT_FILENO);
  close(STDIN_FILENO);

  if (mprotect(PAGEBOUND(PATCH1_ADDR), getpagesize(), RWX_PROT) ||
      mprotect(PAGEBOUND(PATCH2_ADDR), getpagesize(), RWX_PROT) ||
      mprotect(PAGEBOUND(PATCH3_ADDR), getpagesize(), RWX_PROT) ||
      mprotect(PAGEBOUND(ROLLEND_ADDR), getpagesize(), RWX_PROT) ||
      mprotect(PAGEBOUND(RESUME_ADDR), getpagesize(), RWX_PROT) ||
      mprotect(PAGEBOUND(CNT_ADDR), getpagesize(), RWX_PROT)) {
    perror("mprotect");
    exit(1);
  }

  // don't stop waiting for useless input in roller
  memset((void*)PATCH1_ADDR, 0x90, 5);
  memset((void*)PATCH2_ADDR, 0x90, 5);
  memcpy((void*)PATCH3_ADDR, patch_3_instructions, sizeof(patch_3_instructions));
  // don't write .cnt
  if (adom_version == 111) {
    *((uint16_t *)CNT_ADDR) = 0x71eb;
  }
  else if (adom_version == 12018) {
    memset((void*)CNT_ADDR, 0x90, 5);
    if (mprotect(PAGEBOUND(TALENT_ADDR), getpagesize(), RWX_PROT)) {
      perror("mprotect");
      exit(1);
    }
    memcpy((void*)TALENT_ADDR, talent_instructions, sizeof(talent_instructions));
  }
  else {
    printf("Don't know where to inject roller. Unknown ADOM version %i ?\n", adom_version);
    return;
  }

  // patch roll end
  *((char**)ROLLEND_ADDR) = ((char*)(&roll_end)) - ROLLEND_ADDR - 4;

  // resume ADoM
  ((void(*)())RESUME_ADDR)();
  return;
}

static void display_roll_status(RollResult *best_rolls, int rolln, int max_rolls,
                                struct timeval begin) {
  struct timeval end;
  gettimeofday(&end, NULL);

  double tdiff = end.tv_sec-begin.tv_sec;
  tdiff += (end.tv_usec-begin.tv_usec) / 1000000.0;

  printf("\033[2J\033[0;0HRolled %d/%d characters in %.1f s at %.1f Hz as follows.\r\n\r\n",
  rolln, max_rolls, tdiff, ((double)rolln)/tdiff);

  printf("+----+----+----+----+----+----+----+----+----+------+---+---+------+\r\n");

  for (int stat=0; stat < 0x9; stat++)
    printf("| %2s ", stat_names[stat]);

  printf("| Sum  | I | T | Err  |\r\n");
  printf("+----+----+----+----+----+----+----+----+----+------+---+---+------+\r\n");

  for (int stat=0; stat < 0x9; stat++)
    printf("| %2d ", stat_requirements[stat]);

  printf("|      | %c |   |      |\r\n", (item_requirements > 0) ? 'X' : ' ');
  printf("+----+----+----+----+----+----+----+----+----+------+---+---+------+\r\n");

  for (int i=0; i < BEST_ROLLS; i++)
    if (best_rolls[i].stats[0] > 0) {
      for (int stat=0; stat < 0x9; stat++)
        printf("| %2d ", best_rolls[i].stats[stat]);

      printf("| %4d |", +best_rolls[i].stats[0]
                        +best_rolls[i].stats[1]
                        +best_rolls[i].stats[2]
                        +best_rolls[i].stats[3]
                        +best_rolls[i].stats[4]
                        +best_rolls[i].stats[5]
                        +best_rolls[i].stats[6]
                        +best_rolls[i].stats[7]
                        +best_rolls[i].stats[8]);
      printf(" %c |", (best_rolls[i].items[item_requirements-1]) ? 'X' : ' ');
      printf(" %1d |", best_rolls[i].talents);
      printf(" %4d |\r\n", sqerr(best_rolls[i].stats));
    }
  printf("+----+----+----+----+----+----+----+----+----+------+---+---+------+\r\n");

  fflush(stdout);
}

#define BETTER_THAN(i) ((!item_requirements || shm->items[item_requirements-1] == best_rolls[(i)].items[item_requirements-1]) ? sqerr(shm->stats) <= sqerr(best_rolls[(i)].stats) : shm->items[item_requirements-1])

static int process_roll_result(RollResult *best_rolls) {
  uint32_t RNG_ADDR = 0;
  int adom_version = get_version();
  if (adom_version == 111) {
    RNG_ADDR = 0x082ada40;
  }
  else if (adom_version == 12018) {
    RNG_ADDR = 0x082CEBA0;
  }
  else {
    printf("Don't know where to inject roller. Unknown ADOM version %i ?\n", adom_version);
    return 2;
  }

  if (BETTER_THAN(BEST_ROLLS-1)) {
    int pos = BEST_ROLLS-1;

    while (pos-1 >= 0 && BETTER_THAN(pos-1)) {
      memcpy(best_rolls+pos, best_rolls+pos-1, sizeof(RollResult));
      pos--;
    }

    memcpy(best_rolls[pos].rng, (void*)RNG_ADDR, 0x100);
    memcpy(best_rolls[pos].stats, shm->stats, 0x9*sizeof(int));
    memcpy(best_rolls[pos].items, shm->items, (0x2b9+0x2f)*sizeof(int));
    best_rolls[pos].talents = shm->talents;
  }

  if(!best_rolls[0].items[item_requirements-1]) return 0;
  for (int i=0; i < 0x9; i++)
    if (best_rolls[0].stats[i] < stat_requirements[i])
      return 0;

  return 1;
}

static void reset_best(RollResult *best_rolls) {
  for (int i=0; i < BEST_ROLLS; i++) {
    for (int j=0; j < 0x9; j++)
      best_rolls[i].stats[j] = -1;
    for (int j=0; j < 0x2b9+0x2f; j++)
      best_rolls[i].items[j] = 0;
    }
}

void roll_start() {
  uint32_t RESUME_ADDR, RNG_ADDR = 0;
  int adom_version = get_version();

  if (adom_version == 111) {
    RNG_ADDR = 0x082ada40;
    RESUME_ADDR = 0x08073970;
  }
  else if (adom_version == 12018) {
    RNG_ADDR = 0x082CEBA0;
    RESUME_ADDR = 0x0807C7B0;
  }
  else {
    printf("Don't know where to inject roller. Unknown ADOM version %i ?\n", adom_version);
    return;
  }

  unsigned int rolln = 0;
  unsigned int max_rolls = MAX_ROLLS;
  RollResult best_rolls[BEST_ROLLS];
  unsigned char roll_failed = 0;

  shm = (RollerShared*) shm_init( sizeof(RollerShared) );
  srand(time(NULL));

  // initial roll
  reset_best(best_rolls);
  if (!try_fork()) { child(); return; }

  if (wait(NULL) == -1) {
    perror("wait");
    exit(1);
  }

  process_roll_result(best_rolls);
  roll_failed = 2;

  // main roller
  while (!config_roller(best_rolls, roll_failed, &max_rolls)) {
    rolln = 0;
    reset_best(best_rolls);

    struct timeval begin;
    gettimeofday(&begin, NULL);

    for (/*ahh*/; rolln < max_rolls; rolln++) {
      reseed_rng();
      if (!(rolln % 100)) display_roll_status(best_rolls, rolln, max_rolls, begin);

      if (!try_fork()) { child(); return; }

      if (wait(NULL) == -1) {
        perror("wait");
        exit(1);
      }

      if (process_roll_result(best_rolls)) break;
    }

    roll_failed = (rolln == max_rolls);
    memcpy((void*)RNG_ADDR, best_rolls[0].rng, 0x100);
  }

  shm_deinit(shm);

  // resume ADoM
  printf("\033[2J\033[3;3H");
  fflush(stdout);
  ((void(*)())RESUME_ADDR)();
};
