// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __jvs_h__
#define __jvs_h__

#include "chlib/led.h"

void jvs_init();
void jvs_poll(void (*cb)(uint8_t sws[5], bool csw1, bool csw2));

#endif  // __client_h__