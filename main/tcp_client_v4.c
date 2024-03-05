#include "sdkconfig.h"
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include "esp_log.h"

#if defined(CONFIG_SOCKET_IP_INPUT_STDIN)
#include "addr_from_stdin.h"
#endif

#if defined(CONFIG_IPV4)
#define HOST_IP_ADDR CONFIG_IPV4_ADDR
#elif defined(CONFIG_SOCKET_IP_INPUT_STDIN)
#define HOST_IP_ADDR ""
#endif

#define PORT CONFIG_PORT

static const char *TAG = "tcp_client";
int sock = -1;
int connected = -1;
char host_ip[] = HOST_IP_ADDR;

void close_socket(void) {
    if (sock != -1) {
        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
    }
}

void connect_to_server(void) {
    int addr_family = 0;
    int ip_protocol = 0;

#if CONFIG_IPV4
    struct sockaddr_in dest_addr;
    inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol =IPPROTO_IP;
#elif defined(CONFIG_SOCKET_IP_INPUT_STDIN)
    struct sockaddr_storage dest_addr = { 0 };
    ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_STREAM, &ip_protocol, &addr_family, &dest_addr));
#endif

    sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

    connected = connect(sock, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
    if (connected != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        return;
    }

    ESP_LOGI(TAG, "Successfully connected");
}

void send_command(char* payload) {
    if (connected == -1) {
        ESP_LOGE(TAG, "Cannot send command since socket is not connected");
        return;
    }

    int err = send(sock, payload, strlen(payload), 0);
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        if(errno == 104){
            close_socket();
            connected = -1;
        }
        return;
    }
    ESP_LOGI(TAG, "Sent command: %s", payload);
}

void receive_response(char rx_buffer[], size_t n) {
    if (connected == -1) {
        ESP_LOGE(TAG, "Cannot receive command since socket is not connected");
        return;
    }

    int len = recv(sock, rx_buffer, n - 1, 0);

    if (len < 0) {
        ESP_LOGE(TAG, "recv failed: errno %d", errno);
        return;
    } else {
        rx_buffer[len] = 0;
        ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
        ESP_LOGI(TAG, "%s", rx_buffer);
    }
}


