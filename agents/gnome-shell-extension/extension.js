/* -*- mode: js2; js2-basic-offset: 2; indent-tabs-mode: nil -*- */

const Lang           = imports.lang;
const St             = imports.gi.St;
const Clutter        = imports.gi.Clutter;
const Aim            = imports.gi.Aim;
const Main           = imports.ui.main;
const PanelMenu      = imports.ui.panelMenu;
const PopupMenu      = imports.ui.popupMenu;
const ModalDialog    = imports.ui.modalDialog;
const ExtensionUtils = imports.misc.extensionUtils;
const Gettext        = imports.gettext;
const _ = Gettext.gettext;

let aim_agent, aim_menu, extension;

const AboutDialog = new Lang.Class({
  Name: "AboutDialog",

  _init: function() {
    this._modal_dialog = new ModalDialog.ModalDialog({
      styleClass: 'modal-dialog'});

    this._modal_dialog.setButtons([{
      label:   _("Close"),
      action:  Lang.bind(this._modal_dialog, this._modal_dialog.close),
      key:     Clutter.Escape,
      default: true }]);
      let version_text  = extension.metadata['version'];
      let title_label   = new St.Label({ style_class: 'title-label',
                                         text: _("AIM Indicator"),
                                         x_expand: true,
                                         x_align: Clutter.ActorAlign.CENTER});
      let spacing       = new St.Label({ text: "\n" });
      let message_label = new St.Label({
        text: _("aim" + " " + version_text) });
      let content_layout = new St.BoxLayout({ vertical: true });

      content_layout.add(title_label);
      content_layout.add(spacing);
      content_layout.add(message_label, { y_fill:  true,
                                          y_align: St.Align.START });

      this._modal_dialog.contentLayout.add(content_layout, {x_fill: true,
                                                            y_fill: false});
  },

  open: function()
  {
    this._modal_dialog.open();
  }
});

function on_about()
{
  var dialog = new AboutDialog();
  dialog.open()
}

const AimMenu = new Lang.Class({
  Name: 'AimMenu',
  Extends: PanelMenu.Button,

  _init: function() {
    this.parent(0.0, _("AIM"));

    this._hbox = new St.BoxLayout({ style_class: 'panel-status-menu-box' });
    this._label = new St.Label({ text: _("AIM"),
                                 y_expand: true,
                                 y_align: Clutter.ActorAlign.CENTER });

    this._icon = new St.Icon({ icon_name: 'input-keyboard-symbolic',
                               style_class: 'system-status-icon' });
    this._warning_icon = new St.Icon({ icon_name: 'dialog-warning-symbolic',
                               style_class: 'system-status-icon' });
    this._hbox.add_child(this._icon);
    this.actor.add_child(this._hbox);

    this.menu.addMenuItem(new PopupMenu.PopupSeparatorMenuItem());
    this._about_menu = new PopupMenu.PopupMenuItem(_("About"));
    this._about_menu.connect ('activate',
                              Lang.bind(this._about_menu, on_about));
    this.menu.addMenuItem(this._about_menu);
  },
});

function init()
{
  extension = ExtensionUtils.getCurrentExtension();
  let domain    = extension.metadata['gettext-domain'];
  let localedir = extension.metadata['localedir']
  Gettext.bindtextdomain(domain, localedir);
  Gettext.textdomain(domain);
}

function enable()
{
  let child;

  aim_menu = new AimMenu;

  Main.panel.addToStatusArea('aim-agent-extension', aim_menu, 0, 'right');

  aim_agent = new Aim.Agent;

  aim_agent.connect('engine-changed', function(agent, text) {
    child = aim_menu._hbox.get_child_at_index (0);

    if (text == 'focus-out')
    {
      if (child != aim_menu._icon)
        aim_menu._hbox.replace_child (child, aim_menu._icon);
    }
    else
    {
      if (aim_menu._label.text != text)
        aim_menu._label.text = text;

      if (child != aim_menu._label)
        aim_menu._hbox.replace_child (child, aim_menu._label);
    }
  });

  aim_agent.connect('disconnected', function(agent) {
    child = aim_menu._hbox.get_child_at_index (0);
    aim_menu._hbox.replace_child (child, aim_menu._warning_icon);
  });

  aim_agent.connect_to_server();
}

function disable()
{
  aim_menu.destroy();
}
