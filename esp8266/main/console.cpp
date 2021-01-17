#include <cmath>
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "motor.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

struct
{
    struct arg_int* power;
    struct arg_end* end;
} set_power_args;

struct
{
    struct arg_int* no_rotation_timeout;
    struct arg_end* end;
} set_no_rotation_timeout_args;

struct
{
    struct arg_int* degrees;
    struct arg_end* end;
} rotate_args;

int motor_power = 500;
int no_rotation_timeout = 275;
int locked_position = 0;
int unlocked_position = 0;

static int set_power(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &set_power_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, set_power_args.end, argv[0]);
        return 1;
    }
    const auto pwr = set_power_args.power->ival[0];
    if (pwr < 0 || pwr > 1000)
    {
        printf("Invalid power value\n");
        return 1;
    }
    motor_power = pwr;
    printf("Power set to %d\n", motor_power);
    return 0;
}

static int set_no_rotation_timeout(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &set_no_rotation_timeout_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, set_no_rotation_timeout_args.end, argv[0]);
        return 1;
    }
    const auto to = set_no_rotation_timeout_args.no_rotation_timeout->ival[0];
    if (to < 0 || to > 1000)
    {
        printf("Invalid no_rotation_timeout value\n");
        return 1;
    }
    no_rotation_timeout = to;
    printf("No rotation timeout set to %d\n", no_rotation_timeout);
    return 0;
}

bool do_calibration(bool fwd)
{
    const auto pwr = fwd ? CALIBRATE_POWER : -CALIBRATE_POWER;
    printf("- %s (%d)...\n", fwd ? "locking" : "unlocking", pwr);
    auto start_tick = xTaskGetTickCount();
    const auto start_pos = encoder_position.load();
    bool engaged = false;
    motor->drive(pwr);
    const int MAX_TOTAL_PULSES = 2.5 * Encoder::STEPS_PER_REVOLUTION;
    const int MAX_ENGAGE_TIME = 2000; // ms
    int last_encoder_pos = std::numeric_limits<int>::min();
    int last_position_change = 0;
    while (1)
    {
        const auto now = xTaskGetTickCount();
        const auto pos = encoder_position.load();
        if (pos != last_encoder_pos)
        {
            /*printf("%ld Encoder %d\n", (long) now, pos);
              fflush(stdout);*/
        }
        if (!engaged)
        {
            if (now - start_tick > MAX_ENGAGE_TIME/portTICK_PERIOD_MS)
            {
                motor->brake();
                printf("\nEngage timeout!\n");
                led_duty_cycle = 10;
                led_period = 40;
                return false;
            }
            if (pos != start_pos)
            {
                engaged = true;
                printf("\n%ld Engaged\n", (long) now);
                last_position_change = now;
            }
        }
        else
        {
            if (pos != last_encoder_pos)
            {
                last_position_change = now;
            }
            else if (now - last_position_change > no_rotation_timeout)
            {
                motor->brake();
                printf("now %ld last change %ld\n", (long) now, (long) last_position_change);
                printf("\nHit limit\n");
                led_period = LED_DEFAULT_PERIOD;
                led_duty_cycle = LED_DEFAULT_DUTY_CYCLE;
                return true;
            }
        }
        last_encoder_pos = pos;
        if (fabs(pos - start_pos) > MAX_TOTAL_PULSES)
        {
            motor->brake();
            printf("\nTimeout!\n");
            led_period = 10;
            led_duty_cycle = 10;
            return false;
        }
    }
}

