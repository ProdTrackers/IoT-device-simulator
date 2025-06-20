#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECOND 1000000 // microsegundos

typedef struct {
  uart_dev_t uart;
  timer_t timer;
} chip_state_t;

void send_gps_data(void *user_data) {
  chip_state_t *chip = (chip_state_t *)user_data;
  const char *nmea = "$GPRMC,013246.608,A,1210.601,S,07659.944,W,,,150625,000.0,W*7D\r\n";
  uart_write(chip->uart, (uint8_t *)nmea, strlen(nmea));
}

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  uart_config_t uart_config = {
    .tx = pin_init("TX", OUTPUT),
    .rx = pin_init("RX", INPUT),
    .baud_rate = 9600,
    .user_data = chip,
  };
  chip->uart = uart_init(&uart_config);

  timer_config_t timer_config = {
    .callback = send_gps_data,
    .user_data = chip,
  };
  chip->timer = timer_init(&timer_config);

  timer_start(chip->timer, SECOND, true); // Llama cada segundo
}