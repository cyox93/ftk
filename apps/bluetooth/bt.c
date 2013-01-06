#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>

#include "bt.h"

#define _BT_DEV			"/dev/ttySP1"
#define _BT_BAUD		9600

#define BT_CMD_INIT		"ATZ\r"
#define BT_CMD_MASTER		"AT+BTROLE=M\r"
#define BT_CMD_SCAN		"AT+BTINQ?\r"
#define BT_CMD_CANCEL		"AT+BTCANCEL\r"
#define BT_CMD_CONNECT		"ATD%s\r"

typedef enum _BtState {
	BT_STATE_IDLE,
	BT_STATE_SCAN,
	BT_STATE_CONNECT,
} BtState;

typedef struct _Bt {
	int fd;
	BtState state;
} Bt;

static Bt _bt;

static char *_bt_read_line(char *str, char *buf, int max)
{
	char *c;
	int str_len = 0;

	c = str;
	while (*c && (str_len < max)) {
		if (*c == '\n') {
			c++;
			break;
		} else if (*c != '\r') {
			buf[str_len] = *c;
			str_len++;
		}

		c++;
	}

	buf[str_len] = '\0';

	return c;
}

static int _bt_parse_ok(char *buf)
{
	char *ok;

	if (!buf) return -1;

	ok = strstr(buf, "OK");

	return ok ? 0 : -1;
}

static int _bt_parse_connect(char *buf, char *mac)
{
	char *sep, len;

	if (!buf || !mac) return -1;

	sep = strchr(buf, ',');
	if (sep) {
		*sep = '\0';
		sep++;
	}

	len = strlen("CONNECT ");
	if (strncmp(buf, "CONNECT ", len) == 0) {
		strcpy(mac, &buf[len]); 

		return 0;
	}

	return -1;
}

static int _bt_parse_scan(char *buf, BtDevice *device)
{
	char *sep;

	if (!buf || !device) return -1;

	sep = strchr(buf, ',');
	if (sep) {
		*sep = '\0';
		sep++;
	}

	strcpy(device->mac, buf);

	buf = sep;
	if (!*buf) return 0;

	sep = strchr(buf, ',');
	if (sep) *sep = '\0';

	strcpy(device->name, buf);

	return 0;
}

static int _bt_send_cmd(Bt *bt, char *cmd,
		char *result, int max, int timeout)
{
	int ret;
	char *b;
	fd_set rdset;
	struct timeval tm;

	if (cmd) {
		printf("[BT] Send CMD [%s]\n", cmd);
		ret = uart_send(bt->fd, cmd, strlen(cmd));
		if (ret <= 0) return -1;
	}

	memset(result, 0, max);

	b = result;
	FD_ZERO(&rdset);
	FD_SET(bt->fd, &rdset);

	tm.tv_sec = timeout;
	tm.tv_usec = 0;

	while (1) {
		ret = select(bt->fd + 1, &rdset, NULL, NULL, &tm);
		if (ret > 0) {
			if (FD_ISSET(bt->fd, &rdset)) {
				ret = uart_recv(bt->fd, b, max);
				b += ret;
				max -= ret;

				if (_bt_parse_ok(result) == 0) {
					*b = '\0';
					break;
				}
			} else {
				continue;
			}
		} else if (ret == 0) {
			break;
		}
	}

	ret = (int)(b - result);

	if (ret > 0)
		printf("[BT] Result CMD-------\n%s\n-------------------\n", result);
	else
		printf("[BT] Result nothing\n");

	//uart_flush(bt->fd);

	return ret;
}

BtHd bt_open(void)
{
	char temp[1024];

	if (_bt.fd > 0) uart_close(_bt.fd);

	_bt.fd = uart_open(_BT_DEV, _BT_BAUD);
	if (_bt.fd < 0) return NULL;

	_bt_send_cmd(&_bt, BT_CMD_INIT, temp, sizeof(temp), 5);
	if (_bt_parse_ok(temp) < 0) return NULL;

	_bt_send_cmd(&_bt, BT_CMD_MASTER, temp, sizeof(temp), 10);
	if (_bt_parse_ok(temp) < 0) return NULL;

	_bt_send_cmd(&_bt, BT_CMD_INIT, temp, sizeof(temp), 5);
	if (_bt_parse_ok(temp) < 0) return NULL;

	_bt.state = BT_STATE_IDLE;

	return (BtHd)&_bt;
_err_open:
	if (_bt.fd > 0) close(_bt.fd);
	memset(&_bt, 0, sizeof(_bt));

	return NULL;
}

