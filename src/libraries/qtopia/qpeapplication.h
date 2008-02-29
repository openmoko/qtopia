/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
    };

    enum screenSaverHint {
	Disable = 0,
	DisableLightOff = 1,
	DisableSuspend = 2,
	Enable = 100
    };

    static void setInputMethodHint( QWidget *, InputMethodHint );
    static InputMethodHint inputMethodHint( QWidget * );

    void showMainWidget( QWidget*, bool nomax=FALSE );
    void showMainDocumentWidget( QWidget*, bool nomax=FALSE );
    static void showDialog( QDialog*, bool nomax=FALSE );   // libqtopia
    static int execDialog( QDialog*, bool nomax=FALSE );    // libqtopia
    static void setTempScreenSaverMode(screenSaverHint);    // libqtopia

    static void setKeepRunning();
    bool keepRunning() const;

    bool keyboardGrabbed() const;

    int exec();

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
    void hideOrQuit();
    void pluginLibraryManager(PluginLibraryManager**);

protected:
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

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel *sysChannel;
    QCopChannel *pidChannel;
#endif
    QPEApplicationData *d;

    bool reserved_sh;



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
	    if ( mw->metaObject()->slotNames().contains("setDocument(const QString&)") ) \
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
