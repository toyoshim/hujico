// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "jvs.h"

#include "chlib/adc.h"
#include "chlib/ch559.h"
#include "chlib/io.h"
#include "chlib/rs485.h"
#include "jvsio/JVSIO_c.h"

struct JVSIO_DataClient data;
struct JVSIO_SenseClient sense;
struct JVSIO_LedClient led;
struct JVSIO_Lib* lib = 0;

// JVS#1 - P1.0 SENSE
// JVS#2 - P5.4 D-
// JVS#3 - P5.5 D+
// JVS#4 - GND

static int data_available(struct JVSIO_DataClient* client) {
  client;
  return rs485_ready() ? 1 : 0;
}

static void data_setInput(struct JVSIO_DataClient* client) {
  client;
}

static void data_setOutput(struct JVSIO_DataClient* client) {
  client;
}

static void data_startTransaction(struct JVSIO_DataClient* client) {
  client;
}

static void data_endTransaction(struct JVSIO_DataClient* client) {
  client;
}

static uint8_t data_read(struct JVSIO_DataClient* client) {
  client;
  return rs485_recv();
}

static void data_write(struct JVSIO_DataClient* client, uint8_t data) {
  client;
  rs485_send(data);
}

static void data_delayMicroseconds(struct JVSIO_DataClient* client,
                                   unsigned int usec) {
  client;
  delayMicroseconds(usec);
}

static void data_delay(struct JVSIO_DataClient* client, unsigned int msec) {
  client;
  delay(msec);
}

static void sense_begin(struct JVSIO_SenseClient* client) {
  client;
}

static void sense_set(struct JVSIO_SenseClient* client, bool ready) {
  client;
  ready;
}

static bool sense_is_ready(struct JVSIO_SenseClient* client) {
  client;
  return adc_get(0) < (50 << 3);
}

static bool sense_is_connected(struct JVSIO_SenseClient* client) {
  client;
  return adc_get(0) < (240 << 3);
}

static void led_begin(struct JVSIO_LedClient* client) {
  client;
}

static void led_set(struct JVSIO_LedClient* client, bool ready) {
  client;
  ready;
}

void jvs_init() {
  data.available = data_available;
  data.setInput = data_setInput;
  data.setOutput = data_setOutput;
  data.startTransaction = data_startTransaction;
  data.endTransaction = data_endTransaction;
  data.read = data_read;
  data.write = data_write;
  data.delayMicroseconds = data_delayMicroseconds;
  data.delay = data_delay;

  sense.begin = sense_begin;
  sense.set = sense_set;
  sense.is_ready = sense_is_ready;
  sense.is_connected = sense_is_connected;

  led.begin = led_begin;
  led.set = led_set;

  led_init(0, 7, LOW);
  rs485_init();
  adc_init();

  lib = JVSIO_open(&data, &sense, &led, 0);
  lib->begin(lib);
}

void jvs_poll() {
  static bool ready = false;

  led_poll();

  if (sense_is_connected(0)) {
    led_mode(L_ON);
    if (!ready) {
      ready = lib->boot(lib, false);
      if (ready) {
        Serial.println("JVS Bus ready");
        led_oneshot(L_PULSE_THREE_TIMES);
      }
    }
  } else {
    led_mode(L_FASTER_BLINK);
    ready = false;
  }
  if (!ready)
    return;
}