int bt_scan(BtHd hd, BtDevice *devices, int max)
{
	Bt *bt = (Bt *)hd;
	int ret, count = 0, len;
	char temp[2048], line[200], *b;

	if (!bt) return -1;

	if (bt->state != BT_STATE_IDLE) {
		_bt_send_cmd(bt, BT_CMD_CANCEL, temp, sizeof(temp), 5);
		if (_bt_parse_ok(temp) < 0) return -1;
	}

	_bt_send_cmd(bt, BT_CMD_SCAN, temp, sizeof(temp), 5);
	if (_bt_parse_ok(temp) < 0) return -1;

	bt->state = BT_STATE_SCAN;

	_bt_send_cmd(bt, NULL, temp, sizeof(temp), 25);
	b = temp; len = strlen(temp);
	while (*b && (b < (temp + len))) {
		memset(line, 0, sizeof(line));
		b = _bt_read_line(b, line, sizeof(line));
		if (!line[0]) continue;
		if (_bt_parse_ok(line) == 0) break;

		ret = _bt_parse_scan(line, &devices[count]);
		if (ret == 0) count++;
	}

	bt->state = BT_STATE_IDLE;

	return count;
}

int bt_connect(BtHd hd, BtDevice *device)
{
	Bt *bt = (Bt *)hd;
	int ret, len;
	char temp[1024], mac[20], line[200], *b;

	if (!bt) return -1;

	if (!device || !device->mac[0]) return -1;

	if (bt->state != BT_STATE_IDLE) {
		_bt_send_cmd(bt, BT_CMD_CANCEL, temp, sizeof(temp), 5);
		if (_bt_parse_ok(temp) < 0) return -1;
	}

	sprintf(temp, BT_CMD_CONNECT, device->mac);
	_bt_send_cmd(bt, temp, temp, sizeof(temp), 5);
	if (_bt_parse_ok(temp) < 0) return -1;

	bt->state = BT_STATE_CONNECT;

	_bt_send_cmd(bt, NULL, temp, sizeof(temp), 10);
	b = temp; len = strlen(temp);
	while (*b && (b < (temp + len))) {
		memset(line, 0, sizeof(line));
		b = _bt_read_line(b, line, sizeof(line));
		if (!line[0]) continue;
		if (_bt_parse_ok(line) == 0) break;

		ret = _bt_parse_connect(line, mac);
		if (ret == 0) break;
	}

	bt->state = BT_STATE_IDLE;

	return ret;
}

int bt_close(BtHd hd)
{
	Bt *bt = (Bt *)hd;

	if (bt) {
		if (bt->fd) {
			uart_flush(bt->fd);
			uart_close(bt->fd);
		}

		memset(bt, 0, sizeof(Bt));
	}

	return 0;
}


//#define _BT_TEST

#ifdef _BT_TEST
int main(void)
{
	BtHd bt;
	int ret, i;

	BtDevice devices[BT_SCAN_MAX];

	memset(devices, 0, sizeof(devices));

	printf("BT test\n");

	bt = bt_open();
	if (!bt) {
		printf("failed to open bt\n");
		return -1;
	}

	ret = bt_scan(bt, devices, BT_SCAN_MAX);
	if (ret < 0) {
		printf("failed to request scan [%d]\n", ret);
		bt_close(bt);
		return -1;
	}

	printf("scan result [%d]\n", ret);
	for (i = 0; i < ret; i++) {
		printf("[%d] : mac[%s], name[%s]\n", i, devices[i].mac, devices[i].name);
	}

	ret = bt_connect(bt, &devices[0]); 
	if (ret < 0) {
		printf("failed to connect [%s][%d]\n", devices[0].name, ret);
	}

	bt_close(bt);

	return 0;
}
#endif

