// Fichier d'implémentation : mini_shell.c

#include "tinyshell.h"
#include <string.h> // Pour strcmp, strcpy, strtok
#include <stdlib.h> // Pour atoi
#include <stdio.h>  // Pour snprintf (optionnel, pour des commandes complexes)

// --- Configuration du Shell ---
#define SHELL_PROMPT        "> "
#define SHELL_BUFFER_SIZE   64  // Taille max d'une commande
#define SHELL_HISTORY_SIZE  10  // Nombre de commandes dans l'historique
#define SHELL_MAX_ARGS      8   // Nombre max d'arguments (commande incluse)

// --- Tampons statiques ---
static char line_buffer[SHELL_BUFFER_SIZE];
static int  buffer_index = 0;

static char history[SHELL_HISTORY_SIZE][SHELL_BUFFER_SIZE];
static int  history_write_index = 0;
static int  history_read_index = 0;

// Pour la gestion des séquences d'échappement (flèches)
typedef enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_ESC_BRACKET
} escape_state_t;
static escape_state_t escape_state = STATE_NORMAL;


// --- Déclaration des Callbacks de commandes ---
// Chaque fonction de commande doit avoir cette signature.
typedef void (*shell_command_callback_t)(int argc, char *argv[]);

// Exemples de fonctions de commande
static void cmd_help(int argc, char *argv[]);
static void cmd_led(int argc, char *argv[]);
static void cmd_status(int argc, char *argv[]);

// --- Base de données des commandes (le point central) ---
// C'est ici que vous associez un mot-clé à une fonction.
typedef struct {
    const char* command;
    shell_command_callback_t callback;
    const char* help;
} shell_command_t;

// Liste statique et constante des commandes disponibles.
// Mettez-la en 'const' pour la stocker en mémoire Flash sur le MCU.
static const shell_command_t commands[] = {
    {"help",   cmd_help,   "Affiche cette aide"},
    {"led",    cmd_led,    "Controle une LED: led <num> <on|off>"},
    {"status", cmd_status, "Affiche le statut du systeme"},
    // Ajoutez vos nouvelles commandes ici
    {NULL, NULL, NULL} // Marqueur de fin de liste
};


// --- Fonctions internes du Shell ---

/**
 * @brief Efface la ligne actuelle sur le terminal et la redessine.
 */
static void shell_redraw_line() {
    // Efface la ligne: \r (retour chariot) + une série d'espaces + \r
    char temp_buf[SHELL_BUFFER_SIZE + sizeof(SHELL_PROMPT) + 1] = {0};
    memset(temp_buf, ' ', sizeof(temp_buf) - 1);
    temp_buf[sizeof(SHELL_PROMPT) + buffer_index] = '\0';
    shell_puts("\r");
    shell_puts(temp_buf);

    // Redessine le prompt et le contenu du buffer
    shell_puts("\r" SHELL_PROMPT);
    shell_puts(line_buffer);
}

/**
 * @brief Exécute la commande présente dans line_buffer.
 */
static void shell_execute() {
    if (buffer_index == 0) {
        return; // Ligne vide
    }

    // Ajout à l'historique
    // On ne stocke pas deux fois de suite la même commande
    int prev_index = (history_write_index + SHELL_HISTORY_SIZE - 1) % SHELL_HISTORY_SIZE;
    if (strcmp(history[prev_index], line_buffer) != 0) {
        strcpy(history[history_write_index], line_buffer);
        history_write_index = (history_write_index + 1) % SHELL_HISTORY_SIZE;
    }
    history_read_index = history_write_index;

    // Parsing de la commande (argc, argv)
    char* argv[SHELL_MAX_ARGS];
    int argc = 0;
    
    // Utilise strtok pour découper la ligne. Attention: strtok modifie le buffer.
    char* token = strtok(line_buffer, " ");
    while (token != NULL && argc < SHELL_MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) {
        return;
    }

    // Recherche et exécution de la commande
    for (int i = 0; commands[i].command != NULL; i++) {
        if (strcmp(argv[0], commands[i].command) == 0) {
            commands[i].callback(argc, argv); // Commande trouvée, on l'appelle
            return;
        }
    }

    // Si on arrive ici, la commande est inconnue
    shell_puts("Commande inconnue: ");
    shell_puts(argv[0]);
    shell_puts("\r\n");
}


