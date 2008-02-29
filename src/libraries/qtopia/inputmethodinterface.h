/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef INPUTMETHODINTERFACE_H
#define INPUTMETHODINTERFACE_H

#include <qplugin.h>
#include <qfactoryinterface.h>
#include <qicon.h>

#include <QStringList>
#include <QObject>

#include <qtopiaglobal.h>
#include <qtopiaipcmarshal.h>
#include <QSharedDataPointer>

class QWidget;
class QWSInputMethod;
class QWSGestureMethod;
class QMenu;
class QAction;

class QIMActionDescriptionPrivate;

class QTOPIA_EXPORT QIMActionDescription
{
public:
    QIMActionDescription(const QIMActionDescription& original) ;
    explicit QIMActionDescription(int id=0, QString label = QString(), 
        QString iconFileName=QString());
    ~QIMActionDescription();
    int id() const;
    void setId(const int);
    QString label() const;
    void setLabel(const QString&);
    QString iconFileName() const;
    void setIconFileName(const QString&);

template <typename T> 
    void serialize(T &stream) const;

template <typename T>
    void deserialize(T &stream);

protected:
    QIMActionDescription(QIMActionDescriptionPrivate &dd);
private:
    QSharedDataPointer<QIMActionDescriptionPrivate> d;
};

Q_DECLARE_USER_METATYPE ( QIMActionDescription )
Q_DECLARE_USER_METATYPE (QList<QIMActionDescription>)

class QTOPIA_EXPORT QtopiaInputMethod : public QObject
{
    Q_OBJECT
public:
    explicit QtopiaInputMethod(QObject *parent = 0) : QObject(parent) {}
    virtual ~QtopiaInputMethod() {}

    enum Properties {
        RequireMouse = 0x0001,
        RequireKeypad = 0x0002,
        InputModifier = 0x0004,
        InteractiveIcon = 0x0008,
        InputWidget = 0x0010,
        DockedInputWidget = 0x0030,  // 0x30 = 0x20|0x10, as docking makes little sense without an inputwidget.
        MenuItem = 0x0040
    };

    // changes based of the hint.
    enum State {
        Sleeping,
        Ready
    };

    //identifying functions.
    virtual QString name() const = 0;

    virtual QString identifier() const = 0;
    virtual QString version() const = 0;

    virtual State state() const = 0;
    // should return flag object, not int.
    virtual int properties() const = 0;

    bool testProperty(int) const;

    virtual QIcon icon() const = 0;

    // state managment.
    virtual void reset() = 0;

    virtual QWidget *statusWidget( QWidget *parent = 0);
    virtual QWidget *inputWidget( QWidget *parent = 0);
    virtual QWSInputMethod *inputModifier();

    virtual void setHint(const QString &, bool restrictToHint) = 0;
    virtual bool restrictedToHint() const;
    virtual bool passwordHint() const;

    virtual QList<QIMActionDescription*> menuDescription();
signals:
    void stateChanged();
    void updateMenuAction(bool showMenuAction); // Not currently used by the system

public slots:
    // only called on widgets that do not have an inputWidget.
    virtual void clicked();
    virtual void focusChanged();
    virtual void menuActionActivated(int data);

private:
    // internal helper functions
    };

#endif
