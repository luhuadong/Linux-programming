/*
 * Copyright (c) DreamsGrow Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-01-20     luhuadong    the first version
 *
 */

#include <linux/input.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define  INPUT_KEY      "/dev/input/event1"    /* 我电脑的键盘 */
#define  INPUT_MOUSE    "/dev/input/event2"    /* 我电脑的鼠标 */

int main(void)
{
    int fd = -1, ret = -1;
    struct input_event ev;
 
    fd = open(INPUT_MOUSE , O_RDONLY);    /* 打开鼠标的设备文件 */
    if(fd < 0)
    {
        perror("open");
        return -1;
    }

    while(1)
    {
        memset(&ev, 0, sizeof(struct input_event));    
        ret = read(fd, &ev, sizeof(struct input_event));    /* 读鼠标（会阻塞） */

        if(ret != sizeof(struct input_event))
        {
            perror("read");
            close(fd);
            return -1;
        }

        /* 打印读到的键值 */
        printf("--------------------\n");
#if 0
        printf("type = %u\n", ev.type);
        printf("code = %u\n", ev.code);
        printf("value = %u\n", ev.value);
#else
        printf("type = %u ", ev.type);
        if (ev.type == 0x00)
        {
            printf("(EV_SYN)\n");
            printf("code = %u\n", ev.code);
            printf("value = %u\n", ev.value);
        }
        else if (ev.type == 0x01)
        {
            printf("(EV_KEY)\n");
            printf("code = %u ", ev.code);
            if (ev.code == 272)
                printf("BTN_LEFT\n");
            else if (ev.code == 273)
                printf("BTN_RIGHT\n");
            else if (ev.code == 274)
                printf("BTN_MIDDLE\n");
            printf("value = %u\n", ev.value);
        }
        else if (ev.type == 0x02)
        {
            printf("(EV_REL)\n");
            printf("code = %u ", ev.code);
            if (ev.code == 0)
                printf("REL_X\n");
            else if (ev.code == 1)
                printf("REL_Y\n");
            else if (ev.code == 8)
                printf("REL_WHEEL\n");
            printf("value = %u\n", ev.value);
        }
        else if (ev.type == 0x03)
        {
            printf("(EV_ABS)\n");
            printf("code = %u\n", ev.code);
            printf("value = %u\n", ev.value);
        }
        else if (ev.type == 0x04)
        {
            printf("(EV_MSC)\n");
            printf("code = %u ", ev.code);
            if (ev.code == 4)
                printf("MSC_SCAN\n");
            printf("value = %u\n", ev.value);
        }
        else
        {
            printf("code = %u\n", ev.code);
            printf("value = %u\n", ev.value);
        }
#endif
    }

    return 0;
}