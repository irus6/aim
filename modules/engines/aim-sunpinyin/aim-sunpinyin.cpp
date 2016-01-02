/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-sunpinyin.cpp
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

#include <aim.h>
#include <sunpinyin.h>

G_BEGIN_DECLS

class AimWinHandler : public CIMIWinHandler
{
public:
  AimWinHandler(AimEngine *engine);

  virtual ~AimWinHandler()
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);
  }

  virtual void commit(const TWCHAR* wstr);
  virtual void updatePreedit(const IPreeditString* ppd);
  virtual void updateCandidates(const ICandidateList* pcl);
  virtual void updateStatus(int key, int value);

  void set_target(AimConnection *target);

private:
  AimEngine     *m_engine;
  AimConnection *m_target;
};

#define AIM_TYPE_SUNPINYIN             (aim_sunpinyin_get_type ())
#define AIM_SUNPINYIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_TYPE_SUNPINYIN, AimSunpinyin))
#define AIM_SUNPINYIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AIM_TYPE_SUNPINYIN, AimSunpinyinClass))
#define AIM_IS_SUNPINYIN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AIM_TYPE_SUNPINYIN))
#define AIM_IS_SUNPINYIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AIM_TYPE_SUNPINYIN))
#define AIM_SUNPINYIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_TYPE_SUNPINYIN, AimSunpinyinClass))

typedef struct _AimSunpinyin      AimSunpinyin;
typedef struct _AimSunpinyinClass AimSunpinyinClass;

struct _AimSunpinyin
{
  AimEngine parent_instance;

  gchar           *id;
  gchar           *zh_name;
  gchar           *en_name;
  gboolean         is_english_mode;
  gchar           *preedit_string;
  AimPreeditState  preedit_state;

  GSettings       *settings;
  AimKey         **zh_en_keys;

  CIMIView       *view;
  CHotkeyProfile *hotkey_profile;
  AimWinHandler  *win_handler;
};

struct _AimSunpinyinClass
{
  /*< private >*/
  AimEngineClass parent_class;
};

GType aim_sunpinyin_get_type (void) G_GNUC_CONST;

AimWinHandler::AimWinHandler(AimEngine *engine)
  : m_engine(engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
AimWinHandler::commit(const TWCHAR* wstr)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (wstr)
  {
    gchar *text = g_ucs4_to_utf8 (wstr, -1, NULL, NULL, NULL);
    aim_engine_emit_commit (m_engine, m_target, text);
    g_free (text);
  }
}

static void
aim_sunpinyin_update_preedit (AimEngine     *engine,
                              AimConnection *target,
                              gchar         *new_preedit,
                              int            cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimSunpinyin *pinyin = AIM_SUNPINYIN (engine);

  /* preedit-start */
  if (pinyin->preedit_state == AIM_PREEDIT_STATE_END && new_preedit[0] != 0)
  {
    pinyin->preedit_state = AIM_PREEDIT_STATE_START;
    aim_engine_emit_preedit_start (engine, target);
  }

  /* preedit-changed */
  if (pinyin->preedit_string[0] != 0 || new_preedit[0] != 0)
  {
    g_free (pinyin->preedit_string);
    pinyin->preedit_string = new_preedit;
    aim_engine_emit_preedit_changed (engine, target,
                                     pinyin->preedit_string, cursor_pos);
  }
  else
    g_free (new_preedit);

  /* preedit-end */
  if (pinyin->preedit_state == AIM_PREEDIT_STATE_START &&
      pinyin->preedit_string[0] == 0)
  {
    pinyin->preedit_state = AIM_PREEDIT_STATE_END;
    aim_engine_emit_preedit_end (engine, target);
  }
}

void
AimWinHandler::updatePreedit(const IPreeditString* ppd)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimSunpinyin *pinyin;

  if (ppd)
  {
    if (G_UNLIKELY (ppd->size() >= 1019))
    {
      pinyin = AIM_SUNPINYIN (m_engine);
      pinyin->view->updateWindows(pinyin->view->clearIC());
    }

    const TWCHAR *wstr = ppd->string();
    /* aim_sunpinyin_update_preedit takes text */
    aim_sunpinyin_update_preedit (m_engine, m_target,
                                  g_ucs4_to_utf8 (wstr, -1, NULL, NULL, NULL),
                                  ppd->caret());
  }
}

