#include <stdio.h>
#include "pico/stdlib.h"

#include "pico/cyw43_arch.h" 
#include "pico/stdlib.h"  

int LED_DELAY_MS = 250;

void blink_times(int count)
{
    for (int i = 0; i < count; i++)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
        sleep_ms(LED_DELAY_MS);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
        sleep_ms(LED_DELAY_MS);
    }
}

int main() {
    stdio_init_all();

    cyw43_arch_init();

    printf("Available commands are 1, 2, 3, + and -\n");

    while (true) {
        if (uart_is_readable(uart0)) {
            int ch = uart_getc(uart0);  // Read character from UART
            switch (ch) {
                case '1':
                    blink_times(1);
                    break;
                case '2':
                    blink_times(2);
                    break;
                case '3':
                    blink_times(3);
                    break;
                case '+':
                    LED_DELAY_MS += 50;
                    printf("Delay increased.\n");
                    break;
                case '-':
                    LED_DELAY_MS -= 50;
                    printf("Delay decreased.\n");
                    break;
                default:
                    break;
            }

            uart_putc(uart0, ch);  // Echo the character back to UART
            printf("Received: %c\n", ch);  // Send it to the USB connection (PC)
        }
        sleep_ms(100);  // Delay to avoid flooding the serial connection
    }
}
