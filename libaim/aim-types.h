/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-types.h
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

#ifndef __AIM_TYPES_H__
#define __AIM_TYPES_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define AIM_ADDRESS "unix:abstract=aim"
#define AIM_ERROR   aim_error_quark ()

typedef enum
{
  AIM_ERROR_FAILED
} AimError;

typedef enum
{
  AIM_CONNECTION_AIM_IM,
  AIM_CONNECTION_AIM_AGENT,
  AIM_CONNECTION_XIM
} AimConnectionType;

/* copied from GdkModifierType in gdktypes.h for compatibility */
typedef enum
{
  AIM_SHIFT_MASK    = 1 << 0,
  AIM_LOCK_MASK     = 1 << 1,
  AIM_CONTROL_MASK  = 1 << 2,
  AIM_MOD1_MASK     = 1 << 3,
  AIM_MOD2_MASK     = 1 << 4,
  AIM_MOD3_MASK     = 1 << 5,
  AIM_MOD4_MASK     = 1 << 6,
  AIM_MOD5_MASK     = 1 << 7,
  AIM_BUTTON1_MASK  = 1 << 8,
  AIM_BUTTON2_MASK  = 1 << 9,
  AIM_BUTTON3_MASK  = 1 << 10,
  AIM_BUTTON4_MASK  = 1 << 11,
  AIM_BUTTON5_MASK  = 1 << 12,

  AIM_MODIFIER_RESERVED_13_MASK  = 1 << 13,
  AIM_MODIFIER_RESERVED_14_MASK  = 1 << 14,
  AIM_MODIFIER_RESERVED_15_MASK  = 1 << 15,
  AIM_MODIFIER_RESERVED_16_MASK  = 1 << 16,
  AIM_MODIFIER_RESERVED_17_MASK  = 1 << 17,
  AIM_MODIFIER_RESERVED_18_MASK  = 1 << 18,
  AIM_MODIFIER_RESERVED_19_MASK  = 1 << 19,
  AIM_MODIFIER_RESERVED_20_MASK  = 1 << 20,
  AIM_MODIFIER_RESERVED_21_MASK  = 1 << 21,
  AIM_MODIFIER_RESERVED_22_MASK  = 1 << 22,
  AIM_MODIFIER_RESERVED_23_MASK  = 1 << 23,
  AIM_MODIFIER_RESERVED_24_MASK  = 1 << 24,
  AIM_MODIFIER_RESERVED_25_MASK  = 1 << 25,

  AIM_SUPER_MASK    = 1 << 26,
  AIM_HYPER_MASK    = 1 << 27,
  AIM_META_MASK     = 1 << 28,

  AIM_MODIFIER_RESERVED_29_MASK  = 1 << 29,

  AIM_RELEASE_MASK  = 1 << 30,

  /* Combination of AIM_SHIFT_MASK..AIM_BUTTON5_MASK + AIM_SUPER_MASK
     + AIM_HYPER_MASK + AIM_META_MASK + AIM_RELEASE_MASK */
  AIM_MODIFIER_MASK = 0x5c001fff
} AimModifierType;

typedef struct {
  int x, y;
  int width, height;
} AimRectangle;

typedef struct {
  guint mods;
  guint keyval;
} AimKey;

typedef enum
{
  AIM_PREEDIT_STATE_START = 1,
  AIM_PREEDIT_STATE_END   = 0
} AimPreeditState;

GQuark   aim_error_quark        (void);
AimKey  *aim_key_new            (void);
AimKey  *aim_key_new_from_nicks (const gchar **nicks);
void     aim_key_free           (AimKey       *key);
AimKey **aim_key_newv           (const gchar **keys);
void     aim_key_freev          (AimKey      **keys);

G_END_DECLS

#endif /* __AIM_TYPES_H__ */
