#include <stdlib.h>
#include <stdio.h>
#include <iwlib.h>

#define WHEEL_RADIUS  (32)

int main(void) {
unsigned int speed=0xdad9a5;
printf("speed=%f\n", (double)speed*(double)WHEEL_RADIUS*2*3.14/200000000.0);

#if 0
  wireless_scan_head head;
  wireless_scan *result;
  iwrange range;
  int sock;

  /* Open socket to kernel */
  sock = iw_sockets_open();

  /* Get some metadata to use for scanning */
  if (iw_get_range_info(sock, "wlan0", &range) < 0) {
    printf("Error during iw_get_range_info. Aborting.\n");
    exit(2);
  }

  /* Perform the scan */
  if (iw_scan(sock, "wlan0", range.we_version_compiled, &head) < 0) {
    printf("Error during iw_scan. Aborting.\n");
    exit(2);
  }

  /* Traverse the results */
  result = head.result;
  while (NULL != result) {
    printf("%s, Signal=%d:%d\n", result->b.essid, result->stats.qual.level, result->stats.qual.noise);
    result = result->next;
  }
#endif
  return 0;
}

