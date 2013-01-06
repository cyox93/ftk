#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>

static int _get_baudrate(int baud)
{
	if (baud < 19200)
		return B9600;
	else if (baud < 38400)
		return B19200;
	else if (baud < 57600)
		return B38400;
	else if (baud < 115200)
		return B57600;
	else if (baud < 230400)
		return B115200;
	else
		return B230400;

	return -1;
}

int uart_open(const char *dev, int baudrate)
{

	int fd, c, res;
	struct termios newtio;

	fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK); 
	if (fd < 0) return -1;

	bzero(&newtio, sizeof(newtio));
	newtio.c_oflag = 0;
	newtio.c_cflag = _get_baudrate(baudrate) | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;

	newtio.c_lflag = 0; 
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);

	return fd;
}

int uart_send(int fd, char *buf, int len)
{
	int ret = 0;
	char *b;

	if (fd < 0 || buf == NULL || len == 0) return -1;

	b = buf;
	while (len > 0) {
		ret = write(fd, b, 1);
		if (ret < 0) return -1;
		b++; len--;
	}

	return b - buf;
}

int uart_recv(int fd, char *buf, int len)
{
	fd_set rdset;
	int ret;
	struct timeval tm;

	if (fd < 0 || buf == NULL || len == 0) return -1;

	FD_ZERO(&rdset);
	FD_SET(fd, &rdset);

	tm.tv_sec = 1;
	tm.tv_usec = 0;

	while (1) {
		ret = select(fd + 1, &rdset, NULL, NULL, &tm);
		if (ret > 0) {
			if (FD_ISSET(fd, &rdset))
				return read(fd, buf, len);
			else
				continue;
		} else if (ret == 0) {
			break;
		}
	}

	return ret;
}

void uart_flush(int fd)
{
//	tcflush(fd, TCIFLUSH);
}

int uart_close(int fd)
{
	if (fd > 0) {
		tcflush(fd, TCIFLUSH);
		close(fd);
	}

	return 0;
}

