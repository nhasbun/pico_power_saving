add_executable(sleepy_underclock
        sleepy_underclock.c
        )

# pull in common dependencies
target_link_libraries(sleepy_underclock pico_stdlib hardware_sleep)

pico_enable_stdio_usb(sleepy_underclock 1)
pico_enable_stdio_uart(sleepy_underclock 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(sleepy_underclock)

