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

#include "wheelbrowser.h"
#include "qtopiaserverapplication.h"
#include <QPalette>
#include <QVBoxLayout>
#include <QIcon>
#include <QPainter>
#include <QCategoryManager>
#include <QPalette>
#include <QContent>
#include <QKeyEvent>
#include <QSettings>
#include "qpixmapwheel.h"
#include <QTimeLine>
#include <Qtopia>
#include <QContentFilter>
#include <QContentSet>

WheelBrowserScreen::WheelBrowserScreen(QWidget *parent, Qt::WFlags flags)
: QAbstractBrowserScreen(parent, flags),
  m_fillAlpha(0), m_hiding(false), m_wheel(0), m_fillTimeline(0)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(100, 100, 100, 0));
    setPalette(pal);

    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);

    m_wheel = new QPixmapWheel(this);
    QFont f = font();
    f.setBold(true);
    m_wheel->setFont(f);
    m_wheel->setMaximumVisibleIcons(7);

    QObject::connect(m_wheel, SIGNAL(moveToCompleted()),
                     this, SLOT(moveToCompleted()));
    QObject::connect(m_wheel, SIGNAL(itemSelected(QString)),
                     this, SLOT(clicked(QString)));
    setFocusProxy(m_wheel);

    layout->addStretch(1);
    layout->addWidget(m_wheel, 5);
    layout->addStretch(1);

    m_data = new QPixmapWheelData;

    // Load apps
    QSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat);
    cfg.beginGroup("Menu"); // No tr
    const int menur = cfg.value("Rows",3).toInt();
    const int menuc = cfg.value("Columns",3).toInt();
    QString menuKeyMap = cfg.value("Map","123456789").toString();

    for (int i = 0; i < menur*menuc; i++) {
        QChar key = menuKeyMap[i];
        QStringList entries = cfg.value(QString(key)).toStringList();

        if(!entries.isEmpty()) {
            QContent *app = readLauncherMenuItem(entries.first());

            if(app) {
                QString file = app->fileName();
                QString name = app->name();
                if(app->type().startsWith("Folder/") )
                    file = app->type();
                QPixmap pix = app->icon().pixmap(QSize(48, 48));
                if(file.isEmpty())
                    file = app->type();

                if(!file.isEmpty() && !name.isEmpty() && !pix.isNull()) {
                    m_data->appendItem(file, pix, name);
                }
                delete app;
            }
        }
    }

    m_fillTimeline = new QTimeLine(2000, this);
    QObject::connect(m_fillTimeline, SIGNAL(valueChanged(qreal)),
                     this, SLOT(timelineStep(qreal)));
}

void WheelBrowserScreen::showEvent(QShowEvent *e)
{
    setEditFocus(true);
    m_views.clear();
    m_views.append("Main");
    m_wheel->moveToWheel(*m_data);
    m_fillTimeline->setDirection(QTimeLine::Forward);
    m_fillTimeline->start();
    QAbstractBrowserScreen::showEvent(e);
}

void WheelBrowserScreen::hideEvent(QHideEvent *e)
{
    m_wheel->setWheel(QPixmapWheelData());
    m_fillTimeline->stop();
    QAbstractBrowserScreen::hideEvent(e);
}

void WheelBrowserScreen::clicked(const QString &name)
{
    if(name.startsWith("Folder/")) {
        moveToView(name);

    } else {
        QContent app(name,false);
        app.execute();
        doHide();
    }
}

void WheelBrowserScreen::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QColor fill = palette().window().color();
    fill.setAlpha(m_fillAlpha);
    p.fillRect(0, 0, width(), height(), fill);
}

void WheelBrowserScreen::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Back ||
       e->key() == Qt::Key_Down) {

        m_views.removeLast();
        if(m_views.count()) {
            QString toview = m_views.last();
            QPixmapWheelData data = getData(toview);
            if(data.count()) {
                m_wheel->moveToWheel(data);
            }
        } else {
            doHide();
        }
        e->accept();
    } else {
        QAbstractBrowserScreen::keyPressEvent(e);
    }
}

