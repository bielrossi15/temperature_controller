#include <stdio.h>
#include <stdlib.h>
#include <bcm2835.h>
#include <gpio.h>

#define R_PIN RPI_V2_GPIO_P1_16
#define V_PIN RPI_V2_GPIO_P1_18

void configure_r_pin()
{
    if(!bcm2835_init())
    {
        fprintf(stderr, "Error initializing bcm2835\n");
        return;
    }
    bcm2835_gpio_fsel(R_PIN, BCM2835_GPIO_FSEL_OUTP);
}

void configure_v_pin()
{
    if(!bcm2835_init())
    {
        fprintf(stderr, "Error initializing bcm2835\n");
        return;   
    }
    bcm2835_gpio_fsel(V_PIN, BCM2835_GPIO_FSEL_OUTP);
}

void set_r_pin_value(short value)
{
    bcm2835_gpio_write(R_PIN, value);
}

void set_v_pin_value(short value)
{
    bcm2835_gpio_write(V_PIN, value);
}
