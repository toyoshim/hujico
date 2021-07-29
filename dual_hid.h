// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __dual_hid_h__
#define __dual_hid_h__

#include <stdbool.h>

#include "chlib/usb_device.h"

void dual_hid_init();
void dual_hid_update(uint8_t sws[5], bool csw1, bool csw2);

#endif  // __dual_hid_h__