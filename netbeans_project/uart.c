#include "ets_sys.h"
#include "osapi.h"
#include "uart.h"

#define UART0   0

// UartDev is defined and initialized in rom code.
extern UartDevice UartDev;

//configura UART0 como debug
void ICACHE_FLASH_ATTR uart0_init(void)
{
    UartDev.baut_rate = BIT_RATE_115200;
    
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);

    uart_div_modify(UART0, UART_CLK_FREQ / (UartDev.baut_rate));

    WRITE_PERI_REG(UART_CONF0(UART0),    UartDev.exist_parity
                   | UartDev.parity
                   | (UartDev.stop_bits << UART_STOP_BIT_NUM_S)
                   | (UartDev.data_bits << UART_BIT_NUM_S));


    //clear rx and tx fifo,not ready
    SET_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);
}
