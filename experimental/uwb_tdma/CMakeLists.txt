if (TARGET tinyusb_device)
    add_executable(uwb_tdma
           uwb_tdma.c
            )

    # pull in common dependencies
    target_link_libraries(uwb_tdma pico_stdlib hardware_spi)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(uwb_tdma 1)
    pico_enable_stdio_uart(uwb_tdma 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(uwb_tdma)

    # add url via pico_set_program_url
    example_auto_set_url(uwb_tdma)

    #UWB api
    target_sources(uwb_tdma PRIVATE deca_device_api.h)
    target_sources(uwb_tdma PRIVATE deca_regs.h)
    target_sources(uwb_tdma PRIVATE deca_vals.h)
    target_sources(uwb_tdma PRIVATE deca_types.h)
    target_sources(uwb_tdma PRIVATE deca_version.h)
    target_sources(uwb_tdma PRIVATE deca_device.c)

    #UWB/TDMA Config
    target_sources(uwb_tdma PRIVATE uwb_config.h)
    target_sources(uwb_tdma PRIVATE tdma_config.h)

    #UWB Functions
    target_sources(uwb_tdma PRIVATE uwb_library.c)
    target_sources(uwb_tdma PRIVATE uwb_library.h)
    

elseif(PICO_ON_DEVICE)
    message(WARNING "not building uwb_tdma because TinyUSB submodule is not initialized in the SDK")
endif()
