#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/sched.h>
#include <semaphore.h>

pthread_mutex_t mutex;
sem_t barrier[10];

/*
 * SCHED_OTHER -> 0
 * SCHED_FIFO -> 1
 * SCHED_RR -> 2
 * */
#define POLICY_TO_INT(P) \
    !strcmp(P, "SCHED_FIFO") ? SCHED_FIFO : \
    !strcmp(P, "SCHED_RR") ? SCHED_RR : \
    !strcmp(P, "SCHED_OTHER") ? SCHED_OTHER : SCHED_OTHER;

int ARG_THREAD = 0;
int ARG_BUFFER_SIZE = 0;
int ARG_POLICY = SCHED_OTHER;
int ARG_PRIORITY = 0;

char *buffer = NULL;
int buffer_pointer = 0;

void *thread_runner(void *data) {
  intptr_t threadID = (intptr_t)data;

  printf("[THREAD %ld]::%c -> waiting for all to start\n", threadID, (char)(0x41 + threadID));
  sem_wait(&barrier[threadID]);

  while (buffer_pointer < ARG_BUFFER_SIZE) {
	pthread_mutex_lock(&mutex);
	buffer[buffer_pointer] = 0x41 + threadID;
	buffer_pointer++;
	pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void print_formatted_buffer(char *buffer) {
  int i, j = 0;
  char aux = 0;

  char *aux_buffer = (char *)malloc(sizeof(char) * ARG_BUFFER_SIZE);
  int *count_buffer = (int *)malloc(sizeof(int) * ARG_THREAD);

  memset(aux_buffer, 0, ARG_BUFFER_SIZE);
  memset(count_buffer, 0, ARG_THREAD);

  for (i = 0; i < ARG_BUFFER_SIZE; i++) {
	if (aux != buffer[i]) {
	  aux_buffer[j] = buffer[i];
	  j++;
	}
	aux = buffer[i];
  }

  for (i = 0; i < j; i++)
	printf("%c", aux_buffer[i]);

  for (i = 0; i < j; i++)
	count_buffer[aux_buffer[i] - 0x41]++;

  printf("\n");
  for (i = 0; i < ARG_THREAD; i++)
	printf("[THREAD %d] %c: %d\n", i, 0x41 + i, count_buffer[i]);
}

void print_sched(int threadID, int policy) {
  switch (policy) {
	case SCHED_FIFO: printf("[THREAD %d] policy: SCHED_FIFO\n", threadID);
	  break;
	case SCHED_RR: printf("[THREAD %d] policy: SCHED_RR\n", threadID);
	  break;
	case SCHED_OTHER: printf("[THREAD %d] policy: SCHED_OTHER\n", threadID);
	  break;
	default: printf("[THREAD %d] policy: unknown\n", threadID);
  }
}

int getPolicyToInt(char *policy) {
  switch (policy[6]) {
	case 'F': return 1;
	  break;
	case 'R': return 2;
	  break;
	case '0': return 0;
	  break;
	default: return 1;
  }
}

void setpriority(pthread_t *thr, int threadID, int newpolicy, int newpriority) {
  int policy, ret;
  struct sched_param param;
  pthread_t t;

  if (newpriority > sched_get_priority_max(newpolicy) ||
	  newpriority < sched_get_priority_min(newpolicy)) {
	printf("[THREAD %d] invalid priority: MIN: %d, MAX: %d\n",
		   threadID,
		   sched_get_priority_min(newpolicy),
		   sched_get_priority_max(newpolicy));
  }

  pthread_getschedparam(*thr, &policy, &param);
  param.sched_priority = newpriority;
  ret = pthread_setschedparam(*thr, newpolicy, &param);
  if (ret != 0) {
	perror("perror(): ");
  }
  pthread_getschedparam(*thr, &policy, &param);
  print_sched(threadID, policy);
}

int main(int argc, char **argv) {

  ARG_THREAD = atoi(argv[1]);
  ARG_BUFFER_SIZE = atoi(argv[2]);
  ARG_POLICY = POLICY_TO_INT(argv[3]);
  ARG_PRIORITY = atoi(argv[4]);

  if (argc < 4) {
	printf("usage: ./%s <número_de_threads> <tamanho_do_buffer_global_em_kilobytes> <policy> <prioridade>\n\n",
		   argv[0]);
	exit(0);
  }

  int i = 0;
  buffer = (char *)malloc(sizeof(char) * ARG_BUFFER_SIZE);
  pthread_t thr[ARG_THREAD];
  pthread_mutex_init(&mutex, NULL);
  memset(buffer, 0, ARG_BUFFER_SIZE);

  for (i = 0; i < ARG_THREAD; i++)
	sem_init(&barrier[i], 0, 0);

  for (i = 0; i < ARG_THREAD; i++) {
	pthread_create(&thr[i], NULL, thread_runner, (void *)(intptr_t)i);
	setpriority(&thr[i], i, ARG_POLICY, ARG_PRIORITY);
  }

  for (i = 0; i < ARG_THREAD; i++)
	sem_post(&barrier[i]);

  for (i = 0; i < ARG_THREAD; i++)
	pthread_join(thr[i], NULL);

  print_formatted_buffer(buffer);

  return 0;
}
