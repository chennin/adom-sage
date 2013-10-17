#include "jaakkos.h"

void* shm_init(size_t bytes) {
  key_t key;
  int shmid;

  char statnam[1024];
  snprintf(statnam, 1024, "/proc/%d/stat", getpid());

  /* make the key: */
  if ((key = ftok(statnam, 'R')) == -1) {
    perror("ftok");
    exit(1);
  }
  
  /* connect to (and possibly create) the segment: */
  if ((shmid = shmget(key, bytes, 0644 | IPC_CREAT)) == -1) {
    perror("shmget");
    exit(1);
  }
  
  /* attach to the segment to get a pointer to it: */
  void *shm = shmat(shmid, (void *)0, 0);
  if (shm == (void *)(-1)) {
    perror("shmat");
    exit(1);
  }
  
  return shm;
}

void shm_deinit(void *shm) {
  /* detach from the segment: */
  if (shmdt(shm) == -1) {
    perror("shmdt");
    exit(1);
  }
}