void
AimWinHandler::updateCandidates(const ICandidateList* pcl)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (pcl)
  {
    const TWCHAR *wstr;
    gchar **strv = (gchar **) g_malloc0 ((pcl->size() + 1) * sizeof (gchar *));
    gint i;

    for (i = 0; i < pcl->size(); i++)
    {
      wstr = pcl->candiString(i);

      if (wstr)
      {
        gchar *text = g_ucs4_to_utf8 (wstr, -1, NULL, NULL, NULL);
        strv[i] = text;
      }
    }

    aim_engine_update_candidate_window (m_engine, (const gchar **) strv);
    g_strfreev (strv);

    if (pcl->size() > 0)
      aim_engine_show_candidate_window (m_engine, m_target);
    else
      aim_engine_hide_candidate_window (m_engine);
  }
}

void
AimWinHandler::updateStatus(int key, int value)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
AimWinHandler::set_target(AimConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  m_target = target;
}

G_DEFINE_DYNAMIC_TYPE (AimSunpinyin, aim_sunpinyin, AIM_TYPE_ENGINE);

static void
aim_sunpinyin_init (AimSunpinyin *pinyin)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar **zh_en_keys;

  pinyin->settings = g_settings_new ("org.aim.engines.sunpinyin");
  zh_en_keys = g_settings_get_strv (pinyin->settings, "zh-en-keys");
  pinyin->zh_en_keys = aim_key_newv ((const gchar **) zh_en_keys);
  g_strfreev (zh_en_keys);

  pinyin->id = g_strdup ("aim-sunpinyin");
  pinyin->zh_name = g_strdup ("zh");
  pinyin->en_name = g_strdup ("en");
  pinyin->is_english_mode = TRUE;
  pinyin->preedit_string = g_strdup ("");

  CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
  factory.setPinyinScheme(CSunpinyinSessionFactory::QUANPIN);
  factory.setCandiWindowSize(10);
  pinyin->view = factory.createSession();

  if (!pinyin->view)
  {
    g_warning (G_STRLOC ": %s: factory.createSession() failed", G_STRFUNC);
    return;
  }

  pinyin->hotkey_profile = new CHotkeyProfile();
  pinyin->view->setHotkeyProfile(pinyin->hotkey_profile);

  pinyin->win_handler = new AimWinHandler(AIM_ENGINE (pinyin));
  pinyin->view->attachWinHandler(pinyin->win_handler);
}

static void
aim_sunpinyin_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimSunpinyin *pinyin = AIM_SUNPINYIN (object);

  g_free (pinyin->id);
  g_free (pinyin->zh_name);
  g_free (pinyin->en_name);
  g_free (pinyin->preedit_string);
  aim_key_freev (pinyin->zh_en_keys);
  g_object_unref (pinyin->settings);

  if (pinyin->view)
  {
    CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
    factory.destroySession(pinyin->view);
  }

  delete pinyin->win_handler;
  delete pinyin->hotkey_profile;

  G_OBJECT_CLASS (aim_sunpinyin_parent_class)->finalize (object);
}

const gchar *
aim_sunpinyin_get_id (AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (AIM_IS_ENGINE (engine), NULL);

  return AIM_SUNPINYIN (engine)->id;
}

const gchar *
aim_sunpinyin_get_name (AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (AIM_IS_ENGINE (engine), NULL);

  AimSunpinyin *pinyin = AIM_SUNPINYIN (engine);

  return pinyin->is_english_mode ? pinyin->en_name : pinyin->zh_name;
}

void
aim_sunpinyin_reset (AimEngine *engine, AimConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_ENGINE (engine));

  AimSunpinyin *pinyin = AIM_SUNPINYIN (engine);

  pinyin->view->updateWindows(pinyin->view->clearIC());
}

