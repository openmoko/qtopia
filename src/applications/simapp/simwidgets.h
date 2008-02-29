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

#ifndef SIMWIDGETS_H
#define SIMWIDGETS_H

#include <QFrame>
#include <QTextBrowser>
#include <QSimToolkit>
#include <QSimTerminalResponse>
#include <QSimEnvelope>
#include <QDialog>
#include <QPhoneCall>

class QLabel;
class QLineEdit;
class QTextEdit;
class QListWidget;
class QListWidgetItem;
class QSimIconReader;
class SimIcons;
#ifdef MEDIA_SERVER
class QSoundControl;
#else
class QSound;
#endif
class QPhoneCallManager;
class QMessageBox;


class SimCommandView : public QFrame
{
    Q_OBJECT
public:
    SimCommandView(const QSimCommand &cmd, QWidget *parent);
    SimCommandView(const QSimCommand &cmd, QSimIconReader *reader, QWidget *parent = 0);
    ~SimCommandView();

    QSimCommand command() const { return m_command; }

    void setNoResponseTimeout( int msecs );

    virtual uint helpIdentifier() const;

signals:
    void sendResponse(const QSimTerminalResponse &response);

protected slots:
    virtual void iconsReady() {}

protected:
    QSimCommand m_command;
    int tid;
    SimIcons *icons;
};

class SimMenu : public SimCommandView
{
    Q_OBJECT
public:
    SimMenu(const QSimCommand &simCmd, QWidget *parent);
    SimMenu(const QSimCommand &simCmd, QSimIconReader *reader, QWidget *parent);
    virtual ~SimMenu() {}

    bool isMainMenu() const { return m_command.type() == QSimCommand::SetupMenu; }

    uint helpIdentifier() const;

signals:
    void sendEnvelope(const QSimEnvelope &env);

protected:
    void timerEvent(QTimerEvent *);
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void itemSelected(QListWidgetItem*);
    void currentItemChanged(QListWidgetItem*, QListWidgetItem*);
    virtual void iconsReady();

private:
    void init(QSimIconReader *reader = 0);
    void populate(QSimIconReader *reader = 0);

private:
    QLabel *title;
    QListWidget *menu;
};

class SimText : public SimCommandView
{
    Q_OBJECT
public:
    SimText(const QSimCommand &simCmd, QSimIconReader *reader, QWidget *parent);
    virtual ~SimText() {}

    void setCommand(const QSimCommand &, QSimIconReader *reader = 0);
    bool hasHighPriority() const { return m_command.highPriority(); }

protected:
    void timerEvent(QTimerEvent *);
    void showEvent(QShowEvent *);
    bool eventFilter(QObject *o, QEvent *e);

private:
    void initTimer();
    void write();
    void response(const QSimTerminalResponse::Result &result);

private slots:
    virtual void iconsReady();

private:
    QTextBrowser *browser;
};

class SimInKey : public SimCommandView
{
    Q_OBJECT
public:
    SimInKey(const QSimCommand &simCmd, QSimIconReader *reader, QWidget *parent);
    virtual ~SimInKey() {}

protected:
    void keyPressEvent(QKeyEvent *);
    void timerEvent(QTimerEvent *);

private slots:
    void textChanged(const QString &);
    virtual void iconsReady();

private:
    void response(const QString &result);
    void response(const QSimTerminalResponse::Result &result);

private:
    QLabel *text, *iconLabel;
    QLineEdit *edit;
    bool digitsOnly;
    bool wantYesNo;
};

class SimInput : public SimCommandView
{
    Q_OBJECT
public:
    SimInput(const QSimCommand &simCmd, QWidget *parent);
    virtual ~SimInput() {}

    void setInput(const QString &input);

protected:
    void keyPressEvent(QKeyEvent *);
    void timerEvent(QTimerEvent *);
    bool eventFilter(QObject *o, QEvent *e);
    void done();

private:
    QLabel *text;
    QLineEdit *lineEdit;
    QTextEdit *multiEdit;
    bool digitsOnly;
    bool echo;
};

class SoftKeysMenu : public SimCommandView
{
    Q_OBJECT
public:
    SoftKeysMenu(const QSimCommand& cmd, QSimIconReader *reader, QWidget *parent = 0);
    ~SoftKeysMenu();

protected:
    void keyPressEvent(QKeyEvent *);

signals:
    void sendEnvelope(const QSimEnvelope &env);

private slots:
    virtual void iconsReady();

private:
    QLabel *title;
};

class SimTone : public SimCommandView
{
    Q_OBJECT
public:
    SimTone(const QSimCommand& cmd, QSimIconReader *reader, QWidget *parent = 0);
    ~SimTone();

private:
    QString findFile(const QSimCommand::Tone &tone);
    void playTone(const QString &file);

private slots:
    void stopSound();
    void success();
    virtual void iconsReady();

private:
    QLabel *title;
#ifdef MEDIA_SERVER
    QSoundControl *soundcontrol;
#else
    QSound *currentSound;
#endif
    QTimer *soundTimer;
};

class SimSetupCall : public SimCommandView
{
    Q_OBJECT
public:
    SimSetupCall(const QSimCommand &simCmd, QWidget *parent = 0);
    SimSetupCall(const QSimCommand& cmd, QSimIconReader *reader, QWidget *parent = 0);
    ~SimSetupCall() {}

protected:
    void keyPressEvent(QKeyEvent *);

private:
    void init();
    void disposeCalls();

private slots:
    void callStateChanged(const QPhoneCall& call);
    void requestFailed(const QPhoneCall& call, QPhoneCall::Request request);
    void dial();
    virtual void iconsReady();

private:
    QPhoneCallManager *callManager;
    QLabel *text;
    QLabel *iconLabel;
    uint redialCount;
    bool multiPartyCall;
    bool busyOnCall;
    bool callControlProblem;
    QMessageBox *otherTextBox;
};

class SimChannel : public SimCommandView
{
    Q_OBJECT
public:
    SimChannel(const QSimCommand &simCmd, QWidget *parent = 0);
    SimChannel(const QSimCommand &simCmd, QSimIconReader *reader, QWidget *parent = 0);
    ~SimChannel() {}

protected:
    void keyPressEvent(QKeyEvent *);

private:
    void init();

private slots:
    virtual void iconsReady();

private:
    QLabel *text;
    QLabel *iconLabel;
    bool busyOnCall;
};

class SimLaunchBrowser : public SimCommandView
{
    Q_OBJECT
public:
    SimLaunchBrowser(const QSimCommand &simCmd, QWidget *parent = 0);
    SimLaunchBrowser(const QSimCommand &simCmd, QSimIconReader *reader, QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *);

private:
    void init();

private slots:
    virtual void iconsReady();

private:
    QLabel *text;
    QLabel *iconLabel;
    bool isAvailable;
    bool isRunning;
};

#endif