// --- Implémentation des fonctions publiques ---

void shell_init(void) {
    // Nettoie les buffers au cas où
    memset(line_buffer, 0, sizeof(line_buffer));
    buffer_index = 0;
    memset(history, 0, sizeof(history));
    history_write_index = 0;
    history_read_index = 0;

    shell_puts("\r\n--- Mini Shell Initialise ---\r\n");
    shell_puts(SHELL_PROMPT);
}

void shell_process_char(char c) {
    // Gestion des séquences d'échappement pour les flèches
    if (escape_state == STATE_NORMAL) {
        if (c == '\x1B') { // Début d'une séquence (ESC)
            escape_state = STATE_ESC;
            return;
        }
    } else if (escape_state == STATE_ESC) {
        escape_state = (c == '[') ? STATE_ESC_BRACKET : STATE_NORMAL;
        return;
    } else if (escape_state == STATE_ESC_BRACKET) {
        if (c == 'A') { // Flèche HAUT
            history_read_index = (history_read_index + SHELL_HISTORY_SIZE - 1) % SHELL_HISTORY_SIZE;
            // Si on tombe sur une entrée vide, on s'arrête
            if(history[history_read_index][0] == '\0') {
                 history_read_index = (history_read_index + 1) % SHELL_HISTORY_SIZE;
                 escape_state = STATE_NORMAL;
                 return;
            }
            strcpy(line_buffer, history[history_read_index]);
            buffer_index = strlen(line_buffer);
            shell_redraw_line();
        }
        // Ignorer les autres flèches (B, C, D) pour la simplicité
        escape_state = STATE_NORMAL;
        return;
    }

    // Traitement des caractères normaux
    switch (c) {
        case '\r': // Entrée
        case '\n':
            shell_puts("\r\n");
            shell_execute();
            // Réinitialise pour la prochaine commande
            memset(line_buffer, 0, sizeof(line_buffer));
            buffer_index = 0;
            shell_puts(SHELL_PROMPT);
            break;

        case '\b': // Backspace
        case 127:  // ASCII DEL (souvent envoyé par backspace)
            if (buffer_index > 0) {
                buffer_index--;
                line_buffer[buffer_index] = '\0';
                shell_puts("\b \b"); // Efface le caractère sur le terminal
            }
            break;

        default: // Caractère standard
            if (buffer_index < (SHELL_BUFFER_SIZE - 1) && c >= 32 && c <= 126) {
                line_buffer[buffer_index++] = c;
                line_buffer[buffer_index] = '\0';
                shell_putchar(c); // Echo du caractère
            }
            break;
    }
}


// --- Implémentation des callbacks ---

static void cmd_help(int argc, char *argv[]) {
    shell_puts("Liste des commandes disponibles:\r\n");
    for (int i = 0; commands[i].command != NULL; i++) {
        shell_puts("  ");
        shell_puts(commands[i].command);
        shell_puts("\t- ");
        shell_puts(commands[i].help);
        shell_puts("\r\n");
    }
}

static void cmd_led(int argc, char *argv[]) {
    if (argc != 3) {
        shell_puts("Usage: led <num> <on|off>\r\n");
        return;
    }

    int led_num = atoi(argv[1]);
    int state = -1;

    if (strcmp(argv[2], "on") == 0) {
        state = 1;
    } else if (strcmp(argv[2], "off") == 0) {
        state = 0;
    } else {
        shell_puts("Erreur: l'etat doit etre 'on' ou 'off'\r\n");
        return;
    }

    char temp_buf[40];
    snprintf(temp_buf, sizeof(temp_buf), "Action: Allumer LED %d a l'etat %d\r\n", led_num, state);
    shell_puts(temp_buf);
    
    // ICI: Appelez votre fonction hardware, par ex: BSP_LED_Set(led_num, state);
}

static void cmd_status(int argc, char *argv[]) {
    shell_puts("Statut du systeme: OK\r\n");
    shell_puts("Temperature CPU: 42 deg C\r\n");
    // ICI: Appelez vos fonctions pour récupérer les vraies valeurs.
}