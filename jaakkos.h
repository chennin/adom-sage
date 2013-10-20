#ifndef JAAKKOS_H
#define JAAKKOS_H

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <sys/time.h>

#define RWX_PROT (PROT_WRITE | PROT_READ | PROT_EXEC)

// Align address to page boundary
#define PAGEBOUND(p) ((void*)((p) - (p)%getpagesize()))

int get_version();
void inject_my_starsign();
void inject_autosaver();
void inject_roller();
void roll_start();
void load_requirements();

void starsign_select();
void command_hook();
int save_hook(char *msg, int a, char b);

void* shm_init(size_t bytes);
void shm_deinit(void*);

pid_t try_fork();
void load_item_list();
#define ITEMSTRSZ 128
#define ITEMS 744

#endif

