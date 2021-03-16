#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "ini.h"
#include "rs232.h"

typedef struct
{
    char *port_path;
    unsigned int send_byte_once;
    unsigned int send_loop_cnt;
    unsigned int sleep_interval;
    unsigned int baud_rate;

} INI_ITEM;


INI_ITEM ini_item;

#if defined(__linux__)
    int port_fd = -1;
#else
    HANDLE port_hd = NULL;
#endif

void initDev (void);
void startSend (void);
void getIni (void);
int handlerIni (void* user, const char* section, const char* name, const char* value);


/*------------------------------------------------------------------*/
int main (void)
{
    getIni();
    initDev();
    startSend();

    return 0;
}


/*------------------------------------------------------------------*/
void initDev (void)
{
    char *port_path = ini_item.port_path;
    int  baud_rate = ini_item.baud_rate;
    char mode[] = {'8', 'N', '1', 0};


    #if defined(__linux__)
    port_fd = RS232_OpenComport(port_path, baud_rate, mode);
    if(port_fd == -1)
    {
        printf("Open '%s' fail\n", port_path);
        exit(1);
    }
    RS232_flushRX(port_fd);
    #else
    port_hd = RS232_OpenComport(port_path, baud_rate, mode);
    if(port_hd == NULL)
    {
        printf("Open '%s' fail\n", port_path);
        exit(1);
    }
    RS232_flushRX(port_hd);
    #endif

    printf("Open '%s' success\n", port_path);
}


/*------------------------------------------------------------------*/
void startSend (void)
{
    unsigned int send_byte_once = ini_item.send_byte_once;
    unsigned int send_loop_cnt = ini_item.send_loop_cnt;
    unsigned int sleep_interval = ini_item.sleep_interval;
    unsigned char *buf_send = (unsigned char *) malloc(send_byte_once * sizeof(unsigned char));

    for(unsigned int i=0; i<send_byte_once; i++)
    {
        buf_send[i] = i;
    }

    for(int i=0; i<send_loop_cnt; i++)
    {
        #if defined(__linux__)
        if(RS232_SendBuf(port_fd, buf_send, send_byte_once) == send_byte_once)
        #else
        if(RS232_SendBuf(port_hd, buf_send, send_byte_once) == send_byte_once)
        #endif
        {
            printf("Send success ... (%2d/%d)\n", i+1, send_loop_cnt);
        }
        else
        {
            printf("Send fail ... (%2d/%d)\n", i+1, send_loop_cnt);
        }

        if(i < send_loop_cnt - 1)
        {
            #if defined(__linux__)
            usleep(sleep_interval);
            #else
            Sleep(sleep_interval);
            #endif
        }
    }
}


/*------------------------------------------------------------------*/
void getIni (void)
{
    if(ini_parse("setup.ini", handlerIni, &ini_item) < 0)
    {
        printf("Open setup.ini failed\n");
        exit(1);
    }
}


/*------------------------------------------------------------------*/
int handlerIni (void* user, const char* section, const char* name, const char* value)
{
    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    INI_ITEM *p_ini = (INI_ITEM *) user;

    /* [setting] section */
    if(MATCH("setting", "port_path"))
    {
        p_ini->port_path = strdup(value);
        printf("%s\n", p_ini->port_path);
    }
    else if(MATCH("setting", "send_byte_once"))
    {
        p_ini->send_byte_once = (unsigned int) atoi(value);
    }
    else if(MATCH("setting", "send_loop_cnt"))
    {
        p_ini->send_loop_cnt = (unsigned int) atoi(value);
    }
    else if(MATCH("setting", "baud_rate"))
    {
        p_ini->baud_rate = (unsigned int) atoi(value);
    }
    else if(MATCH("setting", "sleep_interval"))
    {
        #if defined(__linux__)
        p_ini->sleep_interval = (unsigned int) atoi(value);
        #else
        p_ini->sleep_interval = ((unsigned int) atoi(value)) / 1000;
        #endif
    }
    /* unknown section or name*/
    else
    {
        return 0;
    }

    return 1;

}