#zephyr_library()
zephyr_library_sources(display_lcd.c
                        framedisk.c)
zephyr_include_directories(include)

zephyr_include_directories(${ZEPHYR_BASE}/../modules/hal/nxp/mcux/mcux-sdk/drivers/elcdif)
zephyr_library_sources(${ZEPHYR_BASE}/../modules/hal/nxp/mcux/mcux-sdk/drivers/elcdif/fsl_elcdif.c)
