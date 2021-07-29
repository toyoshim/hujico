// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "dual_hid.h"

#include "chlib/ch559.h"
#include "chlib/usb.h"

static uint8_t report[6];
static bool hold_csw1 = false;
static bool hold_csw2 = false;

static const uint8_t device_descriptor[] = {
  0x12,  // size
  USB_DESC_DEVICE,
  0x00, 0x02,  // USB version

  // Declare as a composite device
  0x00,  // device class
  0x00,  // device subclass
  0x00,  // protocol

  0x40,  // max packet size
  0x66, 0x66,  // vendor ID
  'D', 'H',  // device ID
  0x00, 0x01,  // device version
  0x01,  // manufacturer string index
  0x02,  // product string index
  0x00,  // serial number string index
  0x01,  // number of configurations
};

static const uint8_t configuration_descriptor[] = {
  0x09,  // size
  USB_DESC_CONFIGURATION,
  0x3b, 0x00,  // total length
  0x02,  // number of interfaces
  0x01,  // index of this configuration
  0x00,  // configuration name string index
  0xa0,  // attributes
  0x32,  // 100mA

  // interface descriptor 0
  0x09,  // size
  USB_DESC_INTERFACE,
  0x00,  // index of this interface
  0x00,  // alternate setting for this interface
  0x01,  // number of endpoints
  0x03,  // interface class (HID)
  0x00,  // interface subclass
  0x00,  // interface protocol
  0x03,  // string index for interface

  // hid descriptor 0
  0x09,  // size
  USB_DESC_HID,
  0x01, 0x01,  // BCD representation of HID verrsion
  0x00,  // target country code
  0x01,  // number of HID report
  USB_DESC_HID_REPORT,
  0x35, 0x00,  // descriptor length

  // endpoint descriptor 0
  0x07,  // size
  USB_DESC_ENDPOINT,
  0x81,  // IN endpoint number 1
  0x03,  // attribute: interrurpt endpoint
  0x40, 0x00,  // maximum packet size
  0x0a,  // poll interval 10ms

  // interface descriptor 1
  0x09,  // size
  USB_DESC_INTERFACE,
  0x01,  // index of this interface
  0x00,  // alternate setting for this interface
  0x01,  // number of endpoints
  0x03,  // interface class (HID)
  0x00,  // interface subclass
  0x00,  // interface protocol
  0x04,  // string index for interface

  // hid descriptor 1
  0x09,  // size
  USB_DESC_HID,
  0x01, 0x01,  // BCD representation of HID verrsion
  0x00,  // target country code
  0x01,  // number of HID report
  USB_DESC_HID_REPORT,
  0x35, 0x00,  // descriptor length

  // endpoint descriptor 1
  0x07,  // size
  USB_DESC_ENDPOINT,
  0x82,  // IN endpoint number 2
  0x03,  // attribute: interrurpt endpoint
  0x40, 0x00,  // maximum packet size
  0x0a,  // poll interval 10ms
};

static const uint8_t hid_report_descriptor[] = {
  0x05, 0x01,  // usage page (desktop)
  0x09, 0x05,  // usage (gamepad)
  0xa1, 0x01,  // collection (application)
  0x15, 0x00,  // logical minimum (0)
  0x25, 0x01,  // logical maximum (1)
  0x35, 0x00,  // physical minimum (0)
  0x45, 0x01,  // physical minimum (1)
  0x75, 0x01,  // report size (1)
  0x95, 0x0e,  // report count (14)
  0x05, 0x09,  // usage page (button)
  0x19, 0x01,  // usage minimum (1)
  0x29, 0x0e,  // usage maximum (14)
  0x81, 0x02,  // input (variable)
  0x95, 0x02,  // report count (2)
  0x81, 0x01,  // input (constant)
  0x05, 0x01,  // usage page (desktop)
  0x25, 0x07,  // logical maximum (7)
  0x45, 0x07,  // physical maximum (7)
  0x75, 0x04,  // report size (4)
  0x95, 0x01,  // report count (1)
  0x65, 0x14,  // unit (degrees)
  0x09, 0x39,  // usage (hat switch)
  0x81, 0x42,  // input (variable, null state)
  0x65, 0x00,  // unit
  0x95, 0x01,  // report count (1)
  0x81, 0x01,  // input (constant)
  0xc0,  // end collection
};

static const uint8_t string_descriptor_0[] = { 0x04, 0x03, 0x09, 0x04 };

static const uint8_t string_descriptor_1[] = {
  0x16, 0x03,
  'M', 0, 'e', 0, 'l', 0, 'l', 0, 'o', 0, 'w', 0, ' ', 0, 'P', 0,
  'C', 0, 'B', 0,
};

static const uint8_t string_descriptor_2[] = {
  0x0e, 0x03,
  'H', 0, 'U', 0, 'J', 0, 'I', 0, 'C', 0, 'O', 0,
};

