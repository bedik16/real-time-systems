#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timerfd.h>

struct periodic_info
{
  int timer_fd;
  unsigned long long wakeups_missed;
};

static int make_periodic(unsigned int period, struct periodic_info *info)
{
  int ret;
  unsigned int ns;
  unsigned int sec;
  int fd;
  struct itimerspec itval;

  /*Create the timer*/
  fd = timerfd_create(CLOCK_MONOTONIC, 0);
  info->wakeups_missed = 0;
  info->timer_fd = fd;
  if (fd == 1)
    return fd;

  /* Make the timer periodic*/
  sec = period / 1000000;
  ns = (period - (sec*1000000)) * 1000;
  itval.it_interval.tv_sec = sec;
  itval.it_interval.tv_nsec = ns;
  itval.it_value.tv_sec = sec;
  itval.it_value.tv_nsec = ns;
  ret = timerfd_settime(fd, 0, &itval, NULL);
  return ret;
}

static void wait_period(struct periodic_info *info)
{
  unsigned long long missed;
  int ret;

  ret = read(info->timer_fd, &missed, sizeof(missed));
  if (ret == -1)
    {
      perror("read timer");
      return;
    }

  info->wakeups_missed += missed;
}

static int thread_1_count;
static int thread_2_count;
static int thread_3_count;

void CPUburn()
{
  for(unsigned long i = 0; i<=1000000000;){i++;}
}

static void *thread_1(void *arg)
{
  struct periodic_info info;

  printf("Thread 1 period 20s\n");
  make_periodic(200000000, &info);
  while (1)
    {
      thread_1_count++;
      CPUburn();
      printf("\t x \n");
      CPUburn();
      printf("\t x \n");
      CPUburn();
      printf("\t x \n");
      wait_period(&info);
    }
  return NULL;
}

static void *thread_2(void *arg)
{
  struct periodic_info info;

  printf("Thread 2 period 5s\n");
  make_periodic(5000000, &info);
  while (1)
    {
      thread_2_count++;
      CPUburn();
      printf("\t\t x \n");
      CPUburn();
      printf("\t\t x \n");
      wait_period(&info);
    }
  return NULL;
}

static void *thread_3(void *arg)
{
  struct periodic_info info;
  printf("Thread 3 period 10s\n");
  make_periodic(10000000, &info);
  while (1)
    {
      thread_3_count++;
      CPUburn();
      printf("\t\t\t x \n");
      CPUburn();
      printf("\t\t\t x \n");
      CPUburn();
      printf("\t\t\t x \n");
      wait_period(&info);
    }
  return NULL;
}
int main(int argc, char *argv[])
{
  pthread_t t_1;
  pthread_t t_2;
  pthread_t t_3;

  printf("Periodic threads using timerfd\n");

  pthread_create(&t_1, NULL, thread_1, NULL);
  pthread_create(&t_2, NULL, thread_2, NULL);
  pthread_create(&t_3, NULL, thread_3, NULL);

  sleep(36);
  printf("thread1 %d iterations\n", thread_1_count);
  printf("thread2 %d iterations\n", thread_2_count);
  printf("thread3 %d iterations\n", thread_3_count);
  return 0;
}
