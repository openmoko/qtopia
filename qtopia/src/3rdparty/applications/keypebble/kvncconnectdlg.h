/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
// -*- c++ -*-

#ifndef KVNCCONNECTDLG_H
#define KVNCCONNECTDLG_H

#include <qdialog.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qurl.h>

class KRFBConnection;

class KVNCConnectDlg : public QDialog
{
  Q_OBJECT

public: 
  KVNCConnectDlg( KRFBConnection *con,
		  QWidget *parent = 0, const char *name = 0 );

  QString hostname() { return hostNameCombo->currentText(); };
  int display() { return displayNumberEdit->value(); };
  QString password() const { return passwordEdit->text(); }
 
protected:
  void accept();

protected slots:
  void options();

private:
  QComboBox *hostNameCombo;
  QSpinBox *displayNumberEdit;
  QLineEdit *passwordEdit;
  KRFBConnection *con;
};

#endif // KVNCCONNECTDLG_H
