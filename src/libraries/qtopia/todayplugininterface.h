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

#ifndef TODAY_PLUGIN_TASK_H
#define TODAY_PLUGIN_TASK_H

#include <qplugin.h>
#include <qfactoryinterface.h>
#include <qobject.h>
#include <qtopiaglobal.h>

class QString;
class QPixmap;
class QWidget;

class QTOPIA_EXPORT TodayItemDisplay : public QObject
{
public:
    explicit TodayItemDisplay(QObject *parent) : QObject(parent) {}
    virtual ~TodayItemDisplay() {}

    virtual QString name() const = 0;
    virtual QPixmap icon() const = 0;

    virtual QString html(uint charWidth, uint lineHeight) const = 0;

    // Reimplement these if your plugin is configurable
    virtual bool configurable() const = 0;
    virtual QWidget* widget(QWidget *)= 0;
    virtual void accepted(QWidget *) const = 0;

    // Reimplement this method if your plugin should do something when links are clicked
    virtual void itemSelected(const QString &) const = 0;

};

struct QTOPIA_EXPORT QTodayFactoryInterface : public QFactoryInterface
{
    virtual TodayItemDisplay *item(const QString &key) = 0;
};

#define QTodayFactoryInterface_iid "com.trolltech.Qtopia.QTodayFactoryInterface"
Q_DECLARE_INTERFACE(QTodayFactoryInterface, QTodayFactoryInterface_iid)

class QTOPIA_EXPORT QTodayPlugin : public QObject, public QTodayFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QTodayFactoryInterface:QFactoryInterface)
public:
    explicit QTodayPlugin(QObject *parent=0) : QObject(parent) {}
    virtual ~QTodayPlugin() {}

    virtual QStringList keys() const = 0;
    virtual TodayItemDisplay *item(const QString &key) = 0;
};
#endif
