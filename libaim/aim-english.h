/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-english.h
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

#ifndef __AIM_ENGLISH_H__
#define __AIM_ENGLISH_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>
#include "aim-engine.h"

G_BEGIN_DECLS

#define AIM_TYPE_ENGLISH             (aim_english_get_type ())
#define AIM_ENGLISH(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_TYPE_ENGLISH, AimEnglish))
#define AIM_ENGLISH_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AIM_TYPE_ENGLISH, AimEnglishClass))
#define AIM_IS_ENGLISH(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AIM_TYPE_ENGLISH))
#define AIM_IS_ENGLISH_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AIM_TYPE_ENGLISH))
#define AIM_ENGLISH_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_TYPE_ENGLISH, AimEnglishClass))

typedef struct _AimEnglish      AimEnglish;
typedef struct _AimEnglishClass AimEnglishClass;

struct _AimEnglish
{
  AimEngine parent_instance;

  gchar *id;
  gchar *name;
};

struct _AimEnglishClass
{
  /*< private >*/
  AimEngineClass parent_class;
};

GType    aim_english_get_type     (void) G_GNUC_CONST;
gboolean aim_english_filter_event (AimEngine     *engine,
                                   AimConnection *connection,
                                   AimEvent      *event);
G_END_DECLS

#endif /* __AIM_ENGLISH_H__ */
