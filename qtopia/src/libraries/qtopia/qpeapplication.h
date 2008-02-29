/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef __QPE_APPLICATION_H__
#define __QPE_APPLICATION_H__

#include <qtopia/qpeglobal.h>
#include <qapplication.h>
#include <qdialog.h>
#include <qtopia/timestring.h>

#ifdef QTOPIA_TEST
# include "qpetestslave.h"
#endif

class QCopChannel;
class QPEApplicationData;
struct QWSEvent;
struct QWSKeyEvent;
class PluginLibraryManager;

class QTOPIA_EXPORT QPEApplication : public QApplication
{
    Q_OBJECT
public:
    QPEApplication( int& argc, char **argv, Type=GuiClient );
    ~QPEApplication();

    static QString qpeDir();
    static QString documentDir();
    void applyStyle();
    static int defaultRotation();
    static void setDefaultRotation(int r);
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
	AlwaysOn
	,
	Number,
	PhoneNumber,
	Words,
	Text,
	Named,
    };

    enum screenSaverHint {
	Disable = 0,
	DisableLightOff = 1,
	DisableSuspend = 2,
	Enable = 100
    };

    static void setInputMethodHint( QWidget *, InputMethodHint, const QString& param=QString::null );
    static void setInputMethodHint( QWidget *, const QString& named );
    static InputMethodHint inputMethodHint( QWidget * );
    static QString inputMethodHintParam( QWidget * );

    void showMainWidget( QWidget*, bool nomax=FALSE );
    void showMainDocumentWidget( QWidget*, bool nomax=FALSE );
    static void showDialog( QDialog*, bool nomax=FALSE );   // libqtopia
    static int execDialog( QDialog*, bool nomax=FALSE );    // libqtopia
    static void setMenuLike( QDialog *, bool ); // libqtopia2
    static bool isMenuLike( const QDialog* ); // libqtopia2
    static void setTempScreenSaverMode(screenSaverHint);    // libqtopia

    static void setKeepRunning();
    static void setHideWindow(); // internal
    
    bool keepRunning() const;

    bool keyboardGrabbed() const;

    int exec();

#ifdef QTOPIA_INTERNAL_LOADTRANSLATIONS
    static void loadTranslations(const QStringList&);
#endif
#ifdef QTOPIA_INTERNAL_INITAPP
    void initApp( int argc, char **argv );
#endif

signals:
    void clientMoused();
    void timeChanged();
    void categoriesChanged();
    void clockChanged( bool pm );
    void volumeChanged( bool muted );
    void appMessage( const QCString& msg, const QByteArray& data);
    void weekChanged( bool startOnMonday );
    void dateFormatChanged( DateFormat );
    void flush();
    void reload();
    void linkChanged( const QString &linkFile );
    
private slots:
    void systemMessage( const QCString &msg, const QByteArray &data );
    void pidMessage( const QCString &msg, const QByteArray &data );
    void removeSenderFromStylusDict();
    void removeSenderFromIMDict();
    void hideOrQuit();
    void pluginLibraryManager(PluginLibraryManager**);
    void lineEditTextChange(const QString &);
    void multiLineEditTextChange();

    void removeFromWidgetFlags();
    

protected:
#if defined(QTOPIA_PHONE) || defined(QTOPIA_TEST)  // since not binary compatible
    bool notify(QObject*,QEvent*);
#endif
    bool qwsEventFilter( QWSEvent * );
    void internalSetStyle( const QString &style );
    void prepareForTermination(bool willrestart);
    virtual void restart();
    virtual void shutdown();
    bool eventFilter( QObject *, QEvent * );
    void timerEvent( QTimerEvent * );
    bool raiseAppropriateWindow();
    virtual void tryQuit();

private:
    void mapToDefaultAction( QWSKeyEvent *ke, int defKey );
    void processQCopFile();

#if defined(QTOPIA_INTERNAL_SENDINPUTHINT)
    static void sendInputHintFor(QWidget*,QEvent::Type);
#endif

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel *sysChannel;
    QCopChannel *pidChannel;
#endif
    QPEApplicationData *d;

    bool reserved_sh;

