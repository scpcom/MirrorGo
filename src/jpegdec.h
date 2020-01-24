/***************************************************
  This library is written to be compatible with Adafruit's CLCDMirror
  library and automatically detects the display type on ESP_WROVER_KITs
  Earlier WROVERs had CLCDMirror, while newer releases have ST7789V

  MIT license, all text above must be included in any redistribution
 ****************************************************/

/*
 * JPEG
 * */

#include <Arduino.h>
#include "FS.h"
#include <Print.h>

//include "rom/tjpgd.h"
#include <stdio.h>
#include "rom/tjpgd.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//include "unity.h"

#define jpgColor(c)                                                            \
  (((uint16_t)(((uint8_t *)(c))[0] & 0xF8) << 8) |                             \
   ((uint16_t)(((uint8_t *)(c))[1] & 0xFC) << 3) |                             \
   ((((uint8_t *)(c))[2] & 0xF8) >> 3))

#ifndef _DISPLAY_H_
typedef enum {
  JPEG_DIV_NONE,
  JPEG_DIV_2,
  JPEG_DIV_4,
  JPEG_DIV_8,
  JPEG_DIV_MAX
} jpeg_div_t;
#endif

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t maxWidth;
  uint16_t maxHeight;
  uint16_t offX;
  uint16_t offY;
  jpeg_div_t scale;
  const void *src;
  size_t len;
  size_t index;
  Print *tft;
  uint16_t outWidth;
  uint16_t outHeight;
} jpg_file_decoder_t;

uint32_t jpgReadFile(JDEC *decoder, uint8_t *buf, uint32_t len);
uint32_t jpgRead(JDEC *decoder, uint8_t *buf, uint32_t len);
bool jpgDecode(jpg_file_decoder_t *jpeg,
                      uint32_t (*reader)(JDEC *, uint8_t *, uint32_t),
                      uint32_t (*writer)(JDEC *, void *, JRECT *));

