/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __INPUT_METHODS_H__
#define __INPUT_METHODS_H__

#include <qwidget.h>
#include <qlist.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include <inputmethodinterface.h>
#include <qtopiaipcadaptor.h>
#include <qvaluespace.h>

#include <QPixmap>

class QToolButton;
class QPluginManager;

class InputMethodInterface;
class ExtInputMethodInterface;
class CoopInputMethodInterface;
class QUnknownInterface;
class IMToolButton;
class QMenu;
class QStackedWidget;
//class InputMethods;

class InputMethodSelector : public QWidget
{
    Q_OBJECT
public:
    InputMethodSelector(QWidget *parent = 0);
    ~InputMethodSelector();
    void add(QtopiaInputMethod *);
    void clear();

    //    void showInputMethod(bool);
    void setInputMethod(const QString &);
    void setInputMethod(QtopiaInputMethod *);
    void setNextInputMethod();
    void setHint(const QString &, bool);

    void sort();
    uint count() const { return list.count(); }

    // filters current list against libname l
    //void filterAgainst(const QString &l);
public slots:
    QtopiaInputMethod *current() const;
    // doesn't affect 'always on' IM's.
    void activateCurrent( bool );
    void updateIMMenuAction(bool addToCurrentMenu);
    void refreshIMMenuAction();
    void showChoice( bool ); //not the preferred way to activate the selector
    void showList();
signals:
    // some are auto activated, dep on type.
    void activated(QtopiaInputMethod *);

    void inputWidgetShown(bool shown);

private slots:
    void focusChanged(QWidget* old, QWidget* now);

private:
    void updateStatusIcon();
    QPixmap generatePixmap() const;

    QToolButton *mChoice;
    QStackedWidget *mButtonStack;
    IMToolButton *mButton;
    QWidget *mStatus;
    QMenu *pop;

    QtopiaInputMethod *mCurrent;
    QList<QtopiaInputMethod *> list;
    bool m_IMMenuActionAdded;
    QValueSpaceObject m_menuVS;
    QString defaultIM;
};

class InputMethods : public QWidget
{
    Q_OBJECT
public:
    enum IMType { Any=0, Mouse=1, Keypad=2 };
    enum SystemMenuItemId { NextInputMethod =-2, ChangeInputMethod =-3};
    
    InputMethods( QWidget *parent =0, IMType=Any );
    ~InputMethods();

    static InputMethods* instance();

    QRect inputRect() const;
    bool shown() const;
    bool selectorShown() const;
    void unloadInputMethods();
    void loadInputMethods();
    void activateMenuItem(int v);

public slots:
    QString currentShown() const; // name of interface
    void inputMethodHint( int hint, int windowId, bool passwordFlag=false );
    void inputMethodHint( const QString& hint, int windowId );
    void inputMethodPasswordHint(bool passwordFlag, int windowId);
    void showInputMethod(const QString& id);
    void showInputMethod();
    void hideInputMethod();
    void setNextInputMethod();
    void changeInputMethod();
    void setType(IMType t);

signals:
    void inputToggled( bool on );
    void visibilityChanged( bool visible);

private slots:
    //void chooseKbd();
    //void chooseIm();

    void resetStates();

    void choose(QtopiaInputMethod *);

    void updateIMVisibility();

#ifdef Q_WS_QWS
    void updateHintMap(QWSWindow *, QWSServer::WindowEvent);
#endif

private:
    void updateHint(int);

private:
    InputMethodSelector *selector;

    QPluginManager *loader;
    IMType type;

#ifdef Q_WS_QWS
    QWSInputMethod *currentIM;
#endif
    QMap<int, QString> hintMap;
    QMap<int, bool> restrictMap;
    int lastActiveWindow;
    QList<QObject *> ifaceList;
    QValueSpaceObject m_IMVisibleVS;
    bool m_IMVisible;
};

class InputMethodService : public QtopiaIpcAdaptor
{
    Q_OBJECT
public:
    explicit InputMethodService( InputMethods *parent );
    ~InputMethodService();

public slots:
    void inputMethodHint( int hint, int windowId );
    void inputMethodHint( const QString& hint, int windowId );
    void inputMethodPasswordHint(bool passwordFlag, int windowId);
    void hideInputMethod();
    void showInputMethod();
    void activateMenuItem(int v);
    void setInputMethod(const QString &inputMethodName);
    void setNextInputMethod();
    void changeInputMethod();
    void loadInputMethods();
    void unloadInputMethods();

private:
    InputMethods *parent;
};

#endif // __INPUT_METHODS_H__
