/*
 * Copyright (c) 2021, Dreamgrow Development Team
 *
 * gpiotest program to test GPIO input event and outputfunction with libgpiod on Apalis iMX8
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-16     luhuadong    first version
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <gpiod.h>

int main(int argc, char *argv[])
{
    //Define GPIO chip and line structs:
    struct gpiod_chip *output_chip;
    struct gpiod_line *output_line;
    struct gpiod_chip *input_chip;
    struct gpiod_line *input_line;
    struct gpiod_line_event event;

    char chip_out[32];
    unsigned int offset_out;
    char chip_in[32];
    unsigned int offset_in;

    int line_value = 0;
    int cnt;
    int pressCnt = 0;

    /* check the arguments */
    if (argc == 5) {
        snprintf(chip_in, sizeof(chip_in), "gpiochip%s", argv[1]);
        offset_in = atoi(argv[2]);
        snprintf(chip_out, sizeof(chip_out), "gpiochip%s", argv[3]);
        offset_out = atoi(argv[4]);
    }
    else {
        printf("Usage by bank/pin number:\n"
         "\tgpio-test INPUT-BANK-NUMBER INPUT-GPIO-NUMBER OUTPUT-BANK-NUMBER OUTPUT-GPIO-NUMBER\n");
        return EXIT_FAILURE;
    }

    //Configure a GPIO as output:
    /* open the GPIO bank */
    output_chip = gpiod_chip_open_by_name(chip_out);
    /* open the GPIO line */
    output_line = gpiod_chip_get_line(output_chip, offset_out);
    /* config as output and set a description */
    if(gpiod_line_request_output(output_line, "gpiotest", GPIOD_LINE_ACTIVE_STATE_LOW) < 0) {
        perror("Request output failed\n");
        return EXIT_FAILURE;
    }
    gpiod_line_set_value(output_line, line_value);
    printf("LED initial status is OFF\n");

    //Configure a GPIO as input:
    /* open the GPIO bank */
    input_chip = gpiod_chip_open_by_name(chip_in);
    /* open the GPIO line */
    input_line = gpiod_chip_get_line(input_chip, offset_in);
    /* config as input and rising-edge event */
    if (gpiod_line_request_rising_edge_events(input_line, "gpiotest") < 0) {
        perror("Request events failed\n");
        return EXIT_FAILURE;
    }

    while (1)
    {
        gpiod_line_event_wait(input_line, NULL);

        if (gpiod_line_event_read(input_line, &event) != 0)
            continue;

        /* this should always be a rising event in our example */
        if (event.event_type != GPIOD_LINE_EVENT_RISING_EDGE)
            continue;

        /* GPIO key debounce */
        cnt = 0;
        for (int i=0; i<5; i++) {
            if(gpiod_line_get_value(input_line)==1) {
                cnt++;
            }
            usleep(50000);
        }

        if(cnt >= 3){
            /* Reverse GPIO LED*/
            pressCnt ++;
            line_value = !line_value;
            gpiod_line_set_value(output_line, line_value);
            /* print GPIO key pressed times and GPIO LED status */
            if(line_value == 1) {
                printf("button pressed %d times\n",pressCnt);
                printf("LED turns ON\n");
            }
            else{
                printf("button pressed %d times\n",pressCnt);
                printf("LED turns OFF\n");
            }
        }
    }

    return EXIT_SUCCESS;
}