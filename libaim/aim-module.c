/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-module.c
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

#include "config.h"
#include "aim-module.h"
#include <gio/gio.h>

G_DEFINE_TYPE (AimModule, aim_module, G_TYPE_TYPE_MODULE);

AimModule *
aim_module_new (const gchar *path)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (path != NULL, NULL);

  AimModule *module = g_object_new (AIM_TYPE_MODULE, NULL);

  module->path = g_strdup (path);

  return module;
}

static gboolean
aim_module_load (GTypeModule *gmodule)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimModule *module = AIM_MODULE (gmodule);

  module->library = g_module_open (module->path, G_MODULE_BIND_LAZY |
                                                 G_MODULE_BIND_LOCAL);

  if (!module->library)
  {
    g_warning (G_STRLOC ": %s", g_module_error ());
    return FALSE;
  }

  if (!g_module_symbol (module->library,
                        "module_load",
                        (gpointer *) &module->load) ||
      !g_module_symbol (module->library,
                        "module_get_type",
                        (gpointer *) &module->get_type) ||
      !g_module_symbol (module->library,
                        "module_unload",
                        (gpointer *) &module->unload))
  {
    g_warning (G_STRLOC ": %s", g_module_error ());
    g_module_close (module->library);

    return FALSE;
  }

  module->load (gmodule);
  module->type = module->get_type ();

  return TRUE;
}

static void
aim_module_unload (GTypeModule *gmodule)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimModule *module = AIM_MODULE (gmodule);

  module->unload ();

  g_module_close (module->library);

  module->load   = NULL;
  module->unload = NULL;
}

static void
aim_module_init (AimModule *module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static void
aim_module_class_init (AimModuleClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS (klass);

  module_class->load   = aim_module_load;
  module_class->unload = aim_module_unload;
}
