if (TARGET tinyusb_device)
    add_executable(uwb_tdma2
    uwb_tdma2.c
            )

    # pull in common dependencies
    target_link_libraries(uwb_tdma2 pico_stdlib hardware_spi)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(uwb_tdma2 1)
    pico_enable_stdio_uart(uwb_tdma2 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(uwb_tdma2)

    # add url via pico_set_program_url
    example_auto_set_url(uwb_tdma2)

    #UWB api
    target_sources(uwb_tdma2 PRIVATE deca_device_api.h)
    target_sources(uwb_tdma2 PRIVATE deca_regs.h)
    target_sources(uwb_tdma2 PRIVATE deca_vals.h)
    target_sources(uwb_tdma2 PRIVATE deca_types.h)
    target_sources(uwb_tdma2 PRIVATE deca_version.h)
    target_sources(uwb_tdma2 PRIVATE deca_device.c)

    #UWB/TDMA Config
    target_sources(uwb_tdma2 PRIVATE uwb_config.h)
    target_sources(uwb_tdma2 PRIVATE tdma_config.h)

    #UWB Functions
    target_sources(uwb_tdma2 PRIVATE uwb_library.c)
    target_sources(uwb_tdma2 PRIVATE uwb_library.h)
    

elseif(PICO_ON_DEVICE)
    message(WARNING "not building uwb_tdma2 because TinyUSB submodule is not initialized in the SDK")
endif()
