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
#include <qframe.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qapplication.h>
#include "krfboptions.h"

#include "kvncoptionsdlg.h"

KVNCOptionsDlg::KVNCOptionsDlg( KRFBOptions *options,
				QWidget *parent, char *name, bool modal )
  : VncOptionsBase( parent, name, modal )
{
  this->options = options;

  hex->setChecked( options->hexTile );
  corre->setChecked( options->corre );
  rre->setChecked( options->rre );
  copyRect->setChecked( options->copyrect );

  deIconify->setChecked( options->deIconify );
  bit->setChecked( options->colors256 );
  shared->setChecked( options->shared );
  timeBox->setValue( options->updateRate );
}

KVNCOptionsDlg::~KVNCOptionsDlg()
{
}

void KVNCOptionsDlg::accept()
{
  options->hexTile = hex->isChecked();
  options->corre = corre->isChecked();
  options->rre = rre->isChecked();
  options->copyrect = copyRect->isChecked();
  options->deIconify = deIconify->isChecked();
  options->colors256 = bit->isChecked();
  options->shared = shared->isChecked();
  options->updateRate = timeBox->value();

  QDialog::accept();
}

