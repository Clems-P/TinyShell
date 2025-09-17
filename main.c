#include <stdio.h> // For getchar() in this simulation example
#include "tinyshell.h"

// --- UART interface function implementations (to adapt) ---
// For this PC test, we use printf. On an MCU, use your UART driver.
void shell_putchar(char c) {
    // Replace with your function, e.g.: HAL_UART_Transmit(&huart1, (uint8_t*)&c, 1, HAL_MAX_DELAY);
    putchar(c);
    fflush(stdout);
}

void shell_puts(const char *s) {
    // Replace with your function, e.g.: HAL_UART_Transmit(&huart1, (uint8_t*)s, strlen(s), HAL_MAX_DELAY);
    printf("%s", s);
    fflush(stdout);
}


// --- main function ---
int main() {
    shell_init();

    // Main application loop
    while (1) {
        char c = getchar();
        shell_process_char(c);
    }

    return 0;
}