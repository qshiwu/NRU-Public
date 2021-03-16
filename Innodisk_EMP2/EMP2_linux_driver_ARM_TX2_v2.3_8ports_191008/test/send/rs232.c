#include "rs232.h"

void showDebugMsg (int line, const char *func, const char *msg)
{
    #if 1
    printf("%s, %d: %s\n", func, line, msg);
    #endif
}


/* Linux & FreeBSD */
/*=================================================================================================================*/
#if defined(__linux__) || defined(__FreeBSD__) || defined(__QNX__)

struct termios port_settings;

/*----------------------------------------------------------------------------*/
int RS232_OpenComport(char *com_path, int baudrate, const char *mode)
{
    int   fd;
    int   error;
    int   baudr;
    int   status;
    int   cbits = CS8;
    int   cpar = 0;
    int   ipar = IGNPAR;
    int   bstop = 0;


    switch(baudrate)
    {
        case      50 : baudr = B50;
                       break;
        case      75 : baudr = B75;
                       break;
        case     110 : baudr = B110;
                       break;
        case     134 : baudr = B134;
                       break;
        case     150 : baudr = B150;
                       break;
        case     200 : baudr = B200;
                       break;
        case     300 : baudr = B300;
                       break;
        case     600 : baudr = B600;
                       break;
        case    1200 : baudr = B1200;
                       break;
        case    1800 : baudr = B1800;
                       break;
        case    2400 : baudr = B2400;
                       break;
        case    4800 : baudr = B4800;
                       break;
        case    9600 : baudr = B9600;
                       break;
        case   19200 : baudr = B19200;
                       break;
        case   38400 : baudr = B38400;
                       break;
        case   57600 : baudr = B57600;
                       break;
        case  115200 : baudr = B115200;
                       break;
        case  230400 : baudr = B230400;
                       break;
        case  460800 : baudr = B460800;
                       break;
        case  500000 : baudr = B500000;
                       break;
        case  576000 : baudr = B576000;
                       break;
        case  921600 : baudr = B921600;
                       break;
        case 1000000 : baudr = B1000000;
                       break;
        case 1152000 : baudr = B1152000;
                       break;
        case 1500000 : baudr = B1500000;
                       break;
        case 2000000 : baudr = B2000000;
                       break;
        case 2500000 : baudr = B2500000;
                       break;
        case 3000000 : baudr = B3000000;
                       break;
        case 3500000 : baudr = B3500000;
                       break;
        case 4000000 : baudr = B4000000;
                       break;
        default      : 
                       showDebugMsg(__LINE__, __FUNCTION__, "invalid baudrate");
                       return(1);
    }

    if(strlen(mode) != 3)
    {
        showDebugMsg(__LINE__, __FUNCTION__, "invalid mode");
        return(1);
    }

    switch(mode[0])
    {
        case '8': cbits = CS8;
                  break;
        case '7': cbits = CS7;
                  break;
        case '6': cbits = CS6;
                  break;
        case '5': cbits = CS5;
                  break;
        default : 
                  showDebugMsg(__LINE__, __FUNCTION__, "invalid number of data-bits");
                  return(1);
    }

    switch(mode[1])
    {
        case 'N':
        case 'n': cpar = 0;
                  ipar = IGNPAR;
                  break;
        case 'E':
        case 'e': cpar = PARENB;
                  ipar = INPCK;
                  break;
        case 'O':
        case 'o': cpar = (PARENB | PARODD);
                  ipar = INPCK;
                  break;
        default :
                  showDebugMsg(__LINE__, __FUNCTION__, "invalid parity");
                  return(1);
    }

    switch(mode[2])
    {
        case '1': bstop = 0;
                  break;
        case '2': bstop = CSTOPB;
                  break;
        default :
                  showDebugMsg(__LINE__, __FUNCTION__, "invalid number of stop bits");
                  return(1);
                  break;
    }

    fd = open(com_path, O_RDWR | O_NOCTTY | O_NDELAY);

    if(fd == -1)
    {
        showDebugMsg(__LINE__, __FUNCTION__, "unable to open comport");
        return -1;
    }

    /* lock access so that another process can't also use the port */
    if(flock(fd, LOCK_EX | LOCK_NB) != 0)
    {
        close(fd);
        showDebugMsg(__LINE__, __FUNCTION__, "Another process has locked the comport");
        return -1;
    }

    error = tcgetattr(fd, &port_settings);
    if(error == -1)
    {
        close(fd);
        flock(fd, LOCK_UN);  /* free the port so that others can use it. */
        showDebugMsg(__LINE__, __FUNCTION__, "unable to read portsettings");
        return -1;
    }

    memset(&port_settings, 0, sizeof(port_settings));

    port_settings.c_cflag = cbits | cpar | bstop | CLOCAL | CREAD;
    port_settings.c_iflag = ipar;
    port_settings.c_oflag = 0;
    port_settings.c_lflag = 0;
    port_settings.c_cc[VMIN] = 0;     /* block untill n bytes are received */
    port_settings.c_cc[VTIME] = 0;    /* block untill a timer expires (n * 100 mSec.) */

    cfsetispeed(&port_settings, baudr);

    error = tcsetattr(fd, TCSANOW, &port_settings);
    if(error == -1)
    {
        close(fd);
        flock(fd, LOCK_UN);  /* free the port so that others can use it. */
        showDebugMsg(__LINE__, __FUNCTION__, "unable to adjust portsettings");

        return -1;
    }

    if(ioctl(fd, TIOCMGET, &status) == -1)
    {
        flock(fd, LOCK_UN);  /* free the port so that others can use it. */
        showDebugMsg(__LINE__, __FUNCTION__, "unable to get portstatus");

        return -1;
    }

    status |= TIOCM_DTR;    /* turn on DTR */
    status |= TIOCM_RTS;    /* turn on RTS */

    if(ioctl(fd, TIOCMSET, &status) == -1)
    {
        flock(fd, LOCK_UN);  /* free the port so that others can use it. */
        showDebugMsg(__LINE__, __FUNCTION__, "unable to set portstatus");

        return -1;
    }

    return fd;
}


