#include "pico/cyw43_arch.h"
#include "lwip/ip_addr.h" // for netif_ip4_addr and ipaddr_ntoa if using lwIP
#include <stdio.h>
#include <string.h>

// Note: cyw43_state is assumed to be defined in your cyw43 implementation.
extern cyw43_t cyw43_state;

// Initialize WiFi adapter
bool wifi_init(void) {
  // Init Wifi Adapter
  if (cyw43_arch_init()) {
    return false;
  }

  // Set power management mode to performance.
  cyw43_wifi_pm(&cyw43_state, CYW43_PERFORMANCE_PM);

  return true;
}

// Join network with given SID and password
bool wifi_join(const char *sid, const char *password) {
  // Enable station mode.
  cyw43_arch_enable_sta_mode();
  printf("Connecting to WiFi... %s \n", sid);

  // Attempt to connect for up to 60000ms.
  if (cyw43_arch_wifi_connect_timeout_ms(sid, password, CYW43_AUTH_WPA2_AES_PSK, 60000)) {
    printf("Failed to join AP: %s.\n", sid);
    // Depending on your required behavior, you might return false here.
  }
  return true;
}

// Copy IP address (in network byte order) to provided 4-byte buffer.
bool wifi_getIPAddress(char *ip) {
  memcpy(ip, netif_ip4_addr(&cyw43_state.netif[0]), 4);
  return true;
}

// Get IP address string, copying to the provided buffer.
bool wifi_getIPAddressStr(char *ips) {
  char *s = ipaddr_ntoa(netif_ip4_addr(&cyw43_state.netif[0]));
  strcpy(ips, s);
  return true;
}

// Copy Gateway address (in network byte order) to provided 4-byte buffer.
bool wifi_getGWAddress(char *ip) {
  memcpy(ip, netif_ip4_gw(&cyw43_state.netif[0]), 4);
  return true;
}

// Get Gateway address string, copying to the provided buffer.
bool wifi_getGWAddressStr(char *ips) {
  char *s = ipaddr_ntoa(netif_ip4_gw(&cyw43_state.netif[0]));
  strcpy(ips, s);
  return true;
}

// Check whether the device has joined the WiFi network.
bool wifi_isJoined(void) {
  int res = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
  return (res >= 0);
}