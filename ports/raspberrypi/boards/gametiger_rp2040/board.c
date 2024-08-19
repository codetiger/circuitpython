// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2021 Scott Shawcroft for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "supervisor/board.h"
#include "mpconfigboard.h"
#include "shared-bindings/busio/SPI.h"
#include "shared-bindings/fourwire/FourWire.h"
#include "shared-module/displayio/__init__.h"
#include "shared-module/displayio/mipi_constants.h"
#include "supervisor/shared/board.h"

// Use the MP_WEAK supervisor/shared/board.c versions of routines not defined here.
#define DELAY 0x80

uint8_t display_init_sequence[] = {
    0x01, 0 | DELAY, 120, // SWRESET
    0x11, 0 | DELAY, 120, // SWRESET

    0x36, 1, 0x70, // MADCTL
    0x3A, 1, 0x55, // COLMOD
    0xB2, 5, 0x0C, 0x0C, 0x00, 0x33, 0x33, // FRMCTR2
    0xB7, 1, 0x75, // GCTRL
    0xBB, 1, 0x2B, // VCOMS
    0xC0, 1, 0x2C, // LCMCTRL
    0xC2, 1, 0x01, // VDVVRHEN
    0xC3, 1, 0x0B, // VRHS
    0xC4, 1, 0x20, // VDVS
    0xC6, 1, 0x0F, // FRCTRL2
    0xD0, 2, 0xA4, 0xA1, // PWRCTRL1

    0xE0, 14, 0xD0, 0x01, 0x04, 0x09, 0x0B, 0x07, 0x2E, 0x44, 0x43, 0x0B, 0x16, 0x15, 0x17, 0x1D, // GMCTRP1
    0xE1, 14, 0xD0, 0x01, 0x05, 0x0A, 0x0B, 0x08, 0x2F, 0x44, 0x41, 0x0A, 0x15, 0x14, 0x19, 0x1D, // GMCTRN1
    0x29, 0 | DELAY, 120, // DISPON

    // 0x2A, 4, 0x00, 0, 0x01, 0x3F, // CASET
    // 0x2B, 4, 0x00, 0, 0x00, 0xEF, // RASET
    // 0x2C, 0, // RAMWR
};

static void display_init(void) {
    fourwire_fourwire_obj_t *bus = &allocate_display_bus()->fourwire_bus;
    busio_spi_obj_t *spi = &bus->inline_bus;

    common_hal_busio_spi_construct(
        spi,
        &pin_GPIO2,    // CLK
        &pin_GPIO3,    // MOSI
        &pin_GPIO0,    // MISO
        false);         // Not half-duplex

    common_hal_busio_spi_never_reset(spi);

    bus->base.type = &fourwire_fourwire_type;

    common_hal_fourwire_fourwire_construct(
        bus,
        spi,
        &pin_GPIO0,    // DC
        &pin_GPIO1,    // CS
        &pin_GPIO4,     // RST
        62.5 * 1000 * 1000,       // baudrate
        0,              // polarity
        0               // phase
        );

    busdisplay_busdisplay_obj_t *display = &allocate_display()->display;
    display->base.type = &busdisplay_busdisplay_type;

    common_hal_busdisplay_busdisplay_construct(
        display,
        bus,
        320,            // width (after rotation)
        240,            // height (after rotation)
        0,              // column start
        0,             // row start
        0,              // rotation
        16,             // color depth
        false,          // grayscale
        false,          // pixels in a byte share a row. Only valid for depths < 8
        1,              // bytes per cell. Only valid for depths < 8
        false,          // reverse_pixels_in_byte. Only valid for depths < 8
        true,           // reverse_pixels_in_word
        MIPI_COMMAND_SET_COLUMN_ADDRESS, // set column command
        MIPI_COMMAND_SET_PAGE_ADDRESS,   // set row command
        MIPI_COMMAND_WRITE_MEMORY_START, // write memory command
        display_init_sequence,
        sizeof(display_init_sequence),
        NULL,    // backlight pin
        NO_BRIGHTNESS_COMMAND,
        1.0f,           // brightness
        false,          // single_byte_bounds
        false,          // data_as_commands
        true,           // auto_refresh
        60,             // native_frames_per_second
        true,           // backlight_on_high
        false,          // SH1107_addressing
        50000           // backlight pwm frequency
        );

}

void board_init(void) {
    display_init();
}
