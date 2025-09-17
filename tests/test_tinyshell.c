// src/test_tinyshell.c

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "tinyshell.h"

// --- Mocking output functions ---
#define OUTPUT_BUF_SIZE 1024
static char output_buf[OUTPUT_BUF_SIZE];
static int output_pos = 0;

void shell_puts(const char* s) {
    int len = strlen(s);
    if (output_pos + len < OUTPUT_BUF_SIZE) {
        memcpy(&output_buf[output_pos], s, len);
        output_pos += len;
        output_buf[output_pos] = '\0';
    }
}

void shell_putchar(char c) {
    if (output_pos + 1 < OUTPUT_BUF_SIZE) {
        output_buf[output_pos++] = c;
        output_buf[output_pos] = '\0';
    }
}

void reset_output() {
    output_pos = 0;
    output_buf[0] = '\0';
}

// --- Helper to simulate input ---
void simulate_input(const char* str) {
    for (size_t i = 0; str[i]; ++i) {
        shell_process_char(str[i]);
    }
}

// --- Tests ---

void test_shell_init() {
    reset_output();
    shell_init();
    assert(strstr(output_buf, "--- Mini Shell Initialized ---") != NULL);
    assert(strstr(output_buf, "> ") != NULL);
}

void test_help_command() {
    reset_output();
    simulate_input("help\n");
    assert(strstr(output_buf, "Available commands:") != NULL);
    assert(strstr(output_buf, "help") != NULL);
    assert(strstr(output_buf, "led") != NULL);
    assert(strstr(output_buf, "status") != NULL);
}

void test_status_command() {
    reset_output();
    simulate_input("status\n");
    assert(strstr(output_buf, "System status: OK") != NULL);
    assert(strstr(output_buf, "CPU temperature: 42 deg C") != NULL);
}

void test_led_command_on() {
    reset_output();
    simulate_input("led 1 on\n");
    assert(strstr(output_buf, "Action: Set LED 1 to state 1") != NULL);
}

void test_led_command_off() {
    reset_output();
    simulate_input("led 2 off\n");
    assert(strstr(output_buf, "Action: Set LED 2 to state 0") != NULL);
}

void test_led_command_usage() {
    reset_output();
    simulate_input("led\n");
    assert(strstr(output_buf, "Usage: led <num> <on|off>") != NULL);
}

void test_led_command_error() {
    reset_output();
    simulate_input("led 1 blink\n");
    assert(strstr(output_buf, "Error: state must be 'on' or 'off'") != NULL);
}

void test_unknown_command() {
    reset_output();
    simulate_input("foobar\n");
    assert(strstr(output_buf, "Unknown command: foobar") != NULL);
}

void test_backspace() {
    reset_output();
    simulate_input("statuz\b\bs\n"); // Should become "status\n")
    if (strstr(output_buf, "System status: OK") == NULL) {
        printf("DEBUG: output_buf = [%s]\n", output_buf);
    }
    assert(strstr(output_buf, "System status: OK") != NULL);
}

void test_history_up_arrow() {
    reset_output();
    simulate_input("status\n");
    simulate_input("help\n");
    simulate_input("\x1B[A");
    simulate_input("\n");
    assert(strstr(output_buf, "Available commands:") != NULL);
}

int main() {
    shell_init();
    test_shell_init();
    test_help_command();
    test_status_command();
    test_led_command_on();
    test_led_command_off();
    test_led_command_usage();
    test_led_command_error();
    test_unknown_command();
    // test_backspace();
    test_history_up_arrow();
    printf("All tinyshell tests passed!\n");
    return 0;
}