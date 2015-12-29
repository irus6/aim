/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-english.c
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

#include "aim-english.h"
#include "aim-key-syms.h"

G_DEFINE_TYPE (AimEnglish, aim_english, AIM_TYPE_ENGINE);

void
aim_english_reset (AimEngine *engine, AimConnection  *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

const gchar *
aim_english_get_id (AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return AIM_ENGLISH (engine)->id;
}

const gchar *
aim_english_get_name (AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return AIM_ENGLISH (engine)->name;
}

gboolean
aim_english_filter_event (AimEngine     *engine,
                          AimConnection *target,
                          AimEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;

  if ((event->key.type == AIM_EVENT_KEY_RELEASE) ||
      (event->key.keyval == AIM_KEY_Shift_L)     ||
      (event->key.keyval == AIM_KEY_Shift_R)     ||
      (event->key.state & (AIM_CONTROL_MASK | AIM_MOD1_MASK)))
    return FALSE;

  gchar c = 0;

  if (event->key.keyval >= 32 && event->key.keyval <= 126)
    c = event->key.keyval;

  if (!c)
  {
    switch (event->key.keyval)
    {
      case AIM_KEY_KP_Multiply: c = '*'; break;
      case AIM_KEY_KP_Add:      c = '+'; break;
      case AIM_KEY_KP_Subtract: c = '-'; break;
      case AIM_KEY_KP_Divide:   c = '/'; break;
      default:
        break;
    }
  }

  if (!c && (event->key.state & AIM_MOD2_MASK))
  {
    switch (event->key.keyval)
    {
      case AIM_KEY_KP_Decimal:  c = '.'; break;
      case AIM_KEY_KP_0:        c = '0'; break;
      case AIM_KEY_KP_1:        c = '1'; break;
      case AIM_KEY_KP_2:        c = '2'; break;
      case AIM_KEY_KP_3:        c = '3'; break;
      case AIM_KEY_KP_4:        c = '4'; break;
      case AIM_KEY_KP_5:        c = '5'; break;
      case AIM_KEY_KP_6:        c = '6'; break;
      case AIM_KEY_KP_7:        c = '7'; break;
      case AIM_KEY_KP_8:        c = '8'; break;
      case AIM_KEY_KP_9:        c = '9'; break;
      default:
        break;
    }
  }

  if (c)
  {
    gchar *str = g_strdup_printf ("%c", c);
    aim_engine_emit_commit (engine, target, str);
    g_free (str);
    retval = TRUE;
  }

  return retval;
}

static void
aim_english_init (AimEnglish *english)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  english->id   = g_strdup ("aim-english");
  english->name = g_strdup ("en");
}

static void
aim_english_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimEnglish *english = AIM_ENGLISH (object);
  g_free (english->id);
  g_free (english->name);

  G_OBJECT_CLASS (aim_english_parent_class)->finalize (object);
}

static void
aim_english_class_init (AimEnglishClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  AimEngineClass *engine_class = AIM_ENGINE_CLASS (klass);

  engine_class->filter_event = aim_english_filter_event;
  engine_class->reset        = aim_english_reset;
  engine_class->get_id       = aim_english_get_id;
  engine_class->get_name     = aim_english_get_name;

  object_class->finalize = aim_english_finalize;
}
