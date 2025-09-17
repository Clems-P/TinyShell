#include <stdio.h> // Pour getchar() dans cet exemple de simulation
#include "src/tinyshell.h"

// --- Implémentation des fonctions d'interface UART (à adapter) ---
// Pour ce test sur PC, on utilise printf. Sur un MCU, utilisez votre driver UART.
void shell_putchar(char c) {
    // Remplacez par votre fonction, par ex: HAL_UART_Transmit(&huart1, (uint8_t*)&c, 1, HAL_MAX_DELAY);
    putchar(c);
    fflush(stdout);
}

void shell_puts(const char *s) {
    // Remplacez par votre fonction, par ex: HAL_UART_Transmit(&huart1, (uint8_t*)s, strlen(s), HAL_MAX_DELAY);
    printf("%s", s);
    fflush(stdout);
}


// --- Fonction main ---
int main() {
    shell_init();

    // Boucle principale de l'application
    while (1) {
        char c = getchar();
        shell_process_char(c);
    }

    return 0;
}