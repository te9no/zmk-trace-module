if(CONFIG_ZMK_TRACE)
  target_sources(app PRIVATE src/trace_json.c)
  target_sources(app PRIVATE src/event_trace.c)
endif()

if(CONFIG_ZMK_TRACE AND CONFIG_ZMK_TRACE_INPUT_PROCESSOR)
  target_sources(app PRIVATE src/input_processor_trace.c)
endif()
