// Implementation file: mini_shell.c

#include "tinyshell.h"
#include <string.h> // For strcmp, strcpy, strtok
#include <stdlib.h> // For atoi
#include <stdio.h>  // For snprintf (optional, for complex commands)

// --- Shell Configuration ---
#define SHELL_PROMPT        "> "
#define SHELL_BUFFER_SIZE   64  // Max command size
#define SHELL_HISTORY_SIZE  10  // Number of commands in history
#define SHELL_MAX_ARGS      8   // Max number of arguments (including command)

// --- Static buffers ---
static char line_buffer[SHELL_BUFFER_SIZE];
static int  buffer_index = 0;

static char history[SHELL_HISTORY_SIZE][SHELL_BUFFER_SIZE];
static int  history_write_index = 0;
static int  history_read_index = 0;

// For escape sequence handling (arrows)
typedef enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_ESC_BRACKET
} escape_state_t;
static escape_state_t escape_state = STATE_NORMAL;


// --- Command callback declarations ---
// Each command function must have this signature.
typedef void (*shell_command_callback_t)(int argc, char *argv[]);

// Example command functions
static void cmd_help(int argc, char *argv[]);
static void cmd_led(int argc, char *argv[]);
static void cmd_status(int argc, char *argv[]);

// --- Command database (the central point) ---
// Here you associate a keyword with a function.
typedef struct {
    const char* command;
    shell_command_callback_t callback;
    const char* help;
} shell_command_t;

// Static and constant list of available commands.
// Use 'const' to store it in Flash memory on the MCU.
static const shell_command_t commands[] = {
    {"help",   cmd_help,   "Display this help"},
    {"led",    cmd_led,    "Control an LED: led <num> <on|off>"},
    {"status", cmd_status, "Display system status"},
    // Add your new commands here
    {NULL, NULL, NULL} // End of list marker
};


// --- Internal Shell functions ---

/**
 * @brief Clears the current line on the terminal and redraws it.
 */
static void shell_redraw_line() {
    // Clear the line: \r (carriage return) + a series of spaces + \r
    char temp_buf[SHELL_BUFFER_SIZE + sizeof(SHELL_PROMPT) + 1] = {0};
    memset(temp_buf, ' ', sizeof(temp_buf) - 1);
    temp_buf[sizeof(SHELL_PROMPT) + buffer_index] = '\0';
    shell_puts("\r");
    shell_puts(temp_buf);

    // Redraw the prompt and buffer content
    shell_puts("\r" SHELL_PROMPT);
    shell_puts(line_buffer);
}

/**
 * @brief Executes the command present in line_buffer.
 */
static void shell_execute() {
    if (buffer_index == 0) {
        return; // Empty line
    }

    // Add to history
    // Do not store the same command twice in a row
    int prev_index = (history_write_index + SHELL_HISTORY_SIZE - 1) % SHELL_HISTORY_SIZE;
    if (strcmp(history[prev_index], line_buffer) != 0) {
        strcpy(history[history_write_index], line_buffer);
        history_write_index = (history_write_index + 1) % SHELL_HISTORY_SIZE;
    }
    history_read_index = history_write_index;

    // Command parsing (argc, argv)
    char* argv[SHELL_MAX_ARGS];
    int argc = 0;
    
    // Use strtok to split the line. Warning: strtok modifies the buffer.
    char* token = strtok(line_buffer, " ");
    while (token != NULL && argc < SHELL_MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) {
        return;
    }

    // Search and execute the command
    for (int i = 0; commands[i].command != NULL; i++) {
        if (strcmp(argv[0], commands[i].command) == 0) {
            commands[i].callback(argc, argv); // Command found, call it
            return;
        }
    }

    // If we reach here, the command is unknown
    shell_puts("Unknown command: ");
    shell_puts(argv[0]);
    shell_puts("\r\n");
}


// --- Public function implementations ---

void shell_init(void) {
    // Clear buffers just in case
    memset(line_buffer, 0, sizeof(line_buffer));
    buffer_index = 0;
    memset(history, 0, sizeof(history));
    history_write_index = 0;
    history_read_index = 0;

    shell_puts("\r\n--- Mini Shell Initialized ---\r\n");
    shell_puts(SHELL_PROMPT);
}

void shell_process_char(char c) {
    // Handle escape sequences for arrows
    if (escape_state == STATE_NORMAL) {
        if (c == '\x1B') { // Start of a sequence (ESC)
            escape_state = STATE_ESC;
            return;
        }
    } else if (escape_state == STATE_ESC) {
        escape_state = (c == '[') ? STATE_ESC_BRACKET : STATE_NORMAL;
        return;
    } else if (escape_state == STATE_ESC_BRACKET) {
        if (c == 'A') { // UP arrow
            history_read_index = (history_read_index + SHELL_HISTORY_SIZE - 1) % SHELL_HISTORY_SIZE;
            // If we hit an empty entry, stop
            if(history[history_read_index][0] == '\0') {
                 history_read_index = (history_read_index + 1) % SHELL_HISTORY_SIZE;
                 escape_state = STATE_NORMAL;
                 return;
            }
            strcpy(line_buffer, history[history_read_index]);
            buffer_index = strlen(line_buffer);
            shell_redraw_line();
        }
        // Ignore other arrows (B, C, D) for simplicity
        escape_state = STATE_NORMAL;
        return;
    }

    // Normal character processing
    switch (c) {
        case '\r': // Enter
        case '\n':
            shell_puts("\r\n");
            shell_execute();
            // Reset for next command
            memset(line_buffer, 0, sizeof(line_buffer));
            buffer_index = 0;
            shell_puts(SHELL_PROMPT);
            break;

        case '\b': // Backspace
        case 127:  // ASCII DEL (often sent by backspace)
            if (buffer_index > 0) {
                buffer_index--;
                line_buffer[buffer_index] = '\0';
                shell_puts("\b \b"); // Erase character on terminal
            }
            break;

        default: // Standard character
            if (buffer_index < (SHELL_BUFFER_SIZE - 1) && c >= 32 && c <= 126) {
                line_buffer[buffer_index++] = c;
                line_buffer[buffer_index] = '\0';
                shell_putchar(c); // Echo character
            }
            break;
    }
}


// --- Command callback implementations ---

static void cmd_help(int argc, char *argv[]) {
    shell_puts("Available commands:\r\n");
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
        shell_puts("Error: state must be 'on' or 'off'\r\n");
        return;
    }

    char temp_buf[40];
    snprintf(temp_buf, sizeof(temp_buf), "Action: Set LED %d to state %d\r\n", led_num, state);
    shell_puts(temp_buf);
    
    // HERE: Call your hardware function, e.g.: BSP_LED_Set(led_num, state);
}

static void cmd_status(int argc, char *argv[]) {
    shell_puts("System status: OK\r\n");
    shell_puts("CPU temperature: 42 deg C\r\n");
    // HERE: Call your functions to get real values.
}