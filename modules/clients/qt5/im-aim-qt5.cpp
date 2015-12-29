/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * im-aim-qt5.cpp
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

#include <QTextFormat>
#include <QInputMethodEvent>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatforminputcontextplugin_p.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <aim.h>

class AimInputContext : public QPlatformInputContext
{
  Q_OBJECT
public:
   AimInputContext ();
  ~AimInputContext ();

  virtual bool isValid () const;
  virtual void reset ();
  virtual void commit ();
  virtual void update (Qt::InputMethodQueries);
  virtual void invokeAction (QInputMethod::Action, int cursorPosition);
  virtual bool filterEvent (const QEvent *event);
  virtual QRectF keyboardRect () const;
  virtual bool isAnimating () const;
  virtual void showInputPanel ();
  virtual void hideInputPanel ();
  virtual bool isInputPanelVisible () const;
  virtual QLocale locale () const;
  virtual Qt::LayoutDirection inputDirection() const;
  virtual void setFocusObject (QObject *object);

  // aim signal callbacks
  static void     on_preedit_start        (AimIM       *im,
                                           gpointer     user_data);
  static void     on_preedit_end          (AimIM       *im,
                                           gpointer     user_data);
  static void     on_preedit_changed      (AimIM       *im,
                                           gpointer     user_data);
  static void     on_commit               (AimIM       *im,
                                           const gchar *text,
                                           gpointer     user_data);
  static gboolean on_retrieve_surrounding (AimIM       *im,
                                           gpointer     user_data);
  static gboolean on_delete_surrounding   (AimIM       *im,
                                           gint         offset,
                                           gint         n_chars,
                                           gpointer     user_data);
private:
  AimIM        *m_im;
  AimRectangle  m_cursor_area;
};

/* aim signal callbacks */
void
AimInputContext::on_preedit_start (AimIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
AimInputContext::on_preedit_end (AimIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
AimInputContext::on_preedit_changed (AimIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gchar *str;
  gint   cursor_pos;
  aim_im_get_preedit_string (im, &str, &cursor_pos);

  QString preeditText = QString::fromUtf8 (str);
  g_free (str);

  QList <QInputMethodEvent::Attribute> attrs;

  QTextCharFormat format = QTextCharFormat();
  format.setUnderlineStyle (QTextCharFormat::DashUnderline);
  // preedit text attribute
  attrs << QInputMethodEvent::Attribute (QInputMethodEvent::TextFormat,
                                         0, preeditText.length (),
                                         format);
  // cursor attribute
  attrs << QInputMethodEvent::Attribute (QInputMethodEvent::Cursor,
                                         cursor_pos, true, 0);

  QInputMethodEvent event (preeditText, attrs);
  QObject *object = qApp->focusObject ();

  if (!object)
    return;

  QCoreApplication::sendEvent (object, &event);
}

void
AimInputContext::on_commit (AimIM       *im,
                            const gchar *text,
                            gpointer     user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  QString str = QString::fromUtf8 (text);
  QInputMethodEvent event;
  event.setCommitString (str);

  QObject *obj = qApp->focusObject();

  if (!obj)
    return;

  QCoreApplication::sendEvent (obj, &event);
}

gboolean
AimInputContext::on_retrieve_surrounding (AimIM *im, gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return FALSE;
}

gboolean
AimInputContext::on_delete_surrounding (AimIM   *im,
                                        gint     offset,
                                        gint     n_chars,
                                        gpointer user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return FALSE;
}

AimInputContext::AimInputContext ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  m_im = aim_im_new ();
  g_signal_connect (m_im, "preedit-start",
                    G_CALLBACK (AimInputContext::on_preedit_start), this);
  g_signal_connect (m_im, "preedit-end",
                    G_CALLBACK (AimInputContext::on_preedit_end), this);
  g_signal_connect (m_im, "preedit-changed",
                    G_CALLBACK (AimInputContext::on_preedit_changed), this);
  g_signal_connect (m_im, "commit",
                    G_CALLBACK (AimInputContext::on_commit), this);
  g_signal_connect (m_im, "retrieve-surrounding",
                    G_CALLBACK (AimInputContext::on_retrieve_surrounding),
                    this);
  g_signal_connect (m_im, "delete-surrounding",
                    G_CALLBACK (AimInputContext::on_delete_surrounding),
                    this);
}

AimInputContext::~AimInputContext ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_object_unref (m_im);
}

