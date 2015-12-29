/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-events.h
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

#ifndef __AIM_EVENTS_H__
#define __AIM_EVENTS_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>
#include "aim-types.h"

G_BEGIN_DECLS

#define AIM_TYPE_EVENT (aim_event_get_type ())

typedef struct _AimEventKey AimEventKey;
typedef union  _AimEvent    AimEvent;

typedef enum
{
  AIM_EVENT_NOTHING     = -1,
  AIM_EVENT_KEY_PRESS   =  0,
  AIM_EVENT_KEY_RELEASE =  1,
} AimEventType;

struct _AimEventKey
{
  AimEventType type;
  guint        state;
  guint        keyval;
  guint16      hardware_keycode;
};

union _AimEvent
{
  AimEventType type;
  AimEventKey  key;
};

GType     aim_event_get_type   (void) G_GNUC_CONST;
AimEvent *aim_event_new        (AimEventType   type);
AimEvent *aim_event_copy       (AimEvent      *event);
void      aim_event_free       (AimEvent      *event);
gboolean  aim_event_matches    (AimEvent      *event,
                                const AimKey **keys);

G_END_DECLS

#endif /* __AIM_EVENTS_H__ */
