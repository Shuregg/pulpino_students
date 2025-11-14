
#define __riscv__
#define LED_DELAY 1000000

//#include <spi.h>
#include <gpio.h>
#include <uart.h>
//#include <utils.h>
#include <pulpino.h>


int main()
{
  uart_set_cfg(0, 325); // 9600 baud UART, no parity (50MHz CPU)

  uart_send("Hello world!\n", 13); // 13 is a number of chars sent: 12 + "\n" 
  uart_wait_tx_done();

  set_pin_function(8, FUNC_GPIO);
  set_gpio_pin_direction(8, DIR_OUT);

  set_gpio_pin_value(8, 0);

  while(1) {

  
    for (int i = 0; i < LED_DELAY; i++) {
      //wait some time
      #ifdef __riscv__
          asm volatile ("nop");
      #else
          asm volatile ("l.nop");
      #endif
    }

    set_gpio_pin_value(8, 1);

    for (int i = 0; i < LED_DELAY; i++) {
      //wait some time
      #ifdef __riscv__
          asm volatile ("nop");
      #else
          asm volatile ("l.nop");
      #endif
    }

    set_gpio_pin_value(8, 0);
    
  }
}
