idf_component_register(
    SRCS
        "thingsboard.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        esp_http_client
        esp-tls
)