void WheelBrowserScreen::doHide()
{
    if(m_hiding) {
        m_hiding = false;
        m_wheel->setWheel(QPixmapWheelData());
        hide();
    } else {
        m_hiding = true;
        m_fillTimeline->stop();
        m_fillTimeline->setDirection(QTimeLine::Backward);
        m_fillTimeline->start();
        m_wheel->moveToWheel(QPixmapWheelData());
    }
}

QString WheelBrowserScreen::currentView() const
{
    return "Main";
}

bool WheelBrowserScreen::viewAvailable(const QString &view) const
{
    return "Main" == view;
}

void WheelBrowserScreen::moveToCompleted()
{
    if(m_hiding) {
        m_hiding = false;
        hide();
    }
}

void WheelBrowserScreen::timelineStep(qreal r)
{
    m_fillAlpha = (int)(100.0 * r);
    update();
}

void WheelBrowserScreen::resetToView(const QString &name)
{
    if(name.isEmpty()) {
        m_wheel->setWheel(QPixmapWheelData());
        m_views.clear();
    } else {
        QPixmapWheelData data = getData(name);
        if(data.count()) {
            m_views.clear();
            m_views.append(name);
            if("Main" == name)
                m_wheel->moveToWheel(data);
            else
                m_wheel->setWheel(data);
        }
    }
}

QPixmapWheelData WheelBrowserScreen::getData(const QString &name)
{
    if("Main" == name) {

        return *m_data;

    } else if(name.startsWith("Folder/")) {
        QString category = name.mid(7 /* ::strlen("Folder/") */);

        QContentFilter filters
                = QContentFilter(QContent::Application)
              & QContentFilter(QContentFilter::Category, category);

        QPixmapWheelData data;
        QContentSet set(filters, QStringList() << QLatin1String("name"));
        QContentList items = set.items();
        for(int ii = 0; ii < items.count(); ++ii) {
            const QContent & app = items.at(ii);

            QString file = app.fileName();
            QString name = app.name();
            QPixmap pix = app.icon().pixmap(QSize(48, 48));
            if(file.isEmpty())
                file = app.type();

            if(!file.isEmpty() && !name.isEmpty() && !pix.isNull()) {
                data.appendItem(file, pix, name);
            }
        }

        return data;
    }

    return QPixmapWheelData();
}

void WheelBrowserScreen::moveToView(const QString &name)
{
    QPixmapWheelData data = getData(name);
    if(data.count()) {
        m_views.append(name);
        m_wheel->moveToWheel(data);
    }
}

QContent *WheelBrowserScreen::readLauncherMenuItem(const QString &entry)
{
    QContent *applnk = 0;

    if (entry.right(8)==".desktop") {
        // There used to be a quick way to locate a .desktop file
        // Now we have to create a QContentSet and iterate over the items

        // The path to the apps folder (which only exists in the database)
        QString apps = Qtopia::qtopiaDir()+"apps/";
        // We need the full path to the entry to compare against the items we get from QContentSet
        QString entryPath = apps+entry;
        applnk = new QContent( entryPath, false );
        if ( applnk->id() == QContent::InvalidId ) {
            delete applnk;
            applnk = 0;
        }
    } else if (entry == "Running") { // No tr
        applnk = new QContent();
        applnk->setName(tr("Running"));
        applnk->setType("Folder/Running");
        applnk->setIcon("Generic");
    } else {
        QCategoryManager catman("Applications");
        if(catman.contains(entry))
        {
            applnk = new QContent();
            applnk->setName(catman.label(entry));
            applnk->setIcon(catman.iconFile(entry));
            applnk->setType("Folder/"+entry);
        }
        else
            applnk = NULL;
    }

    return applnk;
}

QTOPIA_REPLACE_WIDGET(QAbstractBrowserScreen, WheelBrowserScreen);
