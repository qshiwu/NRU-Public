#ifndef __RS232_H__
#define __RS232_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>

#if defined(__linux__) || defined(__FreeBSD__) || defined(__QNX__)
  #include <termios.h>
  #include <sys/ioctl.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <limits.h>
  #include <sys/file.h>
  #include <errno.h>
#else
  #include <windows.h>
#endif

#if defined(__QNX__)
  #define B230400  23400
  #define B460800  460800
  #define B500000  500000
  #define B576000  576000
  #define B921600  921600
  #define B1000000 1000000
  #define B1152000 1152000
  #define B1500000 1500000
  #define B2000000 2000000
  #define B2500000 2500000
  #define B3000000 3000000
  #define B3500000 3500000
  #define B4000000 4000000
#endif


#if defined(__linux__) || defined(__FreeBSD__) || defined(__QNX__)
int  RS232_OpenComport(char *com_path, int baudrate, const char *mode);
int  RS232_PollComport(int fd, unsigned char *buf, int size);
int  RS232_SendBuf(int fd, unsigned char *buf, int size);
void RS232_CloseComport(int fd);
void RS232_flushRX(int fd);
int  RS232_SetRecvBlock(int fd, int is_enable);
#else
HANDLE RS232_OpenComport(char *com_path, int baudrate, const char *mode);
int  RS232_PollComport(HANDLE hd, unsigned char *buf, int size);
int  RS232_SendBuf(HANDLE hd, unsigned char *buf, int size);
void RS232_CloseComport(HANDLE hd);
void RS232_flushRX(HANDLE hd);
int  RS232_SetRecvBlock(HANDLE hd, int is_enable);
#endif


#ifdef __cplusplus
}
#endif

#endif