#include <stdio.h>  
#include <string.h>  
#include <stdlib.h>  
  
#include "pico/stdlib.h"  
#include "pico/cyw43_arch.h"  
  
#include "lwip/tcp.h"  
#include "lwip/ip4_addr.h"  
  
// === Configuration ===  

#ifndef WIFI_SSID
#error "WIFI_SSID is not defined"
#endif
#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD is not defined"
#endif
#ifndef TCP_SERVER_IP
#error "TCP_SERVER_IP is not defined"
#endif

#define SERVER_PORT     8080
  
// === TCP Client State Structure ===  
typedef struct {  
    struct tcp_pcb *pcb;          // Pointer to the TCP control block  
    ip_addr_t server_addr;        // Server IP address  
    bool message_sent;            // Flag indicating if message was sent  
} tcp_client_state_t;  
  
// === Callback: Sent Data ===  
static err_t tcp_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len) {  
    tcp_client_state_t *state = (tcp_client_state_t*)arg;  
    printf("Sent %u bytes to server.\n", len);  
    state->message_sent = true;  
  
    // Close the connection after sending the message  
    tcp_close(tpcb);  
    state->pcb = NULL;  
  
    return ERR_OK;  
}  
  
// === Callback: Connection Established ===  
static err_t tcp_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {  
    tcp_client_state_t *state = (tcp_client_state_t*)arg;  
  
    if (err != ERR_OK) {  
        printf("Connection to server failed: %d\n", err);  
        free(state);  
        return err;  
    }  
  
    printf("Connected to server.\n");  
    
    const char* message = "Hello, World!\n";
    // Send the "Hello, World!" message  
    err_t write_err = tcp_write(tpcb, message, strlen(message), TCP_WRITE_FLAG_COPY);  
    if (write_err != ERR_OK) {  
        printf("Failed to send message: %d\n", write_err);  
        tcp_close(tpcb);  
        free(state);  
        return write_err;  
    }  
  
    // Set the sent callback to know when data has been sent  
    tcp_sent(tpcb, tcp_sent_callback);  
  
    return ERR_OK;  
}  
  
// === Callback: Connection Error ===  
static void tcp_error_callback(void *arg, err_t err) {  
    tcp_client_state_t *state = (tcp_client_state_t*)arg;  
    printf("TCP connection error: %d\n", err);  
    if (state) {  
        free(state);  
    }  
}  
  
// === Function to Start TCP Connection ===  
static void start_tcp_client(tcp_client_state_t *state) {  
    // Create a new TCP PCB with IPv4  
    state->pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);  
    if (!state->pcb) {  
        printf("Failed to create new TCP PCB.\n");  
        free(state);  
        return;  
    }  
  
    // Set the server IP address  
    ip4addr_aton(TCP_SERVER_IP, &state->server_addr);  
  
    // Assign callbacks  
    tcp_arg(state->pcb, state);  
    tcp_err(state->pcb, tcp_error_callback);  
  
    // Initiate connection to the server  
    err_t err = tcp_connect(state->pcb, &state->server_addr, SERVER_PORT, tcp_connected_callback);  
    if (err != ERR_OK) {  
        printf("tcp_connect failed: %d\n", err);  
        tcp_close(state->pcb);  
        free(state);  
        return;  
    }  
}  
  
// === Main Function ===  
int main() {  
    // Initialize standard I/O  
    stdio_init_all();  
  
    // Initialize the CYW43 Wi-Fi driver  
    if (cyw43_arch_init()) {  
        printf("Failed to initialize CYW43 architecture.\n");  
        return 1;  
    }  
  
    // Enable station (client) mode  
    cyw43_arch_enable_sta_mode();  
  
    // Connect to Wi-Fi  
    printf("Connecting to Wi-Fi SSID: %s\n", WIFI_SSID);  
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {  
        printf("Failed to connect to Wi-Fi.\n");  
        cyw43_arch_deinit();  
        return 1;  
    } else {  
        printf("Connected to Wi-Fi.\n");  
    }  
  
    // Allocate memory for TCP client state  
    tcp_client_state_t *client_state = calloc(1, sizeof(tcp_client_state_t));  
    if (!client_state) {  
        printf("Failed to allocate memory for client state.\n");  
        cyw43_arch_deinit();  
        return 1;  
    }  
  
    // Start the TCP client  
    start_tcp_client(client_state);  
  
    // Main loop: Poll for events until the message is sent and connection is closed  
    while (client_state->pcb != NULL) {  
        cyw43_arch_poll();  
        sleep_ms(100);  // Small delay to prevent CPU hogging  
    }  
  
    printf("Message sent and connection closed.\n");  
  
    // Clean up  
    free(client_state);  
    cyw43_arch_deinit();  
    return 0;  
}  

