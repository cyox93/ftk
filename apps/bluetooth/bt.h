#ifndef __BT_H__
#define __BT_H__

#define BT_SCAN_MAX		9

typedef struct _BtDevice {
	char name[100];
	char mac[20];
} BtDevice;

typedef void * BtHd;

BtHd bt_open(void);
int bt_scan(BtHd hd, BtDevice *devices, int max);
int bt_connect(BtHd hd, BtDevice *device);
int bt_close(BtHd hd);

#endif // __BT_H__
