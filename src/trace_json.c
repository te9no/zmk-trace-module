#include "trace_json.h"

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zmk_trace_json, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_TRACE_USB_CDC) && DT_HAS_CHOSEN(zephyr_console)
static const struct device *trace_uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
#endif

int zmk_trace_emit_json(const char *json) {
#if IS_ENABLED(CONFIG_ZMK_TRACE_USB_CDC) && DT_HAS_CHOSEN(zephyr_console)
    static int64_t last_emit_ms;
    uint32_t dtr = 0;
    const int64_t now = k_uptime_get();

    if (!device_is_ready(trace_uart)) {
        return 0;
    }

    if (uart_line_ctrl_get(trace_uart, UART_LINE_CTRL_DTR, &dtr) == 0 && !dtr) {
        return 0;
    }

    if (CONFIG_ZMK_TRACE_MIN_INTERVAL_MS > 0 &&
        (now - last_emit_ms) < CONFIG_ZMK_TRACE_MIN_INTERVAL_MS) {
        return 0;
    }
    last_emit_ms = now;

    for (const char *p = json; *p != '\0'; p++) {
        uart_poll_out(trace_uart, *p);
    }
    uart_poll_out(trace_uart, '\n');
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
