#include <cmath>
#include <stdio.h>
#include <string.h>
#include <string>
#include <utility>

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
#include "nvs.h"
#include "nvs_flash.h"

int verbosity = 0;

void verbose_printf(const char* format, ...)
{
    if (verbosity == 0)
        return;
    printf("%ld ", (long) (xTaskGetTickCount()*portTICK_PERIOD_MS));
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void verbose_wait()
{
    if (verbosity < 2)
        return;
    vTaskDelay(1000);
}

std::pair<int, int> locked_position = std::make_pair(0, 0);
std::pair<int, int> unlocked_position = std::make_pair(0, 0);
bool is_calibrated = false;
enum State {
    // Initial state until calibration
    Unknown,
    // The 'lock' command was successful
    Locked,
    // The 'unlock' command was successful
    Unlocked,
    // The position was changed manually to be neither 'locked' nor 'unlocked'
    ChangedManually,
    // The position was changed manually to be 'locked'
    LockedManually,
    // The position was changed manually to be 'unlocked'
    UnlockedManually
};
State state = Unknown;

static void update_state()
{
    // Check if anybody has tinkered with the knob
    const auto pos = encoder_position.load();
    verbose_printf("update_state: pos %d\n", pos);
    switch (state)
    {
    case Locked:
    case LockedManually:
        // If position is no longer inside the 'locked' interval, someone has fiddled
        if (pos < locked_position.first || pos > locked_position.second)
        {
            verbose_printf("update_state: outside locked_position\n");
            state = ChangedManually;
        }
        break;

    case Unlocked:
    case UnlockedManually:
        // If position is no longer inside the 'unlocked' interval, someone has fiddled
        if (pos < unlocked_position.first || pos > unlocked_position.second)
        {
            verbose_printf("update_state: outside unlocked_position\n");
            state = ChangedManually;
        }
        break;

    default:
        break;
    }

    if (state == ChangedManually)
    {
        // Check if we are now inside either the 'locked' or 'unlocked' interval
        if (pos >= locked_position.first && pos <= locked_position.second)
        {
            verbose_printf("update_state: inside locked_position\n");
            state = LockedManually;
        }
        if (pos >= unlocked_position.first && pos <= unlocked_position.second)
        {
            verbose_printf("update_state: inside unlocked_position\n");
            state = UnlockedManually;
        }
    }
}

struct
{
    struct arg_int* power;
    struct arg_end* end;
} set_power_args;

struct
{
    struct arg_int* degrees;
    struct arg_end* end;
} rotate_args;

struct
{
    struct arg_int* power;
    struct arg_int* milliseconds;
    struct arg_end* end;
} forward_args;

struct
{
    struct arg_int* power;
    struct arg_int* milliseconds;
    struct arg_end* end;
} reverse_args;

struct
{
    struct arg_int* verbosity;
    struct arg_end* end;
} set_verbosity_args;

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
    default_motor_power = pwr;
    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_i32(my_handle, DEFAULT_POWER_KEY, pwr));
    nvs_close(my_handle);    
    printf("OK: power set to %d\n", pwr);
    return 0;
}

static void backoff(int pwr)
{
    vTaskDelay(BACKOFF_MS/portTICK_PERIOD_MS);
    verbose_printf("backoff(): drive\n");
    motor->drive(pwr);
    vTaskDelay(BACKOFF_MS/portTICK_PERIOD_MS);
    verbose_printf("backoff(): brake\n");
    motor->brake();
    vTaskDelay(BACKOFF_MS/portTICK_PERIOD_MS);
}