void
aim_sunpinyin_focus_in (AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_ENGINE (engine));

  AimSunpinyin *pinyin = AIM_SUNPINYIN (engine);

  pinyin->view->updateWindows(CIMIView::PREEDIT_MASK |
                              CIMIView::CANDIDATE_MASK);
}

void
aim_sunpinyin_focus_out (AimEngine *engine, AimConnection  *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_ENGINE (engine));

  aim_sunpinyin_reset (engine, target);
}

gboolean
aim_sunpinyin_filter_event (AimEngine     *engine,
                            AimConnection *target,
                            AimEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimSunpinyin *pinyin = AIM_SUNPINYIN (engine);

  pinyin->win_handler->set_target(target);

  if (event->key.type == AIM_EVENT_KEY_RELEASE)
    return FALSE;

  if (aim_event_matches (event, (const AimKey **) pinyin->zh_en_keys))
  {
    aim_sunpinyin_reset (engine, target);
    pinyin->is_english_mode = !pinyin->is_english_mode;
    aim_engine_emit_engine_changed (engine, target);
    return TRUE;
  }

  if (pinyin->is_english_mode)
  {
    return aim_english_filter_event (engine, target, event);
  }

  switch (event->key.keyval)
  {
    case AIM_KEY_Return:
    case AIM_KEY_KP_Enter:
    case AIM_KEY_space:
      {
        gint index = aim_engine_get_selected_candidate_index (engine);

        if (G_LIKELY (index >= 0))
        {
          pinyin->view->onCandidateSelectRequest(index);
          return TRUE;
        }
      }
      break;
    case AIM_KEY_Up:
    case AIM_KEY_KP_Up:
      if (!aim_engine_is_candidate_window_visible (engine))
        return FALSE;
      aim_engine_select_previous_candidate_item (engine);
      return TRUE;
    case AIM_KEY_Down:
    case AIM_KEY_KP_Down:
      if (!aim_engine_is_candidate_window_visible (engine))
        return FALSE;
      aim_engine_select_next_candidate_item (engine);
      return TRUE;
    default:
      break;
  }

  return pinyin->view->onKeyEvent(CKeyEvent(event->key.keyval,
                                            event->key.keyval,
                                            event->key.state));
}

static void
on_candidate_clicked (AimEngine     *engine,
                      AimConnection *target,
                      gchar         *text,
                      gint           index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimSunpinyin *pinyin = AIM_SUNPINYIN (engine);

  pinyin->view->onCandidateSelectRequest(index);
}

void
aim_sunpinyin_set_english_mode (AimEngine *engine,
                                gboolean   is_english_mode)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AIM_SUNPINYIN (engine)->is_english_mode = is_english_mode;
}

gboolean
aim_sunpinyin_get_english_mode (AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return AIM_SUNPINYIN (engine)->is_english_mode;
}

static void
aim_sunpinyin_class_init (AimSunpinyinClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  AimEngineClass *engine_class = AIM_ENGINE_CLASS (klass);

  engine_class->get_id             = aim_sunpinyin_get_id;
  engine_class->get_name           = aim_sunpinyin_get_name;
  engine_class->set_english_mode   = aim_sunpinyin_set_english_mode;
  engine_class->get_english_mode   = aim_sunpinyin_get_english_mode;
  engine_class->candidate_clicked  = on_candidate_clicked;

  engine_class->focus_in           = aim_sunpinyin_focus_in;
  engine_class->focus_out          = aim_sunpinyin_focus_out;
  engine_class->reset              = aim_sunpinyin_reset;
  engine_class->filter_event       = aim_sunpinyin_filter_event;

  object_class->finalize           = aim_sunpinyin_finalize;
}

static void
aim_sunpinyin_class_finalize (AimSunpinyinClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void module_load (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_sunpinyin_register_type (type_module);
}

GType module_get_type ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return aim_sunpinyin_get_type ();
}

void module_unload ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

G_END_DECLS
