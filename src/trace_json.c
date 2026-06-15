#include "trace_json.h"

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(zmk_trace_json, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_TRACE_USB_CDC) && DT_HAS_CHOSEN(zephyr_console)
static const struct device *trace_uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
K_MSGQ_DEFINE(trace_msgq, CONFIG_ZMK_TRACE_BUFFER_SIZE, CONFIG_ZMK_TRACE_QUEUE_SIZE, 4);

static bool trace_uart_is_connected(void) {
    uint32_t dtr = 0;

    if (!device_is_ready(trace_uart)) {
        return false;
    }

    return uart_line_ctrl_get(trace_uart, UART_LINE_CTRL_DTR, &dtr) != 0 || dtr;
}

static void trace_write_line(const char *line) {
    if (!trace_uart_is_connected()) {
        return;
    }

    for (const char *p = line; *p != '\0'; p++) {
        uart_poll_out(trace_uart, *p);
    }
    uart_poll_out(trace_uart, '\n');
}

static void trace_output_thread(void *arg1, void *arg2, void *arg3) {
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    char line[CONFIG_ZMK_TRACE_BUFFER_SIZE];

    while (true) {
        k_msgq_get(&trace_msgq, line, K_FOREVER);
        trace_write_line(line);
    }
}

K_THREAD_DEFINE(trace_output_tid, CONFIG_ZMK_TRACE_THREAD_STACK_SIZE, trace_output_thread, NULL,
                NULL, NULL, CONFIG_ZMK_TRACE_THREAD_PRIORITY, 0, 0);
#endif

int zmk_trace_emit_json(const char *json) {
#if IS_ENABLED(CONFIG_ZMK_TRACE_USB_CDC) && DT_HAS_CHOSEN(zephyr_console)
    static int64_t last_emit_ms;
    const int64_t now = k_uptime_get();
    char line[CONFIG_ZMK_TRACE_BUFFER_SIZE];

    if (!trace_uart_is_connected()) {
        return 0;
    }

    if (CONFIG_ZMK_TRACE_MIN_INTERVAL_MS > 0 &&
        (now - last_emit_ms) < CONFIG_ZMK_TRACE_MIN_INTERVAL_MS) {
        return 0;
    }
    last_emit_ms = now;

    snprintk(line, sizeof(line), "%s", json);
    (void)k_msgq_put(&trace_msgq, line, K_NO_WAIT);
    return 0;
#endif
    LOG_INF("%s", json);
    return 0;
}

int zmk_trace_emit_event(const char *name, const char *data_json) {
    char line[CONFIG_ZMK_TRACE_BUFFER_SIZE];
    snprintk(line, sizeof(line), "{\"type\":\"event\",\"name\":\"%s\",\"data\":%s}", name, data_json ? data_json : "{}");
    return zmk_trace_emit_json(line);
}