// true -> lock
bool do_calibration(bool fwd)
{
    const auto pwr = fwd ? -MOTOR_CALIBRATE_POWER : MOTOR_CALIBRATE_POWER;
    verbose_printf("- %s (%d)...\n", fwd ? "locking" : "unlocking", pwr);
    auto start_ms = xTaskGetTickCount()*portTICK_PERIOD_MS;
    verbose_printf("- start %ld\n", (long) start_ms);
    const auto start_pos = encoder_position.load();
    bool engaged = false;
    motor->drive(pwr);
    const int MAX_TOTAL_PULSES = 2.5 * Encoder::STEPS_PER_REVOLUTION;
    int last_encoder_pos = std::numeric_limits<int>::min();
    int last_position_change = 0;
    const int max_engage_ms = motor->get_max_engage_time_ms(pwr);
    const int no_rotation_timeout = motor->get_rotation_timeout_ms(pwr);
    while (1)
    {
        const auto now = xTaskGetTickCount()*portTICK_PERIOD_MS;
        const auto pos = encoder_position.load();
        if (!engaged)
        {
            if (now - start_ms > max_engage_ms)
            {
                printf("\nEngage timeout!\n");
                verbose_printf("- now\n");
                backoff(-pwr);
                led.set_params(10, 100, 40);
                return false;
            }
            if (pos != start_pos)
            {
                engaged = true;
                verbose_printf("Engaged\n");
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
                verbose_printf("Hit limit\n");
                verbose_wait();
                backoff(-pwr);
                verbose_printf("last change %ld\n", (long) last_position_change);
                return true;
            }
        }
        last_encoder_pos = pos;
        if (fabs(pos - start_pos) > MAX_TOTAL_PULSES)
        {
            backoff(-pwr);
            printf("\nTimeout!\n");
            led.set_params(10, 100, 10);
            return false;
        }
    }
}

static int calibrate(int argc, char** argv)
{
    state = Unknown;
    
    verbose_printf("Calibrating...\n");

    // We assume that current state is unlocked, so first step is to lock
    led.set_params(50, 100, 1);
    bool ok = do_calibration(true);
    motor->brake();
    if (!ok)
        return 0;

    // Use this position as zero
    reset_encoder.store(true);
    locked_position.first = 0;
    
    // Back off
    backoff(MOTOR_CALIBRATE_POWER);
    verbose_printf("Backed off to %d\n", encoder_position.load());

    locked_position.second = encoder_position.load();
    
    // Now unlock
    ok = do_calibration(false);
    motor->brake();
    if (!ok)
        return 0;
    unlocked_position.second = encoder_position.load();

    // Back off
    verbose_printf("Back off from %d\n", encoder_position.load());
    backoff(-MOTOR_CALIBRATE_POWER);
    verbose_printf("Backed off to %d\n", encoder_position.load());

    unlocked_position.first = encoder_position.load();
    
    led.set_params(LED_DEFAULT_DUTY_CYCLE_NUM,
                   LED_DEFAULT_DUTY_CYCLE_DEN,
                   LED_DEFAULT_PERIOD);

    printf("OK: locked %d-%d Unlocked %d-%d\n",
           locked_position.first, locked_position.second,
           unlocked_position.first, unlocked_position.second);

    is_calibrated = true;
    state = Unlocked;
    
    led.set_params(LED_DEFAULT_DUTY_CYCLE_NUM,
                   LED_DEFAULT_DUTY_CYCLE_DEN,
                   LED_DEFAULT_PERIOD);

    return 0;
}

static int uncalibrate(int argc, char** argv)
{
    is_calibrated = false;

    printf("OK\n");
    
    return 0;
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

    state = Unknown;

    const int sign = degrees < 0 ? -1 : 1;
    const int abs_degrees = abs(degrees);
    const int needed_pulses = abs_degrees * Encoder::STEPS_PER_REVOLUTION / 360;
    
    const int MAX_TIME = 10000; // ms
    const auto start_pos = encoder_position.load();
    const auto start_tick = xTaskGetTickCount();
    motor->drive(sign * default_motor_power);
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
            verbose_printf("Encoder %d\n", pos);
            k = 0;
        }
        if (xTaskGetTickCount() - start_tick > MAX_TIME/portTICK_PERIOD_MS)
        {
            printf("Error: Timeout!\n");
            return 0;
        }
    }
    motor->brake();
    return 0;
}

int do_drive(int sign, int pwr, int ms)
{
    motor->drive(sign * pwr);
    vTaskDelay(ms/portTICK_PERIOD_MS);
    motor->brake();
    return 0;
}

int forward(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &forward_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, forward_args.end, argv[0]);
        return 1;
    }
    const auto pwr = forward_args.power->ival[0];
    if (pwr < 0 || pwr > 1000)
    {
        printf("Invalid power value\n");
        return 1;
    }
    const auto ms = forward_args.milliseconds->ival[0];
    if (ms < 100 || ms > 5000)
    {
        printf("Invalid milliseconds value\n");
        return 1;
    }
    state = Unknown;
    return do_drive(1, pwr, ms);
}

