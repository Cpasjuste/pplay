#ifndef PPLAY_USBFS_H
#define PPLAY_USBFS_H

#ifdef __SWITCH__

#include <cstdlib>
#include <threads.h>
#include <usbhsfs.h>

int usbThread(void *arg);
void usbInit();

#endif
#endif