/*----------------------------------------------------------------------------*/
int RS232_PollComport(int fd, unsigned char *buf, int size)
{
  int n;

  n = read(fd, buf, size);

  if(n < 0)
  {
    if(errno == EAGAIN)  return 0;
  }

  return(n);
}


/*----------------------------------------------------------------------------*/
int RS232_SendBuf(int fd, unsigned char *buf, int size)
{
    int n = write(fd, buf, size);

    if(n < 0)
    {
        if(errno == EAGAIN)
        {
            return -1;
        }
        else
        {
            return -2;
        }
    }

    return n;
}


/*----------------------------------------------------------------------------*/
void RS232_CloseComport(int fd)
{
    int  status;

    if(ioctl(fd, TIOCMGET, &status) == -1)
    {
        showDebugMsg(__LINE__, __FUNCTION__, "unable to get portstatus");
    }

    status &= ~TIOCM_DTR;    /* turn off DTR */
    status &= ~TIOCM_RTS;    /* turn off RTS */

    if(ioctl(fd, TIOCMSET, &status) == -1)
    {
        showDebugMsg(__LINE__, __FUNCTION__, "unable to set portstatus");
    }

    memset(&port_settings, 0, sizeof(port_settings));
    close(fd);
    flock(fd, LOCK_UN);  /* free the port so that others can use it. */
}


/*----------------------------------------------------------------------------*/
void RS232_flushRX(int fd)
{
    tcflush(fd, TCIFLUSH);
}


/*----------------------------------------------------------------------------*/
int RS232_SetRecvBlock (int fd, int is_enable)
{
    if(is_enable)
    {
        port_settings.c_cc[VMIN] = 1;
    }
    else
    {
        port_settings.c_cc[VMIN] = 0;
    }

    return tcsetattr(fd, TCSANOW, &port_settings);
}

/* Windows */
/*=================================================================================================================*/
#else


