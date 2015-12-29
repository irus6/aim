/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-im.h
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

#ifndef __AIM_IM_H__
#define __AIM_IM_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>
#include <gio/gio.h>
#include "aim-events.h"
#include "aim-engine.h"
#include "aim-message.h"
#include "aim-private.h"

G_BEGIN_DECLS

#define AIM_TYPE_IM             (aim_im_get_type ())
#define AIM_IM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_TYPE_IM, AimIM))
#define AIM_IM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AIM_TYPE_IM, AimIMClass))
#define AIM_IS_IM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AIM_TYPE_IM))
#define AIM_IS_IM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AIM_TYPE_IM))
#define AIM_IM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_TYPE_IM, AimIMClass))

typedef struct _AimIM      AimIM;
typedef struct _AimIMClass AimIMClass;

struct _AimIM
{
  GObject parent_instance;

  AimEngine         *engine;
  GSocketConnection *connection;
  AimResult         *result;
  GSource           *sockets_context_source;
  GSource           *default_context_source;
  gchar             *preedit_string;
  gint               cursor_pos;
};

struct _AimIMClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Signals */
  void     (*preedit_start)        (AimIM *im);
  void     (*preedit_end)          (AimIM *im);
  void     (*preedit_changed)      (AimIM *im);
  void     (*commit)               (AimIM *im, const gchar *str);
  gboolean (*retrieve_surrounding) (AimIM *im);
  gboolean (*delete_surrounding)   (AimIM *im,
                                    gint   offset,
                                    gint   n_chars);
};

GType     aim_im_get_type            (void) G_GNUC_CONST;
AimIM    *aim_im_new                 (void);
void      aim_im_focus_in            (AimIM             *im);
void      aim_im_focus_out           (AimIM             *im);
void      aim_im_reset               (AimIM             *im);
gboolean  aim_im_filter_event        (AimIM             *im,
                                      AimEvent          *event);
void      aim_im_get_preedit_string  (AimIM             *im,
                                      gchar            **str,
                                      gint              *cursor_pos);
void      aim_im_set_cursor_location (AimIM             *im,
                                      const AimRectangle *area);
void      aim_im_set_use_preedit     (AimIM             *im,
                                      gboolean           use_preedit);
gboolean  aim_im_get_surrounding     (AimIM             *im,
                                      gchar            **text,
                                      gint              *cursor_index);
void      aim_im_set_surrounding     (AimIM             *im,
                                      const char        *text,
                                      gint               len,
                                      gint               cursor_index);

G_END_DECLS

#endif /* __AIM_IM_H__ */