static int calibrate(int argc, char** argv)
{
    printf("Calibrating...\n");

    // We assume that current state is unlocked, so first step is to lock
    led_period = 1;
    led_duty_cycle = 50;
    bool ok = do_calibration(true);
    motor->brake();
    if (!ok)
        return 1;

    // Back off
    motor->brake();
    vTaskDelay(BACKOFF_TICKS);
    printf("Back off from %d\n", encoder_position.load());
    motor->drive(-CALIBRATE_POWER);
    vTaskDelay(2*BACKOFF_TICKS);
    motor->brake();
    printf("Backed off to %d\n", encoder_position.load());
    vTaskDelay(BACKOFF_TICKS);

    locked_position = encoder_position.load();
    
    // Now unlock
    ok = do_calibration(false);
    motor->brake();
      if (!ok)
        return 1;

    // Back off
    vTaskDelay(BACKOFF_TICKS);
    printf("Back off from %d\n", encoder_position.load());
    motor->drive(CALIBRATE_POWER);
    vTaskDelay(2*BACKOFF_TICKS);
    motor->brake();
    printf("Backed off to %d\n", encoder_position.load());

    unlocked_position = encoder_position.load();
    
    led_period = LED_DEFAULT_PERIOD;
    led_duty_cycle = LED_DEFAULT_DUTY_CYCLE;

    printf("Locked %d Unlocked %d\n", locked_position, unlocked_position);
    
    return 0;
}

// Encoder lag:
// Power  Lag [ms]
//  400   1800
//  500   1400
//  800    800
void wait(int ms)
{
    const int slice = 10;
    int k = 0;
    const auto start_pos = encoder_position.load();
    const auto start_tick = xTaskGetTickCount();
    bool moved = false;
    TickType_t moved_tick = 0;
    for (int n = 0; n < ms/slice; ++n)
    {
        const auto pos = encoder_position.load();
        if (!moved && pos != start_pos)
        {
            moved_tick = xTaskGetTickCount();
            moved = true;
        }
        vTaskDelay(slice/portTICK_PERIOD_MS);
        if (++k > 10)
        {
            printf("Encoder %d\r", pos);
            fflush(stdout);
            k = 0;
        }
    }
    printf("\n%d ticks to engage\n", int(moved_tick - start_tick));
    printf("Pulses %d\n", int(encoder_position.load() - start_pos));
}

// Power  Pulses in 5s
//  400    50
//  500    65
//  800   120
int rotate(int argc, char** argv)
{
    if (argc < 1)
    {
        printf("Missing argument\n");
        return 1;
    }
    // arg_parser believes everything starting with - must be an option
    const auto degrees = atoi(argv[1]);
    if (degrees < -1000 || degrees > 1000)
    {
        printf("Invalid degrees value\n");
        return 1;
    }

    const int sign = degrees < 0 ? -1 : 1;
    const int abs_degrees = abs(degrees);
    const int needed_pulses = abs_degrees * Encoder::STEPS_PER_REVOLUTION / 360;
    
    const int MAX_TIME = 10000; // ms
    const auto start_pos = encoder_position.load();
    const auto start_tick = xTaskGetTickCount();
    motor->drive(sign * motor_power);
    const int slice = 10;
    int k = 0;
    while (1)
    {
        const auto pos = encoder_position.load();
        if (sign*(pos - start_pos) >= needed_pulses)
        {
            break;
        }
        vTaskDelay(slice/portTICK_PERIOD_MS);
        if (++k > 10)
        {
            printf("Encoder %d\n", pos);
            k = 0;
        }
        if (xTaskGetTickCount() - start_tick > MAX_TIME/portTICK_PERIOD_MS)
        {
            printf("Timeout!\n");
            return 1;
        }
    }
    motor->brake();
    return 0;
}

