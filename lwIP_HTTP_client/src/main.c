#include "lwip/apps/http_client.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "lwip/dns.h"
#include "lwip/ip4_addr.h"
#include "lwip/sockets.h"

#include "c_WifiManager.h"
#include <stdio.h>

#define SERVER_HOST "192.168.1.173" //"vmu22a.local.jondurrant.com"
#define SERVER_PORT 8080
#define RELATIVE_URL "/"

// Check these definitions where added from the makefile
#ifndef WIFI_SSID
#error "WIFI_SSID not defined"
#endif
#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD not defined"
#endif

char myBuff[4096];

// Callback on completion of HTTP Get request
void result(void *arg, httpc_result_t httpc_result, u32_t rx_content_len,
            u32_t srv_res, err_t err) {
              if (httpc_result != HTTPC_RESULT_OK) {
                printf("GET request failed.");
              }
  printf("GET request completed.\n");
}

// Callback when the headers are available
err_t headers(httpc_state_t *connection, void *arg, struct pbuf *hdr,
              u16_t hdr_len, u32_t content_len) {
  printf("\nHeaders Recieved\n");
  return ERR_OK;
}

// Callback on Payload available
err_t payload(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err) {
  printf("Payload\n");
  pbuf_copy_partial(p, myBuff, p->tot_len, 0);
  printf("Length: %d\n", p->tot_len);
  myBuff[p->tot_len] = 0;
  printf("%s\n", myBuff);
  printf("END of payload\n");
  return ERR_OK;
}

int main(void) {
  stdio_init_all();
  sleep_ms(2000);
  printf("GO\n");

if (wifi_init()) {
    printf("Wifi Controller Initialised\n");
  } else {
    printf("Failed to initialise controller\n");
    return 1;
  }

  if (wifi_join(WIFI_SSID, WIFI_PASSWORD)) {
    printf("Connect to Wifi\n");
  } else {
    printf("Failed to connect to Wifi \n");
  }

  // Print IP Address
  char ipStr[20];
  wifi_getIPAddress(ipStr);
  printf("IP ADDRESS: %s\n", ipStr);

  // Print Gateway
  wifi_getGWAddress(ipStr);
  printf("Gateway: %s\n", ipStr);

  // HTTP Request
  // httpc_connection_t settings;
  // settings.result_fn = result;
  // settings.headers_done_fn = headers;
  // settings.use_proxy = false;
  //

  httpc_connection_t settings = {
      .result_fn = result,
      .headers_done_fn = headers,
      .use_proxy = false,
  };

  int count = 0;
  char fisk[500] = {0};

  for (;;) {
    sprintf(fisk, "/Count: %d", count++);

    err_t err = httpc_get_file_dns(
      SERVER_HOST,
      SERVER_PORT,
      fisk,
      &settings,
      payload,
      NULL,
      NULL);

    sleep_ms(500);
    cyw43_arch_poll();
  }

  return 0;
}
