#include "usbfs.h"

static UEvent *g_statusChangeEvent, g_exitEvent;
static UsbHsFsDevice *g_usbDevices;
static u32 g_usbDeviceCount;

int usbThread(void *arg)
{
    (void)arg;
    Result rc = 0;
    int idx = 0;
    Waiter status_change_event_waiter = waiterForUEvent(g_statusChangeEvent);
    Waiter exit_event_waiter = waiterForUEvent(&g_exitEvent);
    
    while(true)
    {
        rc = waitMulti(&idx, -1, status_change_event_waiter, exit_event_waiter);
        if (R_FAILED(rc)) continue;
        if (g_usbDevices)
        {
            free(g_usbDevices);
            g_usbDevices = NULL;
        }

        if (idx == 1) break;

        g_usbDeviceCount = usbHsFsGetMountedDeviceCount();
        g_usbDevices = (UsbHsFsDevice*)calloc(g_usbDeviceCount, sizeof(UsbHsFsDevice));
    }
    return 0;
}

void usbInit() {
    thrd_t g_thread = {0};
    usbHsFsSetFileSystemMountFlags(UsbHsFsMountFlags_ShowHiddenFiles | UsbHsFsMountFlags_ReadOnly);
    usbHsFsInitialize(0);
    g_statusChangeEvent = usbHsFsGetStatusChangeUserEvent();
    ueventCreate(&g_exitEvent, true);
    thrd_create(&g_thread, usbThread, NULL);
    sleep(2);
}

