add_executable(underclock
        underclock.c
        )

# pull in common dependencies
target_link_libraries(underclock pico_stdlib)

pico_enable_stdio_usb(underclock 1)
pico_enable_stdio_uart(underclock 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(underclock)

