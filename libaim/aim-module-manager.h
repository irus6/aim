/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-module-manager.h
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

#ifndef __AIM_MODULE_MANAGER_H__
#define __AIM_MODULE_MANAGER_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define AIM_TYPE_MODULE_MANAGER             (aim_module_manager_get_type ())
#define AIM_MODULE_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_TYPE_MODULE_MANAGER, AimModuleManager))
#define AIM_MODULE_MANAGER_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), AIM_TYPE_MODULE_MANAGER, AimModuleManagerClass))
#define AIM_IS_MODULE_MANAGER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AIM_TYPE_MODULE_MANAGER))
#define AIM_IS_MODULE_MANAGER_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), AIM_TYPE_MODULE_MANAGER))
#define AIM_MODULE_MANAGER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_TYPE_MODULE_MANAGER, AimModuleManagerClass))

typedef struct _AimModuleManager      AimModuleManager;
typedef struct _AimModuleManagerClass AimModuleManagerClass;

struct _AimModuleManager
{
  GObject parent_instance;

  GHashTable *modules;
};

struct _AimModuleManagerClass
{
  GObjectClass parent_class;
};

GType             aim_module_manager_get_type    (void) G_GNUC_CONST;
AimModuleManager *aim_module_manager_get_default (void);

G_END_DECLS

#endif /* __AIM_MODULE_MANAGER_H__ */
