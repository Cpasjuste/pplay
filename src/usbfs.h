#ifndef PPLAY_USBFS_H
#define PPLAY_USBFS_H

#include <stdlib.h>
#include <threads.h>
#include <usbhsfs.h>

int usbThread(void *arg);
void usbInit();

#endif