static const uint8_t string_descriptor_3[] = {
  0x0e, 0x03,
  'J', 0, 'V', 0, 'S', 0, ' ', 0, '#', 0, '1', 0,
};

static const uint8_t string_descriptor_4[] = {
  0x0e, 0x03,
  'J', 0, 'V', 0, 'S', 0, ' ', 0, '#', 0, '2', 0,
};

static uint8_t get_descriptor_size(uint8_t type, uint8_t no) {
  switch (type) {
    case USB_DESC_DEVICE:
      return sizeof(device_descriptor);
    case USB_DESC_CONFIGURATION:
      return sizeof(configuration_descriptor);
    case USB_DESC_STRING:
      switch (no) {
        case 0:
          return sizeof(string_descriptor_0);
        case 1:
          return sizeof(string_descriptor_1);
        case 2:
          return sizeof(string_descriptor_2);
        case 3:
          return sizeof(string_descriptor_3);
        case 4:
          return sizeof(string_descriptor_4);
      }
      break;
    case USB_DESC_HID_REPORT:
      return sizeof(hid_report_descriptor);
  }
  return 0;
}

static const uint8_t* get_descriptor(uint8_t type, uint8_t no) {
  switch (type) {
    case USB_DESC_DEVICE:
      return device_descriptor;
    case USB_DESC_CONFIGURATION:
      return configuration_descriptor;
    case USB_DESC_STRING:
      switch (no) {
        case 0:
          return string_descriptor_0;
        case 1:
          return string_descriptor_1;
        case 2:
          return string_descriptor_2;
        case 3:
          return string_descriptor_3;
        case 4:
          return string_descriptor_4;
      }
      break;
    case USB_DESC_HID_REPORT:
      return hid_report_descriptor;
  }
  return 0;
}

static uint8_t get_report(uint8_t no, uint8_t* buffer) {
  uint8_t offset = (no == 1) ? 0 : 3;
  if (no == 1)
    hold_csw1 = false;
  else
    hold_csw2 = false;
  buffer[0] = report[offset + 0];
  buffer[1] = report[offset + 1];
  buffer[2] = report[offset + 2];
  return 3;
}

static uint8_t calc_hat(uint8_t sw1) {
  switch ((sw1 & 0x3c) >> 2) {
    case 8:
      return 0;
    case 9:
      return 1;
    case 1:
      return 2;
    case 5:
      return 3;
    case 4:
      return 4;
    case 6:
      return 5;
    case 2:
      return 6;
    case 10:
      return 7;
    default:
      return 15;
  }
}

void dual_hid_init() {
  static struct usb_device device;
  device.get_descriptor_size = get_descriptor_size;
  device.get_descriptor = get_descriptor;
  device.ep_in = get_report;
  usb_device_init(&device, UD_USE_EP1 | UD_USE_EP2);
}

void dual_hid_update(uint8_t sws[5], bool csw1, bool csw2) {
  hold_csw1 |= csw1;
  hold_csw2 |= csw2;
  report[0] =
      ((sws[1] & 0x02) ? 0x01 : 0) |
      ((sws[1] & 0x01) ? 0x02 : 0) |
      ((sws[2] & 0x80) ? 0x04 : 0) |
      ((sws[2] & 0x40) ? 0x08 : 0) |
      ((sws[2] & 0x20) ? 0x10 : 0) |
      ((sws[2] & 0x10) ? 0x20 : 0) |
      ((sws[2] & 0x08) ? 0x40 : 0) |
      ((sws[2] & 0x04) ? 0x80 : 0);
  report[1] =
      ((sws[2] & 0x02) ? 0x01 : 0) |
      ((sws[2] & 0x01) ? 0x02 : 0) |
      ((sws[0] & 0x80) ? 0x04 : 0) |
      ((sws[1] & 0x40) ? 0x08 : 0) |
      (hold_csw1       ? 0x10 : 0) |
      ((sws[1] & 0x80) ? 0x20 : 0);
  report[2] = calc_hat(sws[1]);
  report[3] =
      ((sws[3] & 0x02) ? 0x01 : 0) |
      ((sws[3] & 0x01) ? 0x02 : 0) |
      ((sws[4] & 0x80) ? 0x04 : 0) |
      ((sws[4] & 0x40) ? 0x08 : 0) |
      ((sws[4] & 0x20) ? 0x10 : 0) |
      ((sws[4] & 0x10) ? 0x20 : 0) |
      ((sws[4] & 0x08) ? 0x40 : 0) |
      ((sws[4] & 0x04) ? 0x80 : 0);
  report[4] =
      ((sws[3] & 0x02) ? 0x01 : 0) |
      ((sws[3] & 0x01) ? 0x02 : 0) |
      ((sws[3] & 0x40) ? 0x08 : 0) |
      (hold_csw2       ? 0x10 : 0) |
      ((sws[3] & 0x80) ? 0x20 : 0);
  report[5] = calc_hat(sws[3]);
}