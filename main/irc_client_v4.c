#include <string.h>
#include "sdkconfig.h"
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include "esp_log.h"

extern void connect_to_server(void);

extern void send_command(char *payload);

extern void receive_response(char *rx_buffer, size_t n);

extern void close_socket(void);

const char* usernames[4] = {"badusb_esp", "_badusb_esp", "__badusb_esp", "___badusb_esp"};
const int username_count = 4;
const char channel_name[] = "#badusb";

void send_nick(char *username) {
    char request[50] = "";
    strcat(request, "NICK ");
    strcat(request, username);
    strcat(request, "\r\n");
    send_command(request);
}

void send_user(char *username) {
    char request[50] = "";
    strcat(request, "USER ");
    strcat(request, username);
    strcat(request, " * * :");
    strcat(request, username);
    strcat(request, "\r\n");
    send_command(request);
}

void register_user(void) {
    int success = 0;
    int current_username = 0;
    while (!success) {
        char rx_buffer[500];
        send_nick(usernames[current_username]);
        send_user(usernames[current_username]);
        receive_response(rx_buffer, sizeof(rx_buffer));
        if (strstr(rx_buffer, "433")) {
            if (current_username < username_count - 1) {
                current_username++;
            } else {
                current_username = 0;
            }
        } else {
            success = 1;
        }
    }
}

void join_channel(void) {
    char request[50] = "JOIN ";
    strcat(request, channel_name);
    strcat(request, "\r\n");
    send_command(request);
}

void send_pong(void) {
    send_command("PONG badusb.test.org\r\n");
}

void send_message(char *message) {
    char request[200] = "PRIVMSG ";
    strcat(request, channel_name);
    strcat(request, " :");
    strcat(request, message);
    strcat(request, "\r\n");
    send_command(request);
}

void handle_messages(void) {
    char rx_buffer[500];
    while (1) {
        receive_response(rx_buffer, sizeof(rx_buffer));
        if (strstr(rx_buffer, "PING")) {
            send_pong();
        } else {
            send_message("Unknown command received!");
        }
    }
}

void irc_client(void) {
    connect_to_server();
    register_user();
    join_channel();
    handle_messages();
}