/*----------------------------------------------------------------------------*/
HANDLE RS232_OpenComport(char *com_path, int baudrate, const char *mode)
{
    HANDLE        hd;
    char          mode_str[128];
    DCB           port_settings;
    COMMTIMEOUTS  Cptimeouts;

    switch(baudrate)
    {
        case     110 : strcpy(mode_str, "baud=110");
                       break;
        case     300 : strcpy(mode_str, "baud=300");
                       break;
        case     600 : strcpy(mode_str, "baud=600");
                       break;
        case    1200 : strcpy(mode_str, "baud=1200");
                       break;
        case    2400 : strcpy(mode_str, "baud=2400");
                       break;
        case    4800 : strcpy(mode_str, "baud=4800");
                       break;
        case    9600 : strcpy(mode_str, "baud=9600");
                       break;
        case   19200 : strcpy(mode_str, "baud=19200");
                       break;
        case   38400 : strcpy(mode_str, "baud=38400");
                       break;
        case   57600 : strcpy(mode_str, "baud=57600");
                       break;
        case  115200 : strcpy(mode_str, "baud=115200");
                       break;
        case  128000 : strcpy(mode_str, "baud=128000");
                       break;
        case  256000 : strcpy(mode_str, "baud=256000");
                       break;
        case  500000 : strcpy(mode_str, "baud=500000");
                       break;
        case 1000000 : strcpy(mode_str, "baud=1000000");
                       break;
        default      :
                       showDebugMsg(__LINE__, __FUNCTION__, "invalid baudrate");
                       return NULL;
    }

    if(strlen(mode) != 3)
    {
        showDebugMsg(__LINE__, __FUNCTION__, "invalid baudrate");
        return NULL;
    }

    switch(mode[0])
    {
        case '8': strcat(mode_str, " data=8");
                  break;
        case '7': strcat(mode_str, " data=7");
                  break;
        case '6': strcat(mode_str, " data=6");
                  break;
        case '5': strcat(mode_str, " data=5");
                  break;
        default :
                  showDebugMsg(__LINE__, __FUNCTION__, "invalid number of data-bits");
                  return NULL;
    }

    switch(mode[1])
    {
        case 'N':
        case 'n': strcat(mode_str, " parity=n");
                  break;
        case 'E':
        case 'e': strcat(mode_str, " parity=e");
                  break;
        case 'O':
        case 'o': strcat(mode_str, " parity=o");
                  break;
        default :
                  showDebugMsg(__LINE__, __FUNCTION__, "invalid parity");
                  return NULL;
    }

    switch(mode[2])
    {
        case '1': strcat(mode_str, " stop=1");
                  break;
        case '2': strcat(mode_str, " stop=2");
                  break;
        default :
                  showDebugMsg(__LINE__, __FUNCTION__, "invalid number of stop bits");
                  return NULL;
    }

    strcat(mode_str, " dtr=on rts=on");

    hd = CreateFileA(com_path,
                     GENERIC_READ|GENERIC_WRITE,
                     0,                          /* no share */
                     NULL,                       /* no security */
                     OPEN_EXISTING,
                     0,                          /* no threads */
                     NULL);                      /* no templates */


    if(hd == INVALID_HANDLE_VALUE)
    {
        showDebugMsg(__LINE__, __FUNCTION__, "unable to open comport");
        return NULL;
    }

    memset(&port_settings, 0, sizeof(port_settings));
    port_settings.DCBlength = sizeof(port_settings);

    if(!BuildCommDCBA(mode_str, &port_settings))
    {
        showDebugMsg(__LINE__, __FUNCTION__, "unable to set comport dcb settings");
        CloseHandle(hd);
        return NULL;
    }

    if(!SetCommState(hd, &port_settings))
    {
        showDebugMsg(__LINE__, __FUNCTION__, "unable to set comport cfg settings");
        CloseHandle(hd);
        return NULL;
    }

    Cptimeouts.ReadIntervalTimeout         = MAXDWORD;
    Cptimeouts.ReadTotalTimeoutMultiplier  = 0;
    Cptimeouts.ReadTotalTimeoutConstant    = 0;
    Cptimeouts.WriteTotalTimeoutMultiplier = 0;
    Cptimeouts.WriteTotalTimeoutConstant   = 0;

    if(!SetCommTimeouts(hd, &Cptimeouts))
    {
        showDebugMsg(__LINE__, __FUNCTION__, "unable to set comport time-out settings");
        CloseHandle(hd);
        return NULL;
    }

    return hd;
}


/*----------------------------------------------------------------------------*/
int RS232_PollComport (HANDLE hd, unsigned char *buf, int size)
{
    int n;

    /* added the void pointer cast, otherwise gcc will complain about */
    /* "warning: dereferencing type-punned pointer will break strict aliasing rules" */
    ReadFile(hd, buf, size, (LPDWORD)((void *)&n), NULL);

    return(n);
}


/*----------------------------------------------------------------------------*/
int RS232_SendBuf (HANDLE hd, unsigned char *buf, int size)
{
    int n;

    if(WriteFile(hd, buf, size, (LPDWORD)((void *)&n), NULL))
    {
        return(n);
    }

    return(-1);
}


/*----------------------------------------------------------------------------*/
void RS232_CloseComport (HANDLE hd)
{
    CloseHandle(hd);
}


/*----------------------------------------------------------------------------*/
void RS232_flushRX (HANDLE hd)
{
    PurgeComm(hd, PURGE_RXCLEAR | PURGE_RXABORT);
}


/*----------------------------------------------------------------------------*/
int RS232_SetRecvBlock (HANDLE hd, int is_enable)
{
    /* not yet */
    return -1;
}

#endif