int reverse(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &reverse_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, reverse_args.end, argv[0]);
        return 1;
    }
    const auto pwr = reverse_args.power->ival[0];
    if (pwr < 0 || pwr > 1000)
    {
        printf("Invalid power value\n");
        return 1;
    }
    const auto ms = reverse_args.milliseconds->ival[0];
    if (ms < 100 || ms > 5000)
    {
        printf("Invalid milliseconds value\n");
        return 1;
    }
    state = Unknown;
    return do_drive(-1, pwr, ms);
}

struct rotate_result
{
    bool ok = false;
    bool reversed = false;
    std::string error_message;
};

rotate_result rotate_to(bool fwd, int position)
{
    rotate_result res;
    
    const auto start_pos = encoder_position.load();
    verbose_printf("rotate_to: start_pos %d\n", start_pos);
    if ((fwd && (position < start_pos)) ||
        (!fwd && (position > start_pos)))
    {
        fwd = !fwd;
        verbose_printf("reverse\n");
        res.reversed = true;
    }
    const int MAX_TOTAL_PULSES = 2.5 * Encoder::STEPS_PER_REVOLUTION;
    const int steps_needed = fabs(position - start_pos);
    if (steps_needed > MAX_TOTAL_PULSES)
    {
        printf("Impossible: Distance %d\n", steps_needed);
        res.error_message = "move too large";
        return res;
    }
    verbose_printf("rotate_to: steps_needed %d\n", steps_needed);

    const auto start_ms = xTaskGetTickCount()*portTICK_PERIOD_MS;
    bool engaged = false;
    const int pwr = fwd ? default_motor_power : -default_motor_power;
    motor->drive(pwr);
    const int max_engage_ms = motor->get_max_engage_time_ms(pwr);
    const int no_rotation_timeout = motor->get_rotation_timeout_ms(pwr);
    int last_encoder_pos = std::numeric_limits<int>::min();
    int last_position_change = 0;
    while (1)
    {
        const auto now = xTaskGetTickCount()*portTICK_PERIOD_MS;
        const auto pos = encoder_position.load();
        if (!engaged)
        {
            if (now - start_ms > max_engage_ms)
            {
                backoff(-pwr);
                printf("Engage timeout: start %ld now %ld\n",
                       (long) start_ms, (long) now);
                led.set_params(10, 100, 40);
                res.error_message = "engage timeout";
                return res;
            }
            if (pos != start_pos)
            {
                engaged = true;
                verbose_printf("Engaged\n");
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
                verbose_printf("Hit limit\n");
                verbose_wait();
                backoff(-pwr);
                verbose_printf("last change %ld\n", (long) last_position_change);
                res.error_message = "hit limit";
                return res;
            }
        }
        last_encoder_pos = pos;
        const auto steps_total = fabs(pos - start_pos);
        if (steps_total > MAX_TOTAL_PULSES)
        {
            backoff(-pwr);
            printf("\nTimeout!\n");
            res.error_message = "limit timeout";
            return res;
        }
        if (steps_total >= steps_needed)
        {
            verbose_printf("rotate_to: steps_total %d\n", steps_needed);
            break;
        }
    }

    motor->brake();
    res.ok = true;
    return res;
}

static int lock(int, char**)
{
    if (!is_calibrated)
    {
        printf("Error: not calibrated\n");
        return 0;
    }
    update_state();
    if (state != Locked)
    {
        state = Unknown;
        led.set_params(50, 100, 1);
        const auto res = rotate_to(false, locked_position.second - 1);
        if (!res.ok)
        {
            backoff(default_motor_power);
            if (rotate_to(true, unlocked_position.first + 1).ok)
            {
                printf("ERROR: could not lock (still unlocked): %s\n", res.error_message.c_str());
                state = Unlocked;
            }
            else
                printf("ERROR: could not lock (or unlock): %s\n", res.error_message.c_str());
            return 0;
        }
        if (!res.reversed)
        {
            // Back off
            backoff(default_motor_power);
            verbose_printf("Backed off to %d\n", encoder_position.load());
        }
        led.set_params(LED_DEFAULT_DUTY_CYCLE_NUM,
                       LED_DEFAULT_DUTY_CYCLE_DEN,
                       LED_DEFAULT_PERIOD);
        state = Locked;
    }
    printf("OK: locked\n");
    return 0;
}

