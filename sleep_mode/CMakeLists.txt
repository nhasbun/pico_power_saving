add_executable(sleep_mode
        sleep_mode.c
        )

# pull in common dependencies
target_link_libraries(sleep_mode pico_stdlib hardware_sleep)

pico_enable_stdio_usb(sleep_mode 1)
pico_enable_stdio_uart(sleep_mode 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(sleep_mode)

