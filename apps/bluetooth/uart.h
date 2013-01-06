#ifndef __UART_H__
#define __UART_H__

int uart_open(const char *dev, int baudrate);
int uart_send(int fd, char *buf, int len);
int uart_recv(int fd, char *buf, int len);
void uart_flush(int fd);
int uart_close(int fd);

#endif // __UART_H__
