/*
 * Copyright (c) 2021, Dreamgrow Development Team
 *
 * gpio-toggle program to test output function with libgpiod on Apalis iMX8
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
    int line_value = 0;
    char chip[32];
    unsigned int offset;

    /* check the arguments */
    if (argc == 3) {
        snprintf(chip, sizeof(chip), "gpiochip%s", argv[1]);
        offset = atoi(argv[2]);
    }
    else {
        printf("Usage by bank/pin number:\n"
               "\tgpio-toggle OUTPUT-BANK-NUMBER OUTPUT-GPIO-NUMBER\n");
        return EXIT_FAILURE;
    }

    /* open the GPIO bank */
    output_chip = gpiod_chip_open_by_name(chip);
    /* open the GPIO line */
    output_line = gpiod_chip_get_line(output_chip, offset);
    /* config as output and set a description */
    gpiod_line_request_output(output_line, "gpio-toggle", GPIOD_LINE_ACTIVE_STATE_HIGH);

    while (1)
    {
        /* GPIO Toggle */
        line_value = !line_value;
        gpiod_line_set_value(output_line, line_value);

        if(line_value==1){
            printf("LED turns ON\n");
        }
        else{
            printf("LED turns OFF\n");
        }

        sleep(1);
    }

    return 0;
}