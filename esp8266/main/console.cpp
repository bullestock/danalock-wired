#include <stdio.h>
#include <string.h>

#include "defines.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "FreeRTOS.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

static int calibrate(int argc, char** argv)
{
    printf("calibrating...\n");
    return 0;
}

static int lock(int, char**)
{
    printf("locking...\n");
    return 0;
}

static int unlock(int, char**)
{
    printf("unlocking...\n");
    return 0;
}

void initialize_console()
{
    // Disable buffering on stdin
    setvbuf(stdin, NULL, _IONBF, 0);

    // Minicom, screen, idf_monitor send CR when ENTER key is pressed
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    // Move the caret to the beginning of the next line on '\n'
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    uart_config_t uart_config = {
        .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0
    };
    ESP_ERROR_CHECK(uart_param_config((uart_port_t) CONFIG_ESP_CONSOLE_UART_NUM, &uart_config));

    // Install UART driver for interrupt-driven reads and writes
    ESP_ERROR_CHECK(uart_driver_install((uart_port_t) CONFIG_ESP_CONSOLE_UART_NUM,
                                        256, 0, 0, NULL, 0));

    // Tell VFS to use UART driver
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    // Initialize the console
    esp_console_config_t console_config = {
        .max_cmdline_length = 256,
        .max_cmdline_args = 8,
#if CONFIG_LOG_COLORS
        .hint_color = atoi(LOG_COLOR_CYAN),
        .hint_bold = 0
#endif
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    linenoiseSetMultiLine(1);
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);
    linenoiseHistorySetMaxLen(100);
}

extern "C" void console_task(void*)
{
    initialize_console();

    // Register commands
    esp_console_register_help_command();

    const esp_console_cmd_t calibrate_cmd = {
        .command = "calibrate",
        .help = "Calibrate locked/unlocked positions",
        .hint = nullptr,
        .func = &calibrate,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&calibrate_cmd));

    const esp_console_cmd_t lock_cmd = {
        .command = "lock",
        .help = "Lock the door",
        .hint = nullptr,
        .func = &lock,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&lock_cmd));

    const esp_console_cmd_t unlock_cmd = {
        .command = "unlock",
        .help = "Unlock the door",
        .hint = nullptr,
        .func = &unlock,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&unlock_cmd));

    const char* prompt = "";

    printf("Danalock " VERSION " ready\n");

    while (true)
    {
        char* line = linenoise(prompt);
        if (!line)
            continue;

        linenoiseHistoryAdd(line);

        int ret = 0;
        esp_err_t err = esp_console_run(line, &ret);
        switch (err)
        {
        case ESP_ERR_NOT_FOUND:
            printf("Unrecognized command\n");
            break;
        case ESP_ERR_INVALID_ARG:
            // command was empty
            break;
        case ESP_OK:
            if (ret != ESP_OK)
                printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(err));
            break;
        default:
            printf("Internal error: %s\n", esp_err_to_name(err));
            break;
        }
        linenoiseFree(line);
    }
}
