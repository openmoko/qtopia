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
#ifndef __QPE_APPLICATION_H__
#define __QPE_APPLICATION_H__

#include <qtopiaglobal.h>
#include <qtopianamespace.h>
#include <qapplication.h>
#include <qdialog.h>
#include <qtimestring.h>
#include <qcontent.h>
#include <QDebug>
#include <qtopialog.h>

#include "perftest.h"
#include "qperformancelog.h"
#include <qtopiasxe.h>

class QtopiaChannel;
class QtopiaApplicationData;
struct QWSEvent;
struct QWSKeyEvent;
class PluginLibraryManager;
class ScreenSaver;
class QtopiaStyle;

extern char _key[];

class QTOPIA_EXPORT QtopiaApplication : public QApplication
{
    Q_OBJECT
    friend class QtopiaApplicationData;
public:
    QtopiaApplication( int& argc, char **argv, Type=GuiClient );
    ~QtopiaApplication();

    static QtopiaApplication *instance();

    bool willKeepRunning() const;
    void registerRunningTask(const QString &, QObject * = 0);
    void unregisterRunningTask(const QString &);
    void unregisterRunningTask(QObject *);

    void applyStyle();
    static void grabKeyboard();
    static void ungrabKeyboard();

    enum StylusMode {
        LeftOnly,
        RightOnHold
        // RightOnHoldLeftDelayed, etc.
    };
    static void setStylusOperation( QWidget*, StylusMode );
    static StylusMode stylusOperation( QWidget* );

    enum InputMethodHint {
        Normal,
        AlwaysOff,
        AlwaysOn,
        Number,
        PhoneNumber,
        Words,
        Text,
        Named
    };

    enum PowerConstraint {
        Disable = 0,  //disable all timeout actions
        DisableLightOff = 50, //do not turn backlight off
        DisableReturnToHomeScreen = 100, //do not return to the homescreen (phone only)
        DisableSuspend = 900, //do not suspend the device (PDA only)
        Enable = 1000 //enable all timeout actions
    };

    static void setInputMethodHint( QWidget *, InputMethodHint, const QString& param=QString() );
    static void setInputMethodHint( QWidget *, const QString& named );
    static InputMethodHint inputMethodHint( QWidget * );
    static QString inputMethodHintParam( QWidget * );
    static void hideInputMethod();
    static void showInputMethod();

    QWidget* mainWidget() const;
    void setMainWidget(QWidget *, bool nomax=false);
    void showMainWidget();
    void showMainDocumentWidget();

    void showMainWidget( QWidget*, bool nomax=false );
    void showMainDocumentWidget( QWidget*, bool nomax=false );
    static void showDialog( QDialog*, bool nomax=false );
    static int execDialog( QDialog*, bool nomax=false );
#ifdef QTOPIA_KEYPAD_NAVIGATION
    static void setMenuLike( QDialog *, bool );
    static bool isMenuLike( const QDialog* );
#endif
    static void setPowerConstraint(PowerConstraint);    // libqtopia

    bool keyboardGrabbed() const;

    int exec();

    static void loadTranslations(const QString&);
    static void loadTranslations(const QStringList&);
    void initApp( int argc, char **argv );

signals:
    void clientMoused();
    void timeChanged();
    void categoriesChanged();
    void clockChanged( bool pm );
    void volumeChanged( bool muted );
    void appMessage( const QString& msg, const QByteArray& data);
    void weekChanged( bool startOnMonday );
    void dateFormatChanged();
    void flush();
    void reload();
    void contentChanged(const QContentIdList &,QContent::ChangeType);
    void resetContent();

private slots:
    void systemMessage( const QString &msg, const QByteArray &data );
    void pidMessage( const QString &msg, const QByteArray &data );
#ifndef QTOPIA_DBUS_IPC
    void dotpidMessage( const QString &msg, const QByteArray &data );
#endif
    void removeSenderFromStylusDict();
    void removeSenderFromIMDict();
    void hideOrQuit();
    void lineEditTextChange(const QString &);
    void multiLineEditTextChange();
    void buttonChange(bool);
    void textBrowserHighlightChange(const QString &);

