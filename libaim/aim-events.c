/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-events.c
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

#include "aim-events.h"
#include "aim-types.h"
#include "aim-key-syms.h"
#include <string.h>

typedef struct _aim_mod_info AimModifierInfo;

struct _aim_mod_info {
  gchar *name;
  AimModifierType mod;
};

static const AimModifierInfo mod_info_list[] = {
  {"Shift",    AIM_SHIFT_MASK},
  {"Lock",     AIM_LOCK_MASK},
  {"Control",  AIM_CONTROL_MASK},
  {"Mod1",     AIM_MOD1_MASK},
  {"Mod2",     AIM_MOD2_MASK}, /* Num Lock */
  {"Mod3",     AIM_MOD3_MASK},
  {"Mod4",     AIM_MOD4_MASK},
  {"Mod5",     AIM_MOD5_MASK},
  {"Button1",  AIM_BUTTON1_MASK},
  {"Button2",  AIM_BUTTON2_MASK},
  {"Button3",  AIM_BUTTON3_MASK},
  {"Button4",  AIM_BUTTON4_MASK},
  {"Button5",  AIM_BUTTON5_MASK},
  {"Super",    AIM_SUPER_MASK},
  {"Hyper",    AIM_HYPER_MASK},
  {"Meta",     AIM_META_MASK},
  {"Release",  AIM_RELEASE_MASK}
};

gboolean
aim_event_matches (AimEvent *event, const AimKey **keys)
{
  g_debug (G_STRLOC ": %s: event->key.state: %d", G_STRFUNC, event->key.state);

  gboolean retval = FALSE;
  gint i;

  /* When pressing Alt key, some programs generate AIM_META_MASK,
   * while some programs don't generate AIM_META_MASK.
   * AIM_MOD2_MASK related to Number key */
  guint mods = event->key.state & (AIM_MOD2_MASK | AIM_META_MASK |
                                   AIM_LOCK_MASK | AIM_RELEASE_MASK);

  for (i = 0; keys[i] != 0; i++)
  {
    if ((event->key.state & AIM_MODIFIER_MASK) == (keys[i]->mods | mods) &&
        event->key.keyval == keys[i]->keyval)
    {
      retval = TRUE;
      break;
    }
  }

  return retval;
}

AimEvent *
aim_event_new (AimEventType type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimEvent *new_event = g_slice_new0 (AimEvent);
  new_event->type = type;

  return new_event;
}

void
aim_event_free (AimEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (event != NULL);

  g_slice_free (AimEvent, event);
}

AimEvent *
aim_event_copy (AimEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (event != NULL, NULL);

  AimEvent *new_event;
  new_event = aim_event_new (AIM_EVENT_NOTHING);
  *new_event = *event;

  return new_event;
}

G_DEFINE_BOXED_TYPE (AimEvent, aim_event, aim_event_copy, aim_event_free)
