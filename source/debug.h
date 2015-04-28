/*
 * debug.h
 *
 * This module is licensed under the GNU General Public License
 * Version 2.  Please see the file "COPYING" in this directory for
 * more information about the GNU General Public License Version 2.
 *
 *     Copyright (C) 2015  Kevin Lamonte
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

/* Includes --------------------------------------------------------------- */

#include "emulation.h"

/* Defines ---------------------------------------------------------------- */

/* Globals ---------------------------------------------------------------- */

/* Functions -------------------------------------------------------------- */

extern Q_EMULATION_STATUS debug_emulator(const unsigned char from_modem, wchar_t * to_screen);
extern void debug_reset();
extern void debug_finish();
extern void debug_local_echo(const unsigned char ch);

#endif /* __DEBUG_H__ */