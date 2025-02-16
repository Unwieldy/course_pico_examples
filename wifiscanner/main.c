#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define MAX_RESULTS 40

typedef struct {
    int count;
    cyw43_ev_scan_result_t results[MAX_RESULTS];
} Scan;


int scan_duplicate_check(Scan *scan, const char *ssid)
{
    if (scan->count == 0)
        return 0;
    for (int i = 0; i < scan->count; i++) {
        if (strcmp(scan->results[i].ssid, ssid) == 0) {
            return 1;
        }
    }
    return 0;
}

int scan_add_result(Scan *scan, const cyw43_ev_scan_result_t *result)
{
    if (scan->count < MAX_RESULTS) {
        if (!scan_duplicate_check(scan, result->ssid)) {
            scan->results[scan->count++] = *result;
        }
        return 0;
    }
    else {
        printf("Scan list is full.\n");
        return 1;
    }
}

void scan_print_ssid(Scan *scan) {
    for (int i = 0; i < scan->count; i++) {
        printf("ssid: %-32s rssi: %4d chan: %3d mac: %02x:%02x:%02x:%02x:%02x:%02x sec: %x\n",
               scan->results[i].ssid,
               scan->results[i].rssi,
               scan->results[i].channel,
               scan->results[i].bssid[0],
               scan->results[i].bssid[1],
               scan->results[i].bssid[2],
               scan->results[i].bssid[3],
               scan->results[i].bssid[4],
               scan->results[i].bssid[5],
               scan->results[i].auth_mode);
    }
}

void scan_reset(Scan *scan)
{
    memset(scan->results, 0, sizeof(scan->results));
    scan->count = 0;
}

// Callback function for scan result.
static int scan_result(void *env, const cyw43_ev_scan_result_t *result) {
    scan_add_result(env, result);
    return 0;
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    Scan scan = {0};
    cyw43_wifi_scan_options_t scan_options = {0};

    for(;;) {
        // Ensure scan is not active before starting.
        if (!cyw43_wifi_scan_active(&cyw43_state)) {
            int err = cyw43_wifi_scan(&cyw43_state, &scan_options, &scan, scan_result);
            if (err == 0) {
                printf("\nScanning for wifi...\n");
            } else {
                printf("Failed to start scan: %d\n", err);
                sleep_ms(2000);
            }

            // Blocking while waiting for scan to complete.
            while(cyw43_wifi_scan_active(&cyw43_state))
            {
                sleep_ms(200);
            }

            scan_print_ssid(&scan);
            scan_reset(&scan);
        }
        sleep_ms(5000);
    }

    cyw43_arch_deinit();
    return 0;
}
