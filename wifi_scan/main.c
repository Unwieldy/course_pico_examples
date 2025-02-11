#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"

#define MAX_RESULTS 40

struct results {
    int count;
    cyw43_ev_scan_result_t results[MAX_RESULTS];
} typedef results;

int add_result(results *res, cyw43_ev_scan_result_t *result)
{
    if (res->count < MAX_RESULTS) {
        // auto scan = (struct results*)env;
        res->results[res->count++] = *result;
        return 0;
    }
    else {
        printf("fail\n");
        return 1;
    }
}


static int scan_result(void *env, const cyw43_ev_scan_result_t *result) {
    add_result(env, result);
    if (result) {
        printf("ssid: %-32s rssi: %4d chan: %3d mac: %02x:%02x:%02x:%02x:%02x:%02x sec: %x\n",
            result->ssid, result->rssi, result->channel,
            result->bssid[0], result->bssid[1], result->bssid[2], result->bssid[3], result->bssid[4], result->bssid[5],
            result->auth_mode);
    }
    return 0;
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    absolute_time_t scan_time = nil_time;

    results ret = {0};
    while(true) {
        if (absolute_time_diff_us(get_absolute_time(), scan_time) < 0) {
            if (!cyw43_wifi_scan_active(&cyw43_state)) {
                cyw43_wifi_scan_options_t scan_options = {0};
                int err = cyw43_wifi_scan(&cyw43_state, &scan_options, &ret, scan_result);
                if (err == 0) {
                    printf("\nScanning for wifi...\n");
                } else {
                    printf("Failed to start scan: %d\n", err);
                    scan_time = make_timeout_time_ms(2000); // wait 10s and scan again
                }

                // Blocking while waiting for scan to complete.
                while(cyw43_wifi_scan_active(&cyw43_state))
                {
                    sleep_ms(200);
                }
                printf("Count: %d\n", ret.count);
                memset(ret.results, 0, sizeof(results));
                ret.count = 0;
            } else {
                scan_time = make_timeout_time_ms(2000); // wait 10s and scan again
            }
        }
        sleep_ms(5000);
    }

    cyw43_arch_deinit();
    return 0;
}
