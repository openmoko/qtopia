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

#include "secondarythemeddisplay.h"
#include "phonethemeview.h"
#include "qtopiaserverapplication.h"
#include "themecontrol.h"
#include "windowmanagement.h"

#include <qexportedbackground.h>

#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QPainter>


//XXX Should probably get secondary screen number from config.
const int SecondaryScreen = 1;

class SecondaryHomeScreen : public PhoneThemedView
{
    Q_OBJECT
public:
    SecondaryHomeScreen(QWidget *parent=0, Qt::WFlags f=0);

private slots:
    void updateBackground();

private:
    QExportedBackground *exportedBg;
};

SecondaryHomeScreen::SecondaryHomeScreen(QWidget *parent, Qt::WFlags f)
    : PhoneThemedView(parent, f), exportedBg(0)
{
    ThemeControl::instance()->registerThemedView(this, "SecondaryHome");

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->screenGeometry(SecondaryScreen);
    QExportedBackground::initExportedBackground(desktopRect.width(),
                                                desktopRect.height(),
                                                SecondaryScreen);

    if (ThemeControl::instance()->exportBackground())
        exportedBg = new QExportedBackground(SecondaryScreen, this);
    connect(ThemeControl::instance(), SIGNAL(themeChanged()),
            this, SLOT(updateBackground()));
}

void SecondaryHomeScreen::updateBackground()
{
    if (ThemeControl::instance()->exportBackground()) {
        ThemeItem *item = findItem("background", ThemedView::Item);
        if (!exportedBg)
            exportedBg = new QExportedBackground(SecondaryScreen, this);
        bool wasExported = exportedBg->isAvailable();
        if (item) {
            QDesktopWidget *desktop = QApplication::desktop();
            QRect desktopRect = desktop->screenGeometry(SecondaryScreen);
            QPixmap pm(desktopRect.width(),
                       desktopRect.height());
            QPainter p(&pm);
            QRect rect(QPoint(0,0), desktopRect.size());
            paint(&p, rect, item);
            QExportedBackground::setExportedBackground(pm, SecondaryScreen);
        } else {
            QExportedBackground::clearExportedBackground(SecondaryScreen);
        }

        if (!wasExported && exportedBg->isAvailable()) {
            QApplication::setPalette(QApplication::palette());
            foreach (QWidget *w, QApplication::topLevelWidgets()) {
                if (QApplication::desktop()->screenNumber(w) == SecondaryScreen) {
                    QApplication::style()->polish(w);
                    foreach (QObject *o, w->children()) {
                        QWidget *sw = qobject_cast<QWidget*>(o);
                        if (sw) {
                            QApplication::style()->polish(sw);
                        }
                    }
                }
            }
        }
    } else {
        QExportedBackground::clearExportedBackground(SecondaryScreen);
    }
}

//===========================================================================

class SecondaryTitle : public PhoneThemedView
{
public:
    SecondaryTitle(QWidget *parent=0, Qt::WFlags f=0);
};

SecondaryTitle::SecondaryTitle(QWidget *parent, Qt::WFlags f)
    : PhoneThemedView(parent, f)
{
    ThemeControl::instance()->registerThemedView(this, "SecondaryTitle");
    WindowManagement::dockWindow(this, WindowManagement::Top, SecondaryScreen);
}

//===========================================================================

ThemedSecondaryDisplay::ThemedSecondaryDisplay(QWidget *parent, Qt::WFlags f)
    : QAbstractSecondaryDisplay(parent, f)
{
    title = new SecondaryTitle(0, Qt::FramelessWindowHint
                                    | Qt::Tool
                                    | Qt::WindowStaysOnTopHint);
    title->show();

    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    home = new SecondaryHomeScreen;
    vbox->addWidget(home);
}

QTOPIA_REPLACE_WIDGET(QAbstractSecondaryDisplay, ThemedSecondaryDisplay);

#include "secondarythemeddisplay.moc"