#if defined QTOPIA_TEST
public:
    QString appName();
    void stopCycleCount() { hasPerfMonitor = FALSE; };
    void startCycleCount() { hasPerfMonitor = TRUE; };
    QPETestSlave app_slave;
private:
    bool hasPerfMonitor;
#endif
};

#ifdef Q_OS_WIN32
#include <stdlib.h>
QTOPIA_EXPORT int setenv(const char* name, const char* value, int overwrite);
QTOPIA_EXPORT void unsetenv(const char *name);
#endif
#endif

// Application main/plugin macro magic

#include <qmap.h>
#include <qtopia/applicationinterface.h>
#include <qmetaobject.h>

typedef QWidget* (*qpeAppCreateFunc)(QWidget*,const char *,Qt::WFlags);
typedef QMap<QString,qpeAppCreateFunc> QPEAppMap;

#define QTOPIA_ADD_APPLICATION(NAME,IMPLEMENTATION) \
    static QWidget *create_ ## IMPLEMENTATION( QWidget *p, const char *n, Qt::WFlags f ) { \
	return new IMPLEMENTATION(p,n,f); } \
    QPEAppMap *qpeAppMap(); \
    static QPEAppMap::Iterator dummy_ ## IMPLEMENTATION = qpeAppMap()->insert(NAME,create_ ## IMPLEMENTATION);

#ifdef QTOPIA_NO_MAIN
#define QTOPIA_MAIN

#else

#ifdef QTOPIA_APP_INTERFACE

# define QTOPIA_MAIN \
    struct ApplicationImpl : public ApplicationInterface { \
	ApplicationImpl() : ref(0) {} \
	QRESULT queryInterface( const QUuid &uuid, QUnknownInterface **iface ) { \
	    *iface = 0; \
	    if ( uuid == IID_QUnknown ) *iface = this; \
	    else if ( uuid == IID_QtopiaApplication ) *iface = this; \
	    else return QS_FALSE; \
	    (*iface)->addRef(); \
	    return QS_OK; \
	} \
	virtual QWidget *createMainWindow( const QString &appName, QWidget *parent, const char *name, Qt::WFlags f ) { \
	    if ( qpeAppMap()->contains(appName) ) \
		return (*qpeAppMap())[appName](parent, name, f); \
	    return 0; \
	} \
	virtual QStringList applications() const { \
	    QStringList list; \
	    for ( QPEAppMap::Iterator it=qpeAppMap()->begin(); it!=qpeAppMap()->end(); ++it ) \
		list += it.key(); \
	    return list; \
	} \
	Q_REFCOUNT \
	private: \
	    ulong ref; \
    }; \
    QPEAppMap *qpeAppMap() { \
	static QPEAppMap *am = 0; \
	if ( !am ) am = new QPEAppMap(); \
	return am; \
    } \
    Q_EXPORT_INTERFACE() { Q_CREATE_INSTANCE( ApplicationImpl ) }

#else

# define QTOPIA_MAIN \
    QPEAppMap *qpeAppMap(); \
    int main( int argc, char ** argv ) { \
	QPEApplication a( argc, argv ); \
	QWidget *mw = 0; \
\
    QString executableName(argv[0]); \
    executableName = executableName.right(executableName.length() \
	    - executableName.findRev('/') - 1); \
\
	if ( qpeAppMap()->contains(executableName) ) \
	    mw = (*qpeAppMap())[executableName](0,0,0); \
	else if ( qpeAppMap()->count() ) \
	    mw = qpeAppMap()->begin().data()(0,0,0); \
	if ( mw ) { \
	    if ( mw->metaObject()->slotNames(true).contains("setDocument(const QString&)") ) \
		a.showMainDocumentWidget( mw ); \
	    else \
		a.showMainWidget( mw ); \
	    int rv = a.exec(); \
	    delete mw; \
	    return rv; \
	} else { \
	    return -1; \
	} \
    } \
    QPEAppMap *qpeAppMap() { \
	static QPEAppMap *am = 0; \
	if ( !am ) am = new QPEAppMap(); \
	return am; \
    } \

#endif
#endif
