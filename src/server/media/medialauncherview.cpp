/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "medialauncherview.h"

#include <qtopianamespace.h>
#include <qcontent.h>
#include <qmimetype.h>
#include <qtopialog.h>

#include <QListWidget>
#include <QVBoxLayout>
#include <QSettings>
#include <QDebug>

class MediaLauncherViewPrivate
{
public:
    struct MediaLauncherItem {
        QContent content;
        bool enabled;
    };
    QMap<QChar,QList<MediaLauncherItem> > launcherEntries;
};


MediaLauncherView::MediaLauncherView(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    d = new MediaLauncherViewPrivate;

    QVBoxLayout *vb = new QVBoxLayout;
    vb->setMargin(20);
    setLayout(vb);

    view = new QListWidget;
    connect(view, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(activated(QListWidgetItem*)));
    vb->addWidget(view);

    load();
}

void MediaLauncherView::load()
{
    QSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat);

    cfg.beginGroup("Menu");

    QString keyMap = cfg.value("Map","123456789").toString();

    for (int i = 0; i < keyMap.length(); i++) {
        QChar key = keyMap[i];
        QStringList entries = cfg.value(QString(key)).toStringList();
        qLog(UI) << "MediaLauncherView::load" << key << "->" << entries;
        if (!entries.isEmpty()) {
            for (int li=0; li < (int)entries.count(); li++) {
                QContent content = readEntry(entries[li]);
                if (content.isValid()) {
                    qLog(UI) << "MediaLauncherView::load QContent: Name" << content.name() << "Exec" << content.executableName();
                    MediaLauncherViewPrivate::MediaLauncherItem item;
                    item.content = content;
                    item.enabled = li == (int)entries.count()-1;
                    d->launcherEntries[key] += item;
                }
                else
                    qLog(UI)  << "MediaLauncherView::load" << "No AppLnk for" << entries[li];
            }
            if (d->launcherEntries[key].count()) {
                QListWidgetItem *item = new QListWidgetItem(
                            d->launcherEntries[key].last().content.icon(),
                            d->launcherEntries[key].last().content.name());
                item->setData(Qt::UserRole, QVariant(key));
                view->addItem(item);
            }
        }
    }
    QString d = cfg.value("Default",keyMap.mid(keyMap.length()/2,1)).toString();
    int defaultSelection = keyMap.indexOf(d);
    view->setCurrentRow(defaultSelection);

    cfg.endGroup();
}

QContent MediaLauncherView::readEntry(const QString &entry)
{
    qLog(UI) << "MediaLauncherView::readLauncherMenuItem" << entry;
    QString dir = QMimeType::appsFolderName();
    QContent content;

    if (entry.right(8)==".desktop") {
        QContent desktop(dir+"/"+entry);
        if (desktop.isValid() && desktop.type() != "Separator" ) // No tr
            content = desktop;
    } else if (entry == "Documents") { // No tr
        content.setName(tr("Documents"));
        content.setType("Folder/Documents");
        content.setIcon("qpe/DocsIcon");
    } else if (entry == "Running") { // No tr
        content.setName(tr("Running"));
        content.setType("Folder/Running");
        content.setIcon("Generic");
    } else {
        QContent directory(dir+"/"+entry+"/.directory");
        if (!directory.name().isEmpty() && !directory.iconName().isEmpty()) {
            content = directory;
            content.setType("Folder/"+entry);
        }
    }

    return content;
}

void MediaLauncherView::activated(QListWidgetItem *item)
{
    if (!item)
        return;

    QChar key = item->data(Qt::UserRole).toChar();
    emit clicked(&d->launcherEntries[key].first().content);
}

