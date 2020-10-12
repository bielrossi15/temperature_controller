#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <ncurses.h>

// including my own files
#include "uart.h"
#include "bme280_i2c.h"
#include "gpio.h"
#include "control_lcd.h"



#define HOUR_SIZE 9
#define DATE_SIZE 11

// functions to handle signals
void sig_handler(int signal);
void alarm_handler(int signal);

// functions to execute on threads
void * temperature_input();
void * internal_temp();
void * external_temp();
void * file_write(FILE* file);
void * controller();
void * menu();
void * lcd();

// helper functions
void format_time(char *date_string, char *hour_string);
void clear_outputs();

float TI = 0.0,
      TR = 0.0,
      TE = 0.0;

short count = 1, 
      p_controller = 0,
      hist = 4;

pthread_t t0, t1, t2, t3, t4, t5;
FILE *file;

char str_TR[50] = "",
     str_HIST[50] = "";


int main(int argc, const char * argv[])
{
    // else if(!file) 
    // {
    //     file = fopen("./data.csv", "a");
    //     fprintf(file, "DATE,HOUR,TI,TE,TR\n");
    // }

    // configuring GPIO pins
    //clear_outputs();
    configure_r_pin();
    configure_v_pin();

    // all handled signals
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGTSTP, sig_handler);
    signal(SIGALRM, alarm_handler);

    ualarm(500000, 500000);

    while(1)
   {
       menu();
   }
    
    return 0;
}

void sig_handler(int signal)
{
    printf("\nReceived signal %d, terminating program...\n", signal);
    alarm(0);
    set_v_pin_value(1);
    set_r_pin_value(1);
    clear_lcd();
    
    exit(0);
}

void alarm_handler(int signal)
{
    if(p_controller == 1)
    {
        pthread_create(&t0, NULL, temperature_input, NULL); // get TR
    }



    pthread_create(&t1, NULL, internal_temp, NULL); // get TI
    pthread_create(&t2, NULL, external_temp, NULL); // get TE

    // pthread_create(&t3, NULL, lcd, NULL); // write in lcd
    // pthread_create(&t4, NULL, controller, NULL); // control internal temperature

    if(count == 4)
    {
        pthread_create(&t5, NULL, (void *) &file_write, file);
        count = 0;
    }
    count++; 
}

void * file_write(FILE *file)
{
    char date[DATE_SIZE];
    char hour[HOUR_SIZE];

    file = fopen("./data.csv", "a");

    format_time(date, hour);
    fprintf(file, "%s,%s,%0.2f,%0.2f,%0.2f\n", date, hour, TI, TE, TR);
    fclose(file);

}

void * temperature_input()
{
    call_uart(1, &TR);
}

void * internal_temp()
{
    call_uart(0, &TI);  
}

void * external_temp()
{
    initialize_bme("/dev/i2c-1", &TE);
}

void * controller()
{
    if(TI > (TR + (hist / 2)))
    {
        set_r_pin_value(1);
        set_v_pin_value(0);
    }

    else if(TI < (TR - (hist / 2)))
    {
        set_v_pin_value(1);
        set_r_pin_value(0);
    }

    else
    {
        set_v_pin_value(1);
        set_r_pin_value(1);
    }
}

void * menu(void * args)
{
    
    short cs = 0;
    float i_temp;

    if(p_controller == 0 && TR == 0)
    {
        printf("================== START ==================\n  1 -> Enter new internal temp\n  2 -> Use TR as internal temperature\n===========================================\n");
        scanf("%hd", &cs);
        switch(cs)
        {
            case 1:
                printf("Enter the new internal temperature -> ");
                scanf("%f", &i_temp);
                TR = i_temp;
                p_controller = 0;
                printf("\n");
            break;

            case 2:
                printf("System temperature is now TR\n");
                p_controller = 1;
                printf("\n");
            break;

            default:
            break;
        }
        clear_outputs();
    }

    else
    {
        printf("========== INTERACTIVE MENU ==========\n  1 -> Print temperature\n  2 -> Enter new internal temp\n  3 -> Enter new hysteresis value\n  4 -> Use TR as internal temperature\n======================================\n");
        printf("-> ");
        scanf("%hd", &cs);
        printf("\n");
        switch (cs)
       {
            case 1:
                printf("TI -> %0.2f\nTE -> %0.2f\nTR -> %0.2f\n\n", TI, TE, TR);
                char q;
                printf("PRESS 'q' TO RETURN TO MENU\n");
                while(1)
                {
                    scanf("%c", &q);
                    printf("\n");
                    pause();

                    if(q == 'q')
                    {
                        clear_outputs();
                        break;
                    }
                }
    
            break;

            case 2:
                printf("Enter the new internal temperature -> ");
                scanf("%f", &i_temp);
                TR = i_temp;
                p_controller = 0;
                printf("\n");
                clear_outputs();
            break;

            case 3:
                printf("Enter the new hysteresis value -> ");
                scanf("%hd", &hist);
                printf("\n");
                clear_outputs();
            break;

            case 4:
                printf("System temperature is now TR\n");
                p_controller = 1;
                printf("\n");
                clear_outputs();
            break;

            default:
            break;
       }
    }
}
  
void * lcd()
{
    lcd_init(); 

    sleep(1);
    
    while(1)
    {
        line_position(LINE1);
        write_string("TI:");
        write_float(TI);
        write_string(" TE:");
        write_float(TE);

        line_position(LINE2);
        write_string("TR:");
        write_float(TR);
    }
}

void format_time(char *date_string, char *hour_string) 
{
    time_t rawtime;
    struct tm * tm_data;

    time(&rawtime);
    tm_data = localtime(&rawtime);

    sprintf(hour_string, "%02d:%02d:%02d", tm_data->tm_hour, tm_data->tm_min, tm_data->tm_sec);

    sprintf(date_string, "%02d-%02d-%04d", tm_data->tm_mday, tm_data->tm_mon+1, 1900+tm_data->tm_year);
}

void clear_outputs() {
    #if defined _WIN32
        system("cls");
    #elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
        system("clear");
    #elif defined (__APPLE__)
        system("clear");
    #endif
}