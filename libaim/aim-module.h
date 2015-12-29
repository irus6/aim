/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-module.h
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

#ifndef __AIM_MODULE_H__
#define __AIM_MODULE_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>
#include <gmodule.h>
#include "aim-engine.h"

G_BEGIN_DECLS

#define AIM_TYPE_MODULE             (aim_module_get_type ())
#define AIM_MODULE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_TYPE_MODULE, AimModule))
#define AIM_MODULE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AIM_TYPE_MODULE, AimModuleClass))
#define AIM_IS_MODULE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AIM_TYPE_MODULE))
#define AIM_IS_MODULE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AIM_TYPE_MODULE))
#define AIM_MODULE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_TYPE_MODULE, AimModuleClass))

typedef struct _AimModule      AimModule;
typedef struct _AimModuleClass AimModuleClass;

struct _AimModule
{
  GTypeModule parent_instance;

  char     *path;
  GModule  *library;
  GType     type;

  void  (* load)     (GTypeModule *module);
  GType (* get_type) (void);
  void  (* unload)   (void);
};

struct _AimModuleClass
{
  GTypeModuleClass parent_class;
};

GType      aim_module_get_type (void) G_GNUC_CONST;
AimModule *aim_module_new      (const gchar *path);

G_END_DECLS

#endif /* __AIM_MODULE_H__ */
