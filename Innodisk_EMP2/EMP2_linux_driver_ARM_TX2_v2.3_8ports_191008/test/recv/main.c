#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#if defined(__linux__)
#include <signal.h>
#include <sys/timeb.h>
#else
#include <windows.h>
#include <time.h>
#endif

#include "ini.h"
#include "rs232.h"

typedef struct
{
    char *port_path;
    unsigned int recv_byte_once;
    unsigned int baud_rate;
    unsigned int recv_byte_total;
    bool show_recv_data;

} INI_ITEM;


INI_ITEM ini_item;

#if defined(__linux__)
int port_fd = -1;
#else
HANDLE port_hd = NULL;
#endif

char *start_time = NULL;
char *end_time = NULL;

void initDev (void);
void startRecv (void);
void getIni (void);
int handlerIni (void* user, const char* section, const char* name, const char* value);

#if defined(__linux__)
void end_test (int type);
#else
#endif

char *get_now_time (void);
char *cal_itvl (char *start_time, char *end_time);
void split (char **arr, char *str, const char *del);


/*------------------------------------------------------------------*/
int main (void)
{
    getIni();
    initDev();

    #if defined(__linux__)
    signal(SIGINT, end_test);
    #else
    #endif

    startRecv();

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
    if(RS232_SetRecvBlock(port_fd, 1))
    {
        printf("Set receive block mode failed\n");
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

    /* "recv block mode" for windows: NOT yet */
    #if 0
    if(RS232_SetRecvBlock(port_hd, 1))
    {
        printf("Set receive block mode failed\n");
        exit(1);
    }
    #endif
    RS232_flushRX(port_hd);
    #endif



    printf("Open '%s' success\n", port_path);
}


/*------------------------------------------------------------------*/
void startRecv (void)
{
    bool is_first = true;
    int rtn;
    unsigned int cnt = 0;
    unsigned int recv_byte = 0;
    bool show_recv_data = ini_item.show_recv_data;
    unsigned int recv_byte_total = ini_item.recv_byte_total;
    unsigned int recv_byte_once = ini_item.recv_byte_once;
    unsigned char *buf_recv = (unsigned char *) malloc(recv_byte_once * sizeof(unsigned char));

    while(1)
    {
        memset(buf_recv, 0, recv_byte_once);

        #if defined(__linux__)
        usleep(1000);
        rtn = RS232_PollComport(port_fd, buf_recv, recv_byte_once);
        #else
        Sleep(1);
        rtn = RS232_PollComport(port_hd, buf_recv, recv_byte_once);
        #endif

        if(rtn > 0)
        {
            /* record the start time */
            if(is_first)
            {
                start_time = get_now_time();
                is_first = false;
            }

            if(end_time)
            {
                free(end_time);
            }
            end_time = get_now_time();

            cnt++;
            printf("%6u. ", cnt);

            /* show recv data or not */
            if(show_recv_data)
            {
                printf("Reve Data: ");
                for(int i=0; i<rtn; i++)
                {
                    printf("%02X ", *(buf_recv + i));
                }                
            }

            recv_byte+=rtn;
            printf("(now:%5d, total:%8u [bytes], %s)\n", rtn, recv_byte, end_time);

            /* if recv_byte_total is set: calculate interval, else: endless loop */
            /*-------------------------------------------------------------------*/
            if(recv_byte == recv_byte_total)
            {
                printf("Start time: %s\n", start_time);
                printf("End time:   %s\n", end_time);
                printf("Interval:   %s\n", cal_itvl(start_time, end_time));
                break;
            }
            /*-------------------------------------------------------------------*/
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
    }
    else if(MATCH("setting", "recv_byte_once"))
    {
        p_ini->recv_byte_once = (unsigned int) atoi(value);
    }
    else if(MATCH("setting", "baud_rate"))
    {
        p_ini->baud_rate = (unsigned int) atoi(value);
    }
    else if(MATCH("setting", "recv_byte_total"))
    {
        p_ini->recv_byte_total = (unsigned int) atoi(value);
    }
    else if(MATCH("setting", "show_recv_data"))
    {
        p_ini->show_recv_data = (bool) atoi(value);
    }    
    /* unknown section or name*/
    else
    {
        return 0;
    }

    return 1;
}


#if defined(__linux__)
/*------------------------------------------------------------------*/
void end_test (int type)
{
    if(type == SIGINT)
    {
        RS232_CloseComport(port_fd);
        printf("\n");
        if(start_time != NULL && end_time != NULL)
        {
            printf("Start time: %s\n", start_time);
            printf("End time:   %s\n", end_time);
            printf("Interval:   %s\n", cal_itvl(start_time, end_time));
        }
        printf("\n");
        exit(0);
    }
}
#else
#endif


/*------------------------------------------------------------------*/
char *get_now_time (void)
{
    #define  TIME_CHAR_NUM  13

    char          time_str[TIME_CHAR_NUM];
    struct tm    *timeinfo;
    struct timeb  now;

    ftime(&now);
    timeinfo = localtime((time_t*) &now);
    sprintf(time_str, "%02d:%02d:%02d:%03d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, now.millitm);

    return strdup(time_str);
}


/*------------------------------------------------------------------*/
char *cal_itvl (char *start_time, char *end_time)
{
    int i;
    int j;
    const char *del = ":";
    char rtn_str[12 + 1];

    /* 4: hour/min/sec/ms */
    char *start_arr[4];
    char *end_arr[4];
    int start_int[4];
    int end_int[4];
    int itvl_int[4] = {0};

    split(start_arr, start_time, del);
    split(end_arr, end_time, del);

    i = 0;
    j = 0;
    while(i<4 && j<4)
    {
        start_int[i++] = atoi(*(start_arr+j++));
    }

    i = 0;
    j = 0;
    while(i<4 && j<4)
    {
        end_int[i++] = atoi(*(end_arr+j++));
    }

    /* now we get hour/min/sec/ms of both start time and end time */
    if(end_int[0] >= start_int[0])
    {
        /* ms */
        if(end_int[3] >= start_int[3])
        {
            itvl_int[3] = end_int[3] - start_int[3];
        }
        else
        {
            itvl_int[3] = end_int[3] + 1000 - start_int[3];
            end_int[2] = end_int[2] - 1;
        }

        /* sec */
        if(end_int[2] >= start_int[2])
        {
            itvl_int[2] = end_int[2] - start_int[2];
        }
        else
        {
            itvl_int[2] = end_int[2] + 60 - start_int[2];
            end_int[1] = end_int[1] - 1;
        }

        /* min */
        if(end_int[1] >= start_int[1])
        {
            itvl_int[1] = end_int[1] - start_int[1];
        }
        else
        {
            itvl_int[1] = end_int[1] + 60 - start_int[1];
            end_int[0] = end_int[0] - 1;
        }
    }
    /* the next day */
    else
    {
        int total_start_int[4] = {0};
        int total_end_int[4] = {0};

        /* get start total itvl */
        if(start_int[3] != 0)
        {
            total_start_int[3] = 1000 - start_int[3];
            start_int[2]++;
        }
        else
        {
            total_start_int[3] = 0;
        }

        if(start_int[2] != 0)
        {
            total_start_int[2] = 60 - start_int[2];
            start_int[1]++;
        }
        else
        {
            total_start_int[2] = 0;
        }

        if(start_int[1] != 0)
        {
            total_start_int[1] = 60 - start_int[1];
            start_int[0]++;
        }
        else
        {
            total_start_int[1] = 0;
        }

        total_start_int[0] = 24 - start_int[0];


        /* get end total itvl */
        for(int i=0; i<4; i++)
        {
            total_end_int[i] = end_int[i];
        }

        /* get all itvl */
        for(int i=0; i<4; i++)
        {
            itvl_int[i] = total_start_int[i] + total_end_int[i];
        }

        if(itvl_int[3] >= 1000)
        {
            itvl_int[3] = itvl_int[3] - 1000;
            itvl_int[2]++;
        }

        if(itvl_int[2] >= 60)
        {
            itvl_int[2] = itvl_int[2] - 60;
            itvl_int[1]++;
        }

        if(itvl_int[1] >= 60)
        {
            itvl_int[1] = itvl_int[1] - 60;
            itvl_int[0]++;
        }
    }

    sprintf(rtn_str, "%02d:%02d:%02d:%03d", itvl_int[0], itvl_int[1], itvl_int[2], itvl_int[3]);
    return strdup(rtn_str);
}


/*----------------------------------------------------------------------*/
void split (char **arr, char *str, const char *del)
{
    char *s = strtok(str, del);

    while(s != NULL)
    {
        *arr++ = s;
        s = strtok(NULL, del);
    }
}
