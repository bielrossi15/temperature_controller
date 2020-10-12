#include <stdio.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include <string.h>
#include "uart.h"

#define MESSAGE_SIZE 5

float * f_buf;
int uart_filestream = -1;

int tx_rx(char rx_sig_1, float * f_buf);

int call_uart(int ttype, float * f_buf) 
{
    uart_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY); //Open in non blocking read/write mode
    if (uart_filestream == -1)
    {
        printf("Error - UART could not be initialized.\n");
    }
    
    struct termios options;
    tcgetattr(uart_filestream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;     // < Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart_filestream, TCIFLUSH);
    tcsetattr(uart_filestream, TCSANOW, &options);

    if(ttype == 0) // get internal temperature
    {
        char rx_sig_1 = 0xA1;
        if(tx_rx(rx_sig_1, f_buf) < 0)
            return -1;
    }
    
    else if(ttype == 1) // reference temperature
    {
        char rx_sig_1 = 0xA2;
        if(tx_rx(rx_sig_1, f_buf) < 0)
            return -1;
    }

    else 
    {
        printf("Invalid option\n");
        return -1;
    }

    return 0;
}


int tx_rx(char rx_sig_1, float * f_buf)
{   
    char rx_sig[MESSAGE_SIZE];

    rx_sig[0] = rx_sig_1;
    rx_sig[1] = 3;
    rx_sig[2] = 6;
    rx_sig[3] = 9;
    rx_sig[4] = 3;

    int count = write(uart_filestream, (const void*) rx_sig, sizeof(rx_sig));
    if (count < 0)
    {
        printf("UART TX error\n");
        return -1;
    }

    usleep(30000);

    int rx_length = read(uart_filestream, f_buf, sizeof(float));
    if (rx_length < 0)
    {
        printf("Reading error.\n"); //An error occured (will occur if there are no bytes)
        return -1;
    }

    return 0;
}
