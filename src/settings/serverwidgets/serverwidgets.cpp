/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include "serverwidgets.h"

#include <QtopiaIpcEnvelope>
#include <QtopiaApplication>

#include <QDebug>
#include <QSettings>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

struct {
    QString description;
    const char *defaultmapping;
    const char *serverinterface;
    const char *browserscreen;
} configurations[] =
    { 
      { QObject::tr("Default Qtopia"), 0, 0, 0 }, 
      { QObject::tr("Qtopia with wheel browser"), 0, 0, "Wheel" },
      { QObject::tr("E1 example"), "E1", 0, 0 },
      { QObject::tr("E2 example"), "E2", 0, 0 } 
    };

ServerWidgetSettings::ServerWidgetSettings( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setWindowTitle(tr("Server Widgets"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    QLabel *label = new QLabel(this);
    label->setWordWrap(true);
    layout->addWidget(label);

    label->setText(tr("<i>Select the Qtopia server configuration.</i>"));
    for(uint ii = 0; ii < sizeof(configurations) / sizeof(configurations[0]); ++ii) {
        QPushButton *pb = new QPushButton(configurations[ii].description, this);
        pb->setProperty("configReference", ii);
        layout->addWidget(pb);
        QObject::connect(pb, SIGNAL(clicked()), this, SLOT(clicked()));
    }
}

ServerWidgetSettings::~ServerWidgetSettings()
{
}

void ServerWidgetSettings::clicked()
{
    QObject *button = sender();
    Q_ASSERT(button);
    int config = button->property("configReference").toInt();

    QSettings cfg("Trolltech", "ServerWidgets");
    cfg.beginGroup("Mapping");
    cfg.remove("Default");
    cfg.remove("ServerInterface");
    cfg.remove("BrowserScreen");

    if(configurations[config].defaultmapping)
        cfg.setValue("Default", configurations[config].defaultmapping);
    if(configurations[config].serverinterface)
        cfg.setValue("ServerInterface", configurations[config].serverinterface);
    if(configurations[config].browserscreen)
        cfg.setValue("BrowserScreen", configurations[config].browserscreen);

    {
        QtopiaIpcEnvelope env( "QPE/System", "restart()" );
    }
    accept();
}