bool rotate_to(bool fwd, int position)
{
    const auto start_pos = encoder_position.load();
    if ((fwd && (position < start_pos)) ||
        (!fwd && (position > start_pos)))
    {
        printf("Impossible: Forward %d, current position %d, requested %d\n",
               fwd, start_pos, position);
        return false;
    }
    const int MAX_TOTAL_PULSES = 2.5 * Encoder::STEPS_PER_REVOLUTION;
    const int steps_needed = fabs(position - start_pos);
    if (steps_needed > MAX_TOTAL_PULSES)
    {
        printf("Impossible: Distance %d\n", steps_needed);
        return false;
    }

    const auto start_tick = xTaskGetTickCount();
    bool engaged = false;
    motor->drive(fwd ? motor_power : -motor_power);
    const int MAX_ENGAGE_TIME = 2000; // ms
    int last_encoder_pos = std::numeric_limits<int>::min();
    int last_position_change = 0;
    while (1)
    {
        const auto now = xTaskGetTickCount();
        const auto pos = encoder_position.load();
        if (!engaged)
        {
            if (now - start_tick > MAX_ENGAGE_TIME/portTICK_PERIOD_MS)
            {
                motor->brake();
                printf("\nEngage timeout!\n");
                led_duty_cycle = 10;
                led_period = 40;
                return false;
            }
            if (pos != start_pos)
            {
                engaged = true;
                printf("\n%ld Engaged\n", (long) now);
                last_position_change = now;
            }
        }
        else
        {
            if (pos != last_encoder_pos)
            {
                last_position_change = now;
            }
            else if (now - last_position_change > no_rotation_timeout)
            {
                motor->brake();
                printf("now %ld last change %ld\n", (long) now, (long) last_position_change);
                printf("\nHit limit\n");
                return false;
            }
        }
        last_encoder_pos = pos;
        const auto steps_total = fabs(pos - start_pos);
        if (steps_total > MAX_TOTAL_PULSES)
        {
            motor->brake();
            printf("\nTimeout!\n");
            return false;
        }
        if (steps_total >= steps_needed)
            break;
    }

    motor->brake();
    return true;
}

static int lock(int, char**)
{
    led_period = 1;
    led_duty_cycle = 50;
    if (!rotate_to(true, locked_position))
        return 1;
    // Back off
    vTaskDelay(BACKOFF_TICKS);
    printf("Back off (%d)\n", motor_power);
    motor->drive(-motor_power);
    vTaskDelay(BACKOFF_TICKS);
    motor->brake();
    printf("Backed off\n");
    led_period = LED_DEFAULT_PERIOD;
    led_duty_cycle = LED_DEFAULT_DUTY_CYCLE;
    printf("done\n");
    return 0;
}

static int unlock(int, char**)
{
    led_period = 1;
    led_duty_cycle = 10;
    if (!rotate_to(false, unlocked_position))
        return 1;
    // Back off
    vTaskDelay(BACKOFF_TICKS);
    printf("Back off (%d)\n", motor_power);
    motor->drive(motor_power);
    vTaskDelay(BACKOFF_TICKS);
    motor->brake();
    printf("Backed off\n");
    led_period = LED_DEFAULT_PERIOD;
    led_duty_cycle = LED_DEFAULT_DUTY_CYCLE;
    printf("done\n");
    return 0;
}

static int read_encoder(int, char**)
{
    for (int n = 0; n < 100; ++n)
    {
        vTaskDelay(500/portTICK_PERIOD_MS);
        printf("Encoder %d\n", encoder_position.load());
        // if (fgetc(stdin) != EOF)
        //     break;
    }
    printf("done\n");
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

    set_power_args.power = arg_int1(NULL, NULL, "<pwr>", "Motor power (0-1000)");
    set_power_args.end = arg_end(2);
    const esp_console_cmd_t set_power_cmd = {
        .command = "set_power",
        .help = "Set motor power",
        .hint = nullptr,
        .func = &set_power,
        .argtable = &set_power_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&set_power_cmd));

    set_no_rotation_timeout_args.no_rotation_timeout = arg_int1(NULL, NULL, "<ms>", "No rotation timeout");
    set_no_rotation_timeout_args.end = arg_end(2);
    const esp_console_cmd_t set_no_rotation_timeout_cmd = {
        .command = "set_no_rotation_timeout",
        .help = "Set no rotation timeout",
        .hint = nullptr,
        .func = &set_no_rotation_timeout,
        .argtable = &set_no_rotation_timeout_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&set_no_rotation_timeout_cmd));

    rotate_args.degrees = arg_int1(NULL, NULL, "<degrees>", "Degrees");
    rotate_args.end = arg_end(2);
    const esp_console_cmd_t rotate_cmd = {
        .command = "rotate",
        .help = "Rotate degrees",
        .hint = nullptr,
        .func = &rotate,
        .argtable = &rotate_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&rotate_cmd));

    const esp_console_cmd_t read_encoder_cmd = {
        .command = "read_encoder",
        .help = "Read encoder",
        .hint = nullptr,
        .func = &read_encoder,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&read_encoder_cmd));

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

    printf("Danalock " VERSION " ready: %d\n", (int) portTICK_PERIOD_MS);

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
