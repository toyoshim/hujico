// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "chlib/ch559.h"
#include "chlib/led.h"
#include "dual_hid.h"

void main() {
  initialize();
  led_init(0, 7, LOW);
  delay(30);
  dual_hid_init();
  Serial.printf("\nBoot FUJICO v1.00\n");

  for (;;)
    led_poll();
}
