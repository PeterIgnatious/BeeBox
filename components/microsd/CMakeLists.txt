idf_component_register(
    SRCS
       "microsd.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        vfs
        fatfs
        sdmmc
        esp_driver_sdspi
        esp_timer
)