bool
AimInputContext::isValid () const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return true;
}

void
AimInputContext::reset ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  aim_im_reset (m_im);
}

void
AimInputContext::commit ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  aim_im_reset (m_im);
}

void
AimInputContext::update (Qt::InputMethodQueries queries) /* FIXME */
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (queries & Qt::ImCursorRectangle)
  {
    QWidget *widget = qApp->focusWidget ();

    if (widget == NULL)
      return;

    QRect  rect  = widget->inputMethodQuery(Qt::ImCursorRectangle).toRect();
    QPoint point = widget->mapToGlobal (QPoint (0, 0));
    rect.translate (point);

    if (m_cursor_area.x      != rect.x ()     ||
        m_cursor_area.y      != rect.y ()     ||
        m_cursor_area.width  != rect.width () ||
        m_cursor_area.height != rect.height ())
    {
      m_cursor_area.x      = rect.x ();
      m_cursor_area.y      = rect.y ();
      m_cursor_area.width  = rect.width ();
      m_cursor_area.height = rect.height ();

      aim_im_set_cursor_location (m_im, &m_cursor_area);
    }
  }
}

void
AimInputContext::invokeAction(QInputMethod::Action, int cursorPosition)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

bool
AimInputContext::filterEvent (const QEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (!qApp->focusObject() || !inputMethodAccepted()))
    return false;

  gboolean         retval;
  const QKeyEvent *key_event = static_cast<const QKeyEvent *>( event );
  AimEvent        *aim_event;
  AimEventType     type = AIM_EVENT_NOTHING;

  switch (event->type ())
  {
#undef KeyPress
    case QEvent::KeyPress:
      type = AIM_EVENT_KEY_PRESS;
      break;
#undef KeyRelease
    case QEvent::KeyRelease:
      type = AIM_EVENT_KEY_RELEASE;
      break;
    case QEvent::MouseButtonPress:
      /* TODO: Provide as a option */
      aim_im_reset (m_im);
    default:
      return false;
  }

  aim_event = aim_event_new (type);
  aim_event->key.state            = key_event->nativeModifiers  ();
  aim_event->key.keyval           = key_event->nativeVirtualKey ();
  aim_event->key.hardware_keycode = key_event->nativeScanCode   (); /* FIXME: guint16 quint32 */

  retval = aim_im_filter_event (m_im, aim_event);
  aim_event_free (aim_event);

  return retval;
}

QRectF
AimInputContext::keyboardRect() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return QRectF ();
}

bool
AimInputContext::isAnimating() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return false;
}

void
AimInputContext::showInputPanel()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

void
AimInputContext::hideInputPanel()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

bool
AimInputContext::isInputPanelVisible() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return false;
}

QLocale
AimInputContext::locale() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return QLocale ();
}

Qt::LayoutDirection
AimInputContext::inputDirection() const
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  return Qt::LayoutDirection ();
}

void
AimInputContext::setFocusObject (QObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (!object || !inputMethodAccepted())
    aim_im_focus_out (m_im);

  QPlatformInputContext::setFocusObject (object);

  if (object && inputMethodAccepted())
    aim_im_focus_in (m_im);

  update (Qt::ImCursorRectangle);
}

/*
 * class AimInputContextPlugin
 */
class AimInputContextPlugin : public QPlatformInputContextPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID
    QPlatformInputContextFactoryInterface_iid
    FILE "./aim.json")

public:
  AimInputContextPlugin ()
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);
  }

  ~AimInputContextPlugin ()
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);
  }

  virtual QStringList keys () const
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);

    return QStringList () <<  "aim";
  }

  virtual QPlatformInputContext *create (const QString     &key,
                                         const QStringList &paramList)
  {
    g_debug (G_STRLOC ": %s", G_STRFUNC);

    return new AimInputContext ();
  }
};

#include "im-aim-qt5.moc"
