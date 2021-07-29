// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "jvs.h"

#include "chlib/ch559.h"
#include "chlib/io.h"
#include "chlib/led.h"
#include "chlib/rs485.h"
#include "jvsio/JVSIO_c.h"

struct JVSIO_DataClient data;
struct JVSIO_SenseClient sense;
struct JVSIO_LedClient led;

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
  // can be true for single node.
  return true;
}

static bool sense_is_connected(struct JVSIO_SenseClient* client) {
  client;
  // can be true for client mode.
  return true;
}

static void led_begin(struct JVSIO_LedClient* client) {
  client;
}

static void led_set(struct JVSIO_LedClient* client, bool ready) {
  client;
  led_oneshot(ready ? L_PULSE_ONCE : L_PULSE_THREE_TIMES);
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

  rs485_init();
}

void jvs_poll() {
}