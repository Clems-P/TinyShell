#ifndef TINY_SHELL_H
#define TINY_SHELL_H

/**
 * @brief Initialize the shell. Must be called once at startup.
 */
void shell_init(void);

/**
 * @brief Main function to call for each character received from UART.
 * @param c The received character.
 */
void shell_process_char(char c);

// --- Functions to be implemented by the user ---
// You must provide the implementation of these two functions to link the shell
// to your UART driver.

/**
 * @brief Interface function to send a single character via UART.
 * @param c The character to send.
 */
void shell_putchar(char c);

/**
 * @brief Interface function to send a string via UART.
 * @param s The string to send.
 */
void shell_puts(const char *s);


#endif // TINY_SHELL_H