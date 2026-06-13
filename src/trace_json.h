#pragma once

#include <zephyr/kernel.h>

int zmk_trace_emit_json(const char *json);
int zmk_trace_emit_event(const char *name, const char *data_json);
