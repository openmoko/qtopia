/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtbrushmanager.h"
#include "brushmanagerproxy.h"
#include "qsimpleresource_p.h"
#include "ui4_p.h"

#include <QDomDocument>

namespace qdesigner_internal {

class BrushManagerProxyPrivate
{
    BrushManagerProxy *q_ptr;
    Q_DECLARE_PUBLIC(BrushManagerProxy)
public:
    void brushAdded(const QString &name, const QBrush &brush);
    void brushRemoved(const QString &name);
    QString uniqueBrushFileName(const QString &brushName) const;

    QtBrushManager *theManager;
    QString theBrushFolder;
    QDesignerFormEditorInterface *theCore;
    QMap<QString, QString> theFileToBrush;
    QMap<QString, QString> theBrushToFile;
};

}  // namespace qdesigner_internal

using namespace qdesigner_internal;

void BrushManagerProxyPrivate::brushAdded(const QString &name, const QBrush &brush)
{
    QString filename = uniqueBrushFileName(name);

    QDir designerDir(QDir::homePath() + QDir::separator() + QLatin1String(".designer"));
    if (!designerDir.exists(QLatin1String("brushes")))
        designerDir.mkdir(QLatin1String("brushes"));

    QFile file(theBrushFolder + QDir::separator() +filename);
    if (!file.open(QIODevice::WriteOnly))
        return;

    QSimpleResource resource(theCore);

    DomBrush *dom = resource.saveBrush(brush);
    QDomDocument doc;
    QDomElement elem = doc.createElement(QLatin1String("description"));
    elem.setAttribute(QLatin1String("name"), name);
    elem.appendChild(dom->write(doc));
    doc.appendChild(elem);
    file.write(doc.toString().toUtf8());

    file.close();

    theFileToBrush[filename] = name;
    theBrushToFile[name] = filename;

    delete dom;
}

void BrushManagerProxyPrivate::brushRemoved(const QString &name)
{
    QDir brushDir(theBrushFolder);

    QString filename = theBrushToFile[name];
    brushDir.remove(filename);
    theBrushToFile.remove(name);
    theFileToBrush.remove(filename);
}

QString BrushManagerProxyPrivate::uniqueBrushFileName(const QString &brushName) const
{
    QString filename = brushName.toLower() + QLatin1String(".br");
    int i = 0;
    while (theFileToBrush.contains(filename))
        filename = brushName.toLower() + QString::number(++i) + QLatin1String(".br");
    return filename;
}


BrushManagerProxy::BrushManagerProxy(QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent)
{
    d_ptr = new BrushManagerProxyPrivate;
    d_ptr->q_ptr = this;

    d_ptr->theManager = 0;
    d_ptr->theBrushFolder = QDir::homePath()
                    + QDir::separator()
                    + QLatin1String(".designer")
                    + QDir::separator()
                    + QLatin1String("brushes");
    d_ptr->theCore = core;
}

BrushManagerProxy::~BrushManagerProxy()
{
    delete d_ptr;
}

void BrushManagerProxy::setBrushManager(QtBrushManager *manager)
{
    if (d_ptr->theManager == manager)
        return;

    if (d_ptr->theManager) {
        disconnect(d_ptr->theManager, SIGNAL(brushAdded(const QString &, const QBrush &)),
                    this, SLOT(brushAdded(const QString &, const QBrush &)));
        disconnect(d_ptr->theManager, SIGNAL(brushRemoved(const QString &)),
                    this, SLOT(brushRemoved(const QString &)));
    }

    d_ptr->theManager = manager;

    if (!d_ptr->theManager)
        return;

    // clear the manager
    QMap<QString, QBrush> brushes = d_ptr->theManager->brushes();
    QMap<QString, QBrush>::ConstIterator it = brushes.constBegin();
    while (it != brushes.constEnd()) {
        QString name = it.key();
        d_ptr->theManager->removeBrush(name);

        it++;
    }

    // fill up the manager from compiled resources or from brush folder here
    QDir brushDir(d_ptr->theBrushFolder);
    bool customBrushesExist = brushDir.exists();
    if (customBrushesExist) {
        // load brushes from brush folder
        QStringList nameFilters;
        nameFilters.append(QLatin1String("*.br"));

        QFileInfoList infos = brushDir.entryInfoList(nameFilters);
        QListIterator<QFileInfo> it(infos);
        while (it.hasNext()) {
            QFileInfo fi = it.next();

            QFile file(fi.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray contents = file.readAll();
                file.close();
                QDomDocument doc;
                if (doc.setContent(contents)) {
                    QDomElement domElement = doc.documentElement();

                    QString name = domElement.attribute(QLatin1String("name"));
                    QString filename = fi.fileName();

                    QSimpleResource resource(d_ptr->theCore);

                    QDomElement brushElement = domElement.firstChildElement(QLatin1String("brush"));
                    DomBrush dom;
                    dom.read(brushElement);
                    QBrush br = resource.setupBrush(&dom);

                    d_ptr->theManager->addBrush(name, br);
                    d_ptr->theFileToBrush[filename] = name;
                    d_ptr->theBrushToFile[name] = filename;
                }
            }
        }
    }

    connect(d_ptr->theManager, SIGNAL(brushAdded(const QString &, const QBrush &)),
            this, SLOT(brushAdded(const QString &, const QBrush &)));
    connect(d_ptr->theManager, SIGNAL(brushRemoved(const QString &)),
            this, SLOT(brushRemoved(const QString &)));

    if (!customBrushesExist) {
        // load brushes from resources
        QFile qrcFile(QLatin1String(":trolltech/brushes/defaultbrushes.xml"));
        if (qrcFile.open(QIODevice::ReadOnly)) {
            QByteArray contents = qrcFile.readAll();
            qrcFile.close();
            QDomDocument doc;
            if (doc.setContent(contents)) {
                QDomElement domElement = doc.documentElement();

                QDomElement descElement = domElement.firstChildElement(QLatin1String("description"));
                while (!descElement.isNull()) {
                    QString name = descElement.attribute(QLatin1String("name"));

                    QSimpleResource resource(d_ptr->theCore);

                    QDomElement brushElement = descElement.firstChildElement(QLatin1String("brush"));
                    DomBrush dom;
                    dom.read(brushElement);
                    QBrush br = resource.setupBrush(&dom);

                    d_ptr->theManager->addBrush(name, br);

                    descElement = descElement.nextSiblingElement(QLatin1String("description"));
                }
            }
        }
    }
}

#include "moc_brushmanagerproxy.cpp"
