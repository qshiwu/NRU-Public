#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#if defined(__linux__)
#include <sys/timeb.h>
#else
#include <windows.h>
#include <time.h>
#endif

#include "ini.h"
#include "rs232.h"

#define clearScreen printf("\033[H\033[J")

typedef struct
{
    /* ini item */
    char *port_path;
    unsigned int send_byte_once;
    unsigned int time_out;
    unsigned int baud_rate;

    /* others */
#if defined(__linux__)
    int   port_fd;
#else
    HANDLE port_hd;
#endif

    int  *TRD_recv_rtn;
    char *start_time;
    char *end_time;
    char *itvl_time;

} TEST_INFO;


void getIni (TEST_INFO *test_info);
void initDev (TEST_INFO *test_info);
void startTest (TEST_INFO *test_info);
void *TRD_recv_fx (void *ptr);
int handlerIni (void* user, const char* section, const char* name, const char* value);
char *get_now_time (void);
char *cal_itvl (char *start_time, char *end_time);
void split (char **arr, char *str, const char *del);
void freeCharStr (char *str);


/*------------------------------------------------------------------*/
int main (void)
{
    #if defined(__linux__)
    TEST_INFO  test_info = {NULL, 0, 0, 0, -1, NULL, NULL, NULL, NULL};
    #else
    TEST_INFO  test_info = {NULL, 0, 0, 0, NULL, NULL, NULL, NULL, NULL};
    #endif

    getIni(&test_info);
    initDev(&test_info);
    startTest(&test_info);

    return 0;
}


/*------------------------------------------------------------------*/
void getIni (TEST_INFO *test_info)
{
    if(ini_parse("setup.ini", handlerIni, test_info) < 0)
    {
        printf("Open setup.ini failed\n");
        exit(1);
    }
}


/*------------------------------------------------------------------*/
void initDev (TEST_INFO *test_info)
{
    char *port_path = test_info->port_path;
    int  baud_rate = test_info->baud_rate;
    char mode[] = {'8', 'N', '1', 0};

    #if defined(__linux__)
        int port_fd = RS232_OpenComport(port_path, baud_rate, mode);
        if(port_fd == -1)
        {
            printf("Open '%s' fail\n", port_path);
            exit(1);
        }
        RS232_flushRX(port_fd);
        test_info->port_fd = port_fd;
    #else
        HANDLE port_hd = RS232_OpenComport(port_path, baud_rate, mode);
        if(port_hd == NULL)
        {
            printf("Open '%s' fail\n", port_path);
            exit(1);
        }
        RS232_flushRX(port_hd);
        test_info->port_hd = port_hd;
    #endif


    printf("Open '%s' success\n", port_path);
}


/*------------------------------------------------------------------*/
void startTest (TEST_INFO *test_info)
{
    #if defined(__linux__)
    int port_fd = test_info->port_fd;
    #else
    HANDLE port_hd = test_info->port_hd;
    #endif

    unsigned int send_byte_once = test_info->send_byte_once;
    unsigned char *buf_send = (unsigned char *) malloc(send_byte_once * sizeof(unsigned char));

    while(1)
    {
        pthread_t  TRD_recv = 0;

        pthread_create(&TRD_recv, NULL, TRD_recv_fx, (void *) test_info);

        /* test data: 0x01 (or someone else) */
        memset(buf_send, 0x01, send_byte_once);

        #if defined(__linux__)
            if(RS232_SendBuf(port_fd, buf_send, send_byte_once) != send_byte_once)
            {
                printf("Send failed !\n");
                exit(1);
            }
        #else
            if(RS232_SendBuf(port_hd, buf_send, send_byte_once) != send_byte_once)
            {
                printf("Send failed !\n");
                exit(1);
            }
        #endif

        pthread_join(TRD_recv, (void**) &test_info->TRD_recv_rtn);

        if(test_info->TRD_recv_rtn == (void *) 0)
        {
            clearScreen;
            printf("start:    %s\n", test_info->start_time);
            printf("end:      %s\n", test_info->end_time);
            printf("interval: %s\n", test_info->itvl_time);
        }
        else
        {
            printf("Receive time out !\n");
            exit(1);
        }
    }
}


/*------------------------------------------------------------------*/
void *TRD_recv_fx (void *ptr)
{
    TEST_INFO *test_info = (TEST_INFO *) ptr;

    int recv_byte = 0;
    int total_recv_byte = 0;
    void *rtn = NULL;

    #if defined(__linux__)
    int port_fd = test_info->port_fd;
    clock_t time_out = (test_info->time_out)*1000;
    #else
    HANDLE port_hd = test_info->port_hd;
    clock_t time_out = (test_info->time_out);
    #endif

    clock_t start;
    clock_t now;
    unsigned int send_byte_once = test_info->send_byte_once;
    unsigned char *buf_recv = (unsigned char *) malloc(send_byte_once * sizeof(unsigned char));

    start = clock();
    freeCharStr(test_info->start_time);
    test_info->start_time = get_now_time();

    do
    {
        memset(buf_recv, 0, send_byte_once);

        #if defined(__linux__)
        recv_byte = RS232_PollComport(port_fd, buf_recv, send_byte_once);
        #else
        recv_byte = RS232_PollComport(port_hd, buf_recv, send_byte_once);
        #endif

        total_recv_byte+=recv_byte;
        now = clock();

        /* time out */
        if(now > start + time_out)
        {
            break;
        }

    } while(total_recv_byte < send_byte_once);


    /* recv success */
    if(total_recv_byte == send_byte_once)
    {
        freeCharStr(test_info->end_time);
        freeCharStr(test_info->itvl_time);
        test_info->end_time = get_now_time();
        test_info->itvl_time = cal_itvl(test_info->start_time, test_info->end_time);

        rtn = (void *) 0;
    }
    /* recv time out */
    else
    {
        rtn = (void *) 1;
    }

    free(buf_recv);
    pthread_exit((void *) rtn);

    return NULL;
}


/*------------------------------------------------------------------*/
int handlerIni (void* user, const char* section, const char* name, const char* value)
{
    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    TEST_INFO *p_ini = (TEST_INFO *) user;

    /* [setting] section */
    if(MATCH("setting", "port_path"))
    {
        p_ini->port_path = strdup(value);
    }
    else if(MATCH("setting", "baud_rate"))
    {
        p_ini->baud_rate = (unsigned int) atoi(value);
    }
    else if(MATCH("setting", "time_out"))
    {
        p_ini->time_out = (clock_t) atoi(value);
    }
    else if(MATCH("setting", "send_byte_once"))
    {
        p_ini->send_byte_once = (unsigned int) atoi(value);
    }
    /* unknown section or name*/
    else
    {
        return 0;
    }

    return 1;
}


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
char *cal_itvl (char *_start_time, char *_end_time)
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
    char *start_time = strdup(_start_time);
    char *end_time = strdup(_end_time);

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
    free(start_time);
    free(end_time);

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


/*----------------------------------------------------------------------*/
void freeCharStr (char *str)
{
    if(str != NULL)
    {
        free(str);
    }
}