static int unlock(int, char**)
{
    if (!is_calibrated)
    {
        printf("Error: not calibrated\n");
        return 0;
    }
    update_state();
    if (state != Unlocked)
    {
        state = Unknown;
        led.set_params(10, 100, 1);
        const auto res = rotate_to(true, unlocked_position.first + 1);
        if (!res.ok)
        {
            backoff(default_motor_power);
            if (rotate_to(false, locked_position.second - 1).ok)
            {
                printf("ERROR: could not unlock (still locked): %s\n", res.error_message.c_str());
                state = Locked;
            }
            else
                printf("ERROR: could not unlock (or lock): %s\n", res.error_message.c_str());
            return 0;
        }
        if (!res.reversed)
        {
            // Back off
            backoff(-default_motor_power);
            verbose_printf("Backed off\n");
        }
        led.set_params(LED_DEFAULT_DUTY_CYCLE_NUM,
                       LED_DEFAULT_DUTY_CYCLE_DEN,
                       LED_DEFAULT_PERIOD);
        state = Unlocked;
    }
    printf("OK: unlocked\n");
    return 0;
}

static int set_verbosity(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &set_verbosity_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, set_verbosity_args.end, argv[0]);
        return 1;
    }
    verbosity = set_verbosity_args.verbosity->ival[0];
    printf("OK: Verbosity is %d\n", verbosity);
    return 0;
}

static int version(int, char**)
{
    printf("Danalock " VERSION "\n");
    return 0;
}

static int status(int, char**)
{
    verbose_printf("status: initial state %d\n", (int) state);
    update_state();
    const char* status = "?";
    switch (state)
    {
    case Unknown:
        status = "unknown";
        break;
    case Locked:
        status = "locked";
        break;
    case Unlocked:
        status = "unlocked";
        break;
    case LockedManually:
        status = "lockedmanually";
        break;
    case UnlockedManually:
        status = "unlockedmanually";
        break;
    case ChangedManually:
        status = "changedmanually";
        break;
    default:
        printf("Unhandled state: %d\n", (int) state);
        assert(false);
        break;
    }
    printf("OK: status %s\n", status);
    return 0;
}

static int read_encoder(int, char**)
{
    for (int n = 0; n < 100; ++n)
    {
        vTaskDelay(500/portTICK_PERIOD_MS);
        printf("Encoder %d\n", encoder_position.load());
    }
    update_state();
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
    linenoiseSetDumbMode(1);
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

    rotate_args.degrees = arg_int1(NULL, NULL, "<degrees>", "Degrees");
    rotate_args.end = arg_end(2);
    const esp_console_cmd_t rotate_cmd = {
        .command = "rotate",
        .help = "Rotate <degrees>",
        .hint = nullptr,
        .func = &rotate,
        .argtable = &rotate_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&rotate_cmd));

    forward_args.power = arg_int1(NULL, NULL, "<power>", "Power");
    forward_args.milliseconds = arg_int1(NULL, NULL, "<milliseconds>", "Milliseconds");
    forward_args.end = arg_end(3);
    const esp_console_cmd_t forward_cmd = {
        .command = "forward",
        .help = "Run forward at <power> for <milliseconds>",
        .hint = nullptr,
        .func = &forward,
        .argtable = &forward_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&forward_cmd));

    reverse_args.power = arg_int1(NULL, NULL, "<power>", "Power");
    reverse_args.milliseconds = arg_int1(NULL, NULL, "<milliseconds>", "Milliseconds");
    reverse_args.end = arg_end(3);
    const esp_console_cmd_t reverse_cmd = {
        .command = "reverse",
        .help = "Run in reverse at <power> for <milliseconds>",
        .hint = nullptr,
        .func = &reverse,
        .argtable = &reverse_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&reverse_cmd));

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

    const esp_console_cmd_t uncalibrate_cmd = {
        .command = "uncalibrate",
        .help = "Forget calibration",
        .hint = nullptr,
        .func = &uncalibrate,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&uncalibrate_cmd));

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

    set_verbosity_args.verbosity = arg_int1(NULL, NULL, "<verbosity>", "Verbosity");
    set_verbosity_args.end = arg_end(2);
    const esp_console_cmd_t set_verbosity_cmd = {
        .command = "set_verbosity",
        .help = "Set verbosity",
        .hint = nullptr,
        .func = &set_verbosity,
        .argtable = &set_verbosity_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&set_verbosity_cmd));

    const esp_console_cmd_t version_cmd = {
        .command = "V",
        .help = "Get version",
        .hint = nullptr,
        .func = &version,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&version_cmd));

    const esp_console_cmd_t status_cmd = {
        .command = "status",
        .help = "Get status",
        .hint = nullptr,
        .func = &status,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&status_cmd));

    const char* prompt = "";

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
