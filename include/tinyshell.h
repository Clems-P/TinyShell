#ifndef TINY_SHELL_H
#define TINY_SHELL_H

/**
 * @brief Initialise le shell. Doit être appelée une fois au démarrage.
 */
void shell_init(void);

/**
 * @brief Fonction principale à appeler pour chaque caractère reçu de l'UART.
 * @param c Le caractère reçu.
 */
void shell_process_char(char c);

// --- Fonctions à implémenter par l'utilisateur ---
// Vous devez fournir le corps de ces deux fonctions pour lier le shell
// à votre driver UART.

/**
 * @brief Fonction d'interface pour envoyer un seul caractère via l'UART.
 * @param c Le caractère à envoyer.
 */
void shell_putchar(char c);

/**
 * @brief Fonction d'interface pour envoyer une chaîne de caractères via l'UART.
 * @param s La chaîne à envoyer.
 */
void shell_puts(const char *s);


#endif // TINY_SHELL_H