    void removeFromWidgetFlags();
    void updateDialogGeometry();


protected:
    bool notify(QObject*,QEvent*);
    bool qwsEventFilter( QWSEvent * );
    QtopiaStyle *internalSetStyle( const QString &style );
    virtual void restart();
    virtual void shutdown();
    bool eventFilter( QObject *, QEvent * );
    void timerEvent( QTimerEvent * );
    bool raiseAppropriateWindow();
    virtual void tryQuit();

private:
    void mapToDefaultAction( QWSKeyEvent *ke, int defKey );
    void processQCopFile();

#ifdef QTOPIA_PHONE
    void hideMessageBoxButtons( QMessageBox * );
#endif

    static void sendInputHintFor(QWidget*,QEvent::Type);

    QtopiaApplicationData *d;
};

#ifdef Q_OS_WIN32
#include <stdlib.h>
QTOPIA_EXPORT int setenv(const char* name, const char* value, int overwrite);
QTOPIA_EXPORT void unsetenv(const char *name);
#endif

/*
 * Macros for simplifying the support of multiple launch modes in dynamic and singleexec builds.
 *
 * The options are:
 *   - singleexec/quicklaunch
 *   - singleexec/manual (user-supplied main function)
 *   - dynamic/quicklaunch
 *   - dynamic/normal
 *
 * There's no need to support dynamic/manual because any old main function will do. It's required
 * for singleexec because of the extra registration required to associate the main function with
 * the application's name. singleexec/normal is not supported because there's no point (size/load
 * penalties of quicklaunch don't exist).
 *
 * First up is the helpers. The primary macros (QTOPIA_ADD_APPLICATION, QTOPIA_MAIN) are below.
 */
#include <qmap.h>
#include <qapplicationplugin.h>
#include <qmetaobject.h>

// helper types/functions
typedef QWidget* (*qpeAppCreateFunc)(QWidget*,Qt::WFlags);
typedef QMap<QString,qpeAppCreateFunc> QPEAppMap;
extern void qtopia_registerApp(const char *name, qpeAppCreateFunc createFunc);

typedef int (*qpeMainFunc)(int,char**);
typedef QMap<QString,qpeMainFunc> QPEMainMap;
extern void qtopia_registerMain(const char *name, qpeMainFunc mainFunc);

// The SXE stuff (depends on quicklaunch or normal mode)
#ifdef QTOPIA_APP_INTERFACE
#define QTOPIA_APP_KEY QSXE_QL_APP_KEY
#else
#define QTOPIA_APP_KEY QSXE_APP_KEY
#endif

// Register a main function (singleexec/manual)
#define QTOPIA_REGISTER_SINGLE_EXEC_MAIN(NAME,IMPLEMENTATION) \
    int main_##IMPLEMENTATION( int argc, char **argv );\
    static qpeMainFunc append_##IMPLEMENTATION() \
        { qtopia_registerMain(NAME, main_##IMPLEMENTATION); \
            return main_##IMPLEMENTATION; } \
    static qpeMainFunc dummy_##IMPLEMENTATION = \
        append_##IMPLEMENTATION();

// Register a widget (*/quicklaunch, dynamic/normal)
#define QTOPIA_REGISTER_WIDGET(NAME,IMPLEMENTATION) \
    static QWidget *create_##IMPLEMENTATION( QWidget *p, Qt::WFlags f ) \
        { return new IMPLEMENTATION(p, f); } \
    static qpeAppCreateFunc append_##IMPLEMENTATION() \
        { qtopia_registerApp(NAME, create_##IMPLEMENTATION); \
            return create_##IMPLEMENTATION; } \
    static qpeAppCreateFunc dummy_##IMPLEMENTATION = \
        append_##IMPLEMENTATION();

// Setup the app map (dynamic/*)
#define QTOPIA_SETUP_APP_MAP \
    QTOPIA_APP_KEY \
    QPEAppMap *qpeAppMap() { \
        static QPEAppMap *am = 0; \
        if ( !am ) am = new QPEAppMap(); \
        return am; \
    } \
    void qtopia_registerApp(const char *name, qpeAppCreateFunc createFunc) { \
        QPEAppMap *am = qpeAppMap(); \
        am->insert(name, createFunc); \
    }

// Setup the quicklaunch plugin (dynamic/quicklaunch)
#define QTOPIA_QUICKLAUNCH_IMPL \
    struct ApplicationImpl : public QApplicationPlugin { \
        ApplicationImpl() {} \
        virtual void setProcessKey( const QString &appName ) { \
            Q_UNUSED(appName); \
            QSXE_SET_QL_KEY(qPrintable(appName)) \
        } \
        virtual QWidget *createMainWindow( const QString &appName, QWidget *parent, Qt::WFlags f ) { \
            qLog(Quicklauncher) << "created main window for quicklaunched" << appName.toLocal8Bit().constData(); \
            if ( qpeAppMap()->contains(appName) ) \
                return (*qpeAppMap())[appName](parent, f); \
            return 0; \
        } \
        virtual QStringList keys() const { \
            QStringList list; \
            for ( QPEAppMap::Iterator it=qpeAppMap()->begin(); it!=qpeAppMap()->end(); ++it ) \
                list += it.key(); \
            return list; \
        } \
    }; \
    QTOPIA_EXPORT_PLUGIN(ApplicationImpl)

// The main function (dynamic/normal)
#define QTOPIA_MAIN_IMPL \
    int main( int argc, char **argv ) { \
        QSXE_SET_APP_KEY(argv[0]) \
        QString executableName(argv[0]); \
        executableName = executableName.right(executableName.length() - executableName.lastIndexOf('/') - 1); \
        QPerformanceLog(executableName.toLatin1().constData())  << "Starting main()"; \
        QtopiaApplication a( argc, argv ); \
        QWidget *mw = 0; \
        if ( qpeAppMap()->contains(executableName) ) \
            mw = (*qpeAppMap())[executableName](0,0); \
        else if ( qpeAppMap()->count() ) \
            mw = qpeAppMap()->begin().value()(0,0); \
        if ( mw ) { \
            int rv = 0; \
            a.setMainWidget(mw); \
            if ( mw->metaObject()->indexOfSlot("setDocument(QString)") != -1 ) { \
                a.showMainDocumentWidget(); \
            } else { \
                a.showMainWidget(); \
            } \
            QPerformanceLog(executableName.toLatin1().constData()) << "Entering event loop"; \
            rv = a.exec(); \
            QPerformanceLog(executableName.toLatin1().constData()) << "Exited event loop"; \
            delete mw; \
            QPerformanceLog(executableName.toLatin1().constData()) << "Exiting main"; \
            return rv; \
        } \
        return -1; \
    }

/*
 * Definitions of the primary macros.
 */
#if defined(SINGLE_EXEC)

// The QTOPIA_MAIN macro is useless in single-exec mode
#define QTOPIA_MAIN

#if defined(SINGLE_EXEC_USE_MAIN)

// singleexec/manual
#define QTOPIA_ADD_APPLICATION(NAME,IMPLEMENTATION) \
    QTOPIA_REGISTER_SINGLE_EXEC_MAIN(NAME,IMPLEMENTATION)

#endif

#endif // SINGLE_EXEC

// */quicklaunch, dynamic/normal
#ifndef QTOPIA_ADD_APPLICATION
#define QTOPIA_ADD_APPLICATION(NAME,IMPLEMENTATION) \
    QTOPIA_REGISTER_WIDGET(NAME,IMPLEMENTATION)
#endif

#ifndef QTOPIA_MAIN

#ifdef QTOPIA_APP_INTERFACE

// dynamic/quicklaunch
#define QTOPIA_MAIN \
    QTOPIA_SETUP_APP_MAP \
    QTOPIA_QUICKLAUNCH_IMPL

#else // QTOPIA_APP_INTERFACE

// dynamic/normal
#define QTOPIA_MAIN \
    QTOPIA_SETUP_APP_MAP \
    QTOPIA_MAIN_IMPL

#endif // QTOPIA_APP_INTERFACE

#endif // QTOPIA_MAIN

#endif // __QPE_APPLICATION_H__

