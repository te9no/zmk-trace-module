#define DT_DRV_COMPAT zmk_input_processor_trace

#include "trace_json.h"

#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <drivers/input_processor.h>
#include <zmk/keymap.h>

struct zmk_trace_input_processor_config {
    const char *stage;
};

static const char *event_code_name(uint16_t code) {
    switch (code) {
    case INPUT_REL_X:
        return "REL_X";
    case INPUT_REL_Y:
        return "REL_Y";
    case INPUT_REL_WHEEL:
        return "REL_WHEEL";
    case INPUT_REL_HWHEEL:
        return "REL_HWHEEL";
    default:
        return "OTHER";
    }
}

static void event_to_values(const struct input_event *event, int16_t *x, int16_t *y,
                            int16_t *scroll_x, int16_t *scroll_y) {
    *x = *y = *scroll_x = *scroll_y = 0;

    if (event->type != INPUT_EV_REL) {
        return;
    }

    switch (event->code) {
    case INPUT_REL_X:
        *x = event->value;
        break;
    case INPUT_REL_Y:
        *y = event->value;
        break;
    case INPUT_REL_HWHEEL:
        *scroll_x = event->value;
        break;
    case INPUT_REL_WHEEL:
        *scroll_y = event->value;
        break;
    default:
        break;
    }
}

static int trace_input_processor_handle_event(const struct device *dev, struct input_event *event,
                                              uint32_t param1, uint32_t param2,
                                              struct zmk_input_processor_state *state) {
    const struct zmk_trace_input_processor_config *config = dev->config;
    int16_t x, y, scroll_x, scroll_y;
    event_to_values(event, &x, &y, &scroll_x, &scroll_y);

    char line[CONFIG_ZMK_TRACE_BUFFER_SIZE];
    const uint8_t listener_index = state ? state->input_device_index : 0;
    const uint8_t active_layer = zmk_keymap_highest_layer_active();
    const zmk_keymap_layers_state_t layer_state = zmk_keymap_layer_state();
#if IS_ENABLED(CONFIG_ZMK_TRACE_INPUT_COMPACT)
    snprintk(line, sizeof(line), "i,%s,%u,%u,%d,%u,%u,%u,0x%08x", config->stage, event->type,
             event->code, event->value, event->sync ? 1 : 0, listener_index, active_layer,
             layer_state);
#else
    snprintk(line, sizeof(line),
             "{\"type\":\"input_stage\",\"processor\":\"%s\",\"stage\":\"%s\","
             "\"event\":{\"type\":%u,\"code\":%u,\"code_name\":\"%s\",\"value\":%d,\"sync\":%s},"
             "\"before\":{\"x\":%d,\"y\":%d,\"scroll_x\":%d,\"scroll_y\":%d},"
             "\"after\":{\"x\":%d,\"y\":%d,\"scroll_x\":%d,\"scroll_y\":%d},"
             "\"data\":{\"input_device_index\":%u,\"active_layer\":%u,\"layer_state\":%u,"
             "\"param1\":%u,\"param2\":%u}}",
             dev->name, config->stage, event->type, event->code, event_code_name(event->code),
             event->value, event->sync ? "true" : "false", x, y, scroll_x, scroll_y, x, y,
             scroll_x, scroll_y, listener_index, active_layer, layer_state, param1, param2);
#endif
    zmk_trace_emit_json(line);

    return ZMK_INPUT_PROC_CONTINUE;
}

static struct zmk_input_processor_driver_api trace_input_processor_driver_api = {
    .handle_event = trace_input_processor_handle_event,
};

static int zmk_trace_input_processor_init(const struct device *dev) {
    const struct zmk_trace_input_processor_config *config = dev->config;
    char line[CONFIG_ZMK_TRACE_BUFFER_SIZE];
    snprintk(line, sizeof(line), "{\"type\":\"event\",\"name\":\"trace_processor_ready\",\"data\":{\"stage\":\"%s\"}}",
             config->stage);
    return zmk_trace_emit_json(line);
}

#define ZMK_TRACE_INPUT_PROCESSOR_DEFINE(inst)                                                       \
    static const struct zmk_trace_input_processor_config zmk_trace_input_processor_config_##inst = { \
        .stage = DT_INST_PROP(inst, stage),                                                          \
    };                                                                                               \
    DEVICE_DT_INST_DEFINE(inst, zmk_trace_input_processor_init, NULL, NULL,                          \
                          &zmk_trace_input_processor_config_##inst, POST_KERNEL,                     \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &trace_input_processor_driver_api);

DT_INST_FOREACH_STATUS_OKAY(ZMK_TRACE_INPUT_PROCESSOR_DEFINE)
