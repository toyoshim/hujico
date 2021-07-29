// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "jvs.h"

#include "chlib/adc.h"
#include "chlib/ch559.h"
#include "chlib/io.h"
#include "chlib/rs485.h"
#include "jvsio/JVSIO_c.h"

static struct JVSIO_DataClient data;
static struct JVSIO_SenseClient sense;
static struct JVSIO_LedClient led;
static struct JVSIO_Lib* lib = 0;

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

void jvs_poll(void (*cb)(uint8_t sws[5], bool csw1, bool csw2)) {
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

  uint8_t requestSwInput[] = { 0x01, 0x04, kCmdSwInput, 2, 2 };
  uint8_t* ack;
  uint8_t ack_len;
  bool result = lib->sendAndReceive(lib, requestSwInput, &ack, &ack_len);
  if (!result || ack_len != 7 || ack[0] != 1 || ack[1] != 1)
    return;
  uint8_t sws[5];
  for (uint8_t i = 0; i < 5; ++i)
    sws[i] = ack[i + 2];

  uint8_t requestCoinInput[] = { 0x01, 0x03, kCmdCoinInput, 2 };
  result = lib->sendAndReceive(lib, requestCoinInput, &ack, &ack_len);
  if (!result || ack_len != 6 || ack[0] != 1 || ack[1] != 1)
    return;
  if (ack[2] & 0xc0 || ack[4] & 0xc0)
    return;
  uint16_t c1 = (ack[2] << 8) | ack[3];
  uint16_t c2 = (ack[4] << 8) | ack[5];

  uint8_t slot = c1 ? 1 : c2 ? 2 : 0;
  if (slot) {
    uint8_t requestCoinSub[] = { 0x01, 0x05, kCmdCoinSub, slot, 0, 1 };
    result = lib->sendAndReceive(lib, requestCoinSub, &ack, &ack_len);
    if (!result || ack_len != 2 || ack[0] != 1 || ack[1] != 1)
      return;
  }
  bool csw1 = slot == 1;
  bool csw2 = slot == 2;

  if (cb)
    cb(sws, csw1, csw2);
}