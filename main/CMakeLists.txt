idf_build_get_property(target IDF_TARGET)

if(${target} STREQUAL "linux")
    set(requires esp_event esp_stubs protocol_examples_common nvs_flash)
endif()

if("${CONFIG_IPV4}" STREQUAL y)
    set(tcp_client_ip tcp_client_v4.c)
    set(irc_client_ip irc_client_v4.c)
endif()

idf_component_register(SRCS "irc_client_main.c" "irc_client_v4.c" "${tcp_client_ip}"
        INCLUDE_DIRS "."
        REQUIRES ${requires})
