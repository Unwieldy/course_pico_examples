#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define TCP_PORT 4242
#define BUF_SIZE 2048
#define POLL_TIME_S 5

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    bool complete;
} TCP_SERVER_T;


static err_t tcp_server_close_client(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != NULL) {
        tcp_arg(state->client_pcb, NULL);
        tcp_poll(state->client_pcb, NULL, 0);
        tcp_sent(state->client_pcb, NULL);
        tcp_recv(state->client_pcb, NULL);
        tcp_err(state->client_pcb, NULL);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            printf("Close failed %d, calling abort\n", err);
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = NULL;
    }
    return err;
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb) {
    printf("tcp_server_poll_fn\n");
    return tcp_server_close_client(arg);
}

static void tcp_server_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        printf("tcp_client_err_fn %d\n", err);
    }
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        return tcp_server_close_client(arg);
    }

    if (p->tot_len > 0) {
        char *data = malloc(p->tot_len + 1);
        if (pbuf_copy_partial(p, data, p->tot_len, 0) > 0) {
            data[p->tot_len] = '\0';
            printf("Received: %s\n", data);
            tcp_write(tpcb, data, p->tot_len, TCP_WRITE_FLAG_COPY);
        }
        free(data);
        tcp_recved(tpcb, p->tot_len);
    }

    pbuf_free(p);
    return ERR_OK;
}

static TCP_SERVER_T* tcp_server_init(void) {
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        printf("Failed to allocate state\n");
        return NULL;
    }
    return state;
}


static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        printf("Failure in accept\n");
        return ERR_VAL;
    }
    printf("Client connected\n");

    state->client_pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

static bool tcp_server_open(TCP_SERVER_T *state) {
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        printf("Failed to create pcb\n");
        return false;
    }

    if (tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT) != ERR_OK) {
        printf("Failed to bind to port %u\n", TCP_PORT);
        tcp_close(pcb);
        return false;
    }

    state->server_pcb = tcp_listen(pcb);
    if (!state->server_pcb) {
        printf("Failed to listen\n");
        tcp_close(pcb);
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);
    return true;
}



void tcp_server_run(void) {
    TCP_SERVER_T *state = tcp_server_init();
    if (!state) {
        return;
    }
    if (!tcp_server_open(state)) {
        free(state);
        return;
    }

    while (!state->complete) {
        cyw43_arch_poll();
        sleep_ms(1000);
    }

    tcp_server_close_client(state);
    free(state);
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect.\n");
        return 1;
    }
    printf("Connected.\n");

    tcp_server_run();
    cyw43_arch_deinit();
    return 0;
}

