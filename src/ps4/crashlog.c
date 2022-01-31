#ifdef __PS4__
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <orbis/libkernel.h>

#define MAX_MESSAGE_SIZE    0x1000
#define MAX_STACK_FRAMES    32

/**
 * A callframe captures the stack and program pointer of a
 * frame in the call path.
 **/
typedef struct  {
  void *sp;
  void *pc;
} callframe_t;


/**
 * Log a backtrace to /dev/klog
 **/
static void
backtrace(const char* reason) {
  char addr2line[MAX_STACK_FRAMES * 20];
  callframe_t frames[MAX_STACK_FRAMES];
  OrbisKernelVirtualQueryInfo info;
  char buf[MAX_MESSAGE_SIZE + 3];
  unsigned int nb_frames = 0;
  char temp[80];

  memset(addr2line, 0, sizeof addr2line);
  memset(frames, 0, sizeof frames);
  memset(buf, 0, sizeof buf);

  snprintf(buf, sizeof buf, "<118>[Crashlog]: %s\n", reason);
  
  strncat(buf, "<118>[Crashlog]: Backtrace:\n", MAX_MESSAGE_SIZE);
  sceKernelBacktraceSelf(frames, sizeof frames, &nb_frames, 0);
  for(unsigned int i=0; i<nb_frames; i++) {
    memset(&info, 0, sizeof info);
    sceKernelVirtualQuery(frames[i].pc, 0, &info, sizeof info);

    snprintf(temp, sizeof temp,
	     "<118>[Crashlog]:   #%02d %32s: 0x%lx\n",
	     i + 1, info.name, frames[i].pc - info.unk01 - 1);
    strncat(buf, temp, MAX_MESSAGE_SIZE);
    
    snprintf(temp, sizeof temp,
	     "0x%lx ", frames[i].pc - info.unk01 - 1);
    strncat(addr2line, temp, sizeof addr2line - 1);
  }

  strncat(buf, "<118>[Crashlog]: addr2line: ", MAX_MESSAGE_SIZE);
  strncat(buf, addr2line, MAX_MESSAGE_SIZE);
  strncat(buf, "\n", MAX_MESSAGE_SIZE);

  buf[MAX_MESSAGE_SIZE+1] = '\n';
  buf[MAX_MESSAGE_SIZE+2] = '\0';
  
  sceKernelDebugOutText(0, buf);
}

/**
 * Log fatal signals to kernel log.
 **/
static void
fatal_signal(int sig) {
  char reason[64];

  sprintf(reason, "Received the fatal POSIX signal %d", sig);
  backtrace(reason);
  _exit(1);
}


/**
 * Register handlers for fatal signals.
 **/
static void __attribute__((constructor))
init_signal(void) {
  signal(5, fatal_signal);  // SIGTRAP
  signal(6, fatal_signal);  // SIGABRT
  signal(8, fatal_signal);  // SIGFPE
  signal(10, fatal_signal); // SIGBUS
  signal(11, fatal_signal); // SIGSEGV
}
#endif