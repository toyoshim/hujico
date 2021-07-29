// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "chlib/ch559.h"
#include "dual_hid.h"
#include "jvs.h"
#include "jvsio/JVSIO_c.h"

void main() {
  initialize();
  Serial.printf("\nBoot FUJICO v1.00\n");

  delay(30);

  dual_hid_init();
  Serial.println("USB Device ready");

  jvs_init();
  Serial.println("JVS Host ready");

  for (;;)
    jvs_poll();
}
