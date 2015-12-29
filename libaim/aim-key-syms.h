/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-key-syms.h
 * This file is part of AIM.
 *
 * Copyright (C) 2015 Hodong Kim <hodong@cogno.org>
 *
 * AIM is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AIM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program;  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __AIM_KEY_SYMS_H__
#define __AIM_KEY_SYMS_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

typedef enum
{
  AIM_KEY_space        = 0x0020,

  AIM_KEY_0            = 0x030,
  AIM_KEY_1            = 0x031,
  AIM_KEY_2            = 0x032,
  AIM_KEY_3            = 0x033,
  AIM_KEY_4            = 0x034,
  AIM_KEY_5            = 0x035,
  AIM_KEY_6            = 0x036,
  AIM_KEY_7            = 0x037,
  AIM_KEY_8            = 0x038,
  AIM_KEY_9            = 0x039,

  AIM_KEY_h            = 0x068,

  AIM_KEY_j            = 0x06a,
  AIM_KEY_k            = 0x06b,
  AIM_KEY_l            = 0x06c,

  AIM_KEY_BackSpace    = 0xff08,
  AIM_KEY_Return       = 0xff0d,

  AIM_KEY_Escape       = 0xff1b,

  AIM_KEY_Hangul       = 0xff31,

  AIM_KEY_Hangul_Hanja = 0xff34,

  AIM_KEY_Left         = 0xff51,
  AIM_KEY_Up           = 0xff52,
  AIM_KEY_Right        = 0xff53,
  AIM_KEY_Down         = 0xff54,
  AIM_KEY_Page_Up      = 0xff55,
  AIM_KEY_Page_Down    = 0xff56,

  AIM_KEY_KP_Enter     = 0xff8d,

  AIM_KEY_KP_Left      = 0xff96,
  AIM_KEY_KP_Up        = 0xff97,
  AIM_KEY_KP_Right     = 0xff98,
  AIM_KEY_KP_Down      = 0xff99,
  AIM_KEY_KP_Page_Up   = 0xff9a,
  AIM_KEY_KP_Page_Down = 0xff9b,

  AIM_KEY_KP_Delete    = 0xff9f,

  AIM_KEY_KP_Multiply  = 0xffaa,
  AIM_KEY_KP_Add       = 0xffab,

  AIM_KEY_KP_Subtract  = 0xffad,
  AIM_KEY_KP_Decimal   = 0xffae,
  AIM_KEY_KP_Divide    = 0xffaf,
  AIM_KEY_KP_0         = 0xffb0,
  AIM_KEY_KP_1         = 0xffb1,
  AIM_KEY_KP_2         = 0xffb2,
  AIM_KEY_KP_3         = 0xffb3,
  AIM_KEY_KP_4         = 0xffb4,
  AIM_KEY_KP_5         = 0xffb5,
  AIM_KEY_KP_6         = 0xffb6,
  AIM_KEY_KP_7         = 0xffb7,
  AIM_KEY_KP_8         = 0xffb8,
  AIM_KEY_KP_9         = 0xffb9,

  AIM_KEY_F1           = 0xffbe,
  AIM_KEY_F2           = 0xffbf,
  AIM_KEY_F3           = 0xffc0,
  AIM_KEY_F4           = 0xffc1,
  AIM_KEY_F5           = 0xffc2,
  AIM_KEY_F6           = 0xffc3,
  AIM_KEY_F7           = 0xffc4,
  AIM_KEY_F8           = 0xffc5,
  AIM_KEY_F9           = 0xffc6,
  AIM_KEY_F10          = 0xffc7,
  AIM_KEY_F11          = 0xffc8,
  AIM_KEY_F12          = 0xffc9,

  AIM_KEY_Shift_L      = 0xffe1,
  AIM_KEY_Shift_R      = 0xffe2,
  AIM_KEY_Control_L    = 0xffe3,
  AIM_KEY_Control_R    = 0xffe4,

  AIM_KEY_Meta_L       = 0xffe7,
  AIM_KEY_Meta_R       = 0xffe8,
  AIM_KEY_Alt_L        = 0xffe9,
  AIM_KEY_Alt_R        = 0xffea,

  AIM_KEY_Delete       = 0xffff,

  AIM_KEY_VoidSymbol   = 0xffffff
} AimKeySym;

#endif /* __AIM_KEY_SYMS_H__ */
