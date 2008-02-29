/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "dialing.h"

#include <qtopiaapplication.h>

#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>

DialingPage::DialingPage( const QtopiaNetworkProperties prop, QWidget* parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
    init();
    readConfig( prop );
}

DialingPage::~DialingPage()
{
}

void DialingPage::init()
{
}

void DialingPage::readConfig( const QtopiaNetworkProperties& prop )
{
}

QtopiaNetworkProperties DialingPage::properties()
{
    QtopiaNetworkProperties props;
    return props;
}
