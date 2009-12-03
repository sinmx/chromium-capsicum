// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/gtk/html_dialog_gtk.h"

#include <gtk/gtk.h>

#include "chrome/browser/browser.h"
#include "chrome/browser/browser_window.h"
#include "chrome/browser/dom_ui/html_dialog_ui.h"
#include "chrome/browser/gtk/tab_contents_container_gtk.h"
#include "chrome/browser/tab_contents/tab_contents.h"
#include "chrome/common/gtk_util.h"

// static
void HtmlDialogGtk::ShowHtmlDialogGtk(Browser* browser,
                                      HtmlDialogUIDelegate* delegate,
                                      gfx::NativeWindow parent_window) {
  HtmlDialogGtk* html_dialog =
      new HtmlDialogGtk(browser->profile(), delegate, parent_window);
  html_dialog->InitDialog();
}

////////////////////////////////////////////////////////////////////////////////
// HtmlDialogGtk, public:

HtmlDialogGtk::HtmlDialogGtk(Profile* profile,
                             HtmlDialogUIDelegate* delegate,
                             gfx::NativeWindow parent_window)
    : HtmlDialogTabContentsDelegate(profile),
      delegate_(delegate),
      parent_window_(parent_window),
      dialog_(NULL) {
}

HtmlDialogGtk::~HtmlDialogGtk() {
}

////////////////////////////////////////////////////////////////////////////////
// HtmlDialogUIDelegate implementation:

bool HtmlDialogGtk::IsDialogModal() const {
  return delegate_ ? delegate_->IsDialogModal() : false;
}

std::wstring HtmlDialogGtk::GetDialogTitle() const {
  return delegate_ ? delegate_->GetDialogTitle() : L"";
}

GURL HtmlDialogGtk::GetDialogContentURL() const {
  if (delegate_)
    return delegate_->GetDialogContentURL();
  else
    return GURL();
}

void HtmlDialogGtk::GetDOMMessageHandlers(
    std::vector<DOMMessageHandler*>* handlers) const {
  if (delegate_)
    delegate_->GetDOMMessageHandlers(handlers);
  else
    handlers->clear();
}

void HtmlDialogGtk::GetDialogSize(gfx::Size* size) const {
  if (delegate_)
    delegate_->GetDialogSize(size);
  else
    *size = gfx::Size();
}

std::string HtmlDialogGtk::GetDialogArgs() const {
  if (delegate_)
    return delegate_->GetDialogArgs();
  else
    return std::string();
}

void HtmlDialogGtk::OnDialogClosed(const std::string& json_retval) {
  DCHECK(delegate_);
  DCHECK(dialog_);

  HtmlDialogUIDelegate* dialog_delegate = delegate_;
  delegate_ = NULL;  // We will not communicate further with the delegate.
  Detach();
  dialog_delegate->OnDialogClosed(json_retval);
  gtk_widget_destroy(dialog_);
  delete this;
}

////////////////////////////////////////////////////////////////////////////////
// TabContentsDelegate implementation:

void HtmlDialogGtk::MoveContents(TabContents* source, const gfx::Rect& pos) {
  // The contained web page wishes to resize itself. We let it do this because
  // if it's a dialog we know about, we trust it not to be mean to the user.
}

void HtmlDialogGtk::ToolbarSizeChanged(TabContents* source,
                                       bool is_animating) {
  // Ignored.
}

////////////////////////////////////////////////////////////////////////////////
// HtmlDialogGtk:

void HtmlDialogGtk::InitDialog() {
  tab_contents_.reset(
      new TabContents(profile(), NULL, MSG_ROUTING_NONE, NULL));
  tab_contents_->set_delegate(this);

  // This must be done before loading the page; see the comments in
  // HtmlDialogUI.
  HtmlDialogUI::GetPropertyAccessor().SetProperty(tab_contents_->property_bag(),
                                                  this);

  tab_contents_->controller().LoadURL(GetDialogContentURL(),
                                      GURL(), PageTransition::START_PAGE);
  GtkDialogFlags flags = GTK_DIALOG_NO_SEPARATOR;
  if (delegate_->IsDialogModal())
    flags = static_cast<GtkDialogFlags>(flags | GTK_DIALOG_MODAL);

  dialog_ = gtk_dialog_new_with_buttons(
      WideToUTF8(delegate_->GetDialogTitle()).c_str(),
      parent_window_,
      flags,
      NULL);

  g_signal_connect(dialog_, "response", G_CALLBACK(OnResponse), this);

  tab_contents_container_.reset(new TabContentsContainerGtk(NULL));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_)->vbox),
                     tab_contents_container_->widget(), TRUE, TRUE, 0);

  tab_contents_container_->SetTabContents(tab_contents_.get());

  gfx::Size dialog_size;
  delegate_->GetDialogSize(&dialog_size);

  gtk_window_set_default_size(GTK_WINDOW(dialog_),
                              dialog_size.width(),
                              dialog_size.height());
  gtk_widget_show_all(dialog_);
}

// static
void HtmlDialogGtk::OnResponse(GtkWidget* widget, int response,
                               HtmlDialogGtk* dialog) {
  dialog->OnDialogClosed(std::string());
}
