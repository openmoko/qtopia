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
#include "qtsingleapplication.h"
#include <qt_windows.h>
#include <qwidget.h>

class QtSingletonSysPrivate : public QWidget
{
public:
    QtSingletonSysPrivate()
        : QWidget(), listening(FALSE)
    {
    }
    uint listening :1;

protected:
    bool winEvent(MSG *msg, long *result)
    {
        if (msg->message != WM_COPYDATA)
            return FALSE;

        Q_ASSERT(msg->hwnd == winId());
        COPYDATASTRUCT *data = (COPYDATASTRUCT*)msg->lParam;
        QString message = QString::fromUtf16((unsigned short*)data->lpData);

        emit ((QtSingleApplication*)qApp)->messageReceived( message );

	if (result)
	  *result = 0;
        return TRUE;
    }

};

static HANDLE createLockedMutex(const QString &id)
{
    HANDLE mutex;
    QT_WA({
        mutex = CreateMutex(0, FALSE, (TCHAR*)id.utf16());
    }, {
       mutex = CreateMutexA(0, FALSE, id.toLocal8Bit().data());
    });

    switch (WaitForSingleObject(mutex, INFINITE)) {
    case WAIT_ABANDONED:
    case WAIT_TIMEOUT:
        CloseHandle(mutex);
        mutex = 0;
        break;
    default: // WAIT_OBJECT_0
        break;
    }

    return mutex;
}

static inline void closeLockedMutex(HANDLE handle)
{
    if (!handle)
        return;

    ReleaseMutex(handle);
    CloseHandle(handle);
}

static HWND findWindow(const QString &id)
{
    HANDLE mutex = createLockedMutex(id);

    HWND hwnd;
    QString wid = id + "_QtSingleApplicationWindow";

    QT_WA( {
	hwnd = ::FindWindow(L"QWidget", (TCHAR*)wid.utf16());
    }, {
	hwnd = ::FindWindowA("QWidget", wid.toLocal8Bit().data());
    } )

    closeLockedMutex(mutex);

    return hwnd;
}

void QtSingleApplication::sysInit()
{
    sysd = new QtSingletonSysPrivate;
}

void QtSingleApplication::sysCleanup()
{
    HANDLE mutex = createLockedMutex(id());

    delete sysd;

    closeLockedMutex(mutex);
}

void QtSingleApplication::initialize( bool activate )
{
    if (sysd->listening)
	return;

    HANDLE mutex = createLockedMutex(id());
    
    sysd->listening = true;
    sysd->winId();
    sysd->setWindowTitle(id() + "_QtSingleApplicationWindow");

    closeLockedMutex(mutex);

    if ( activate )
	connect(this, SIGNAL(messageReceived(QString)),
		this, SLOT(activateMainWidget()));
}

bool QtSingleApplication::isRunning() const
{
    return findWindow(id()) != 0;
}

bool QtSingleApplication::sendMessage( const QString &message, int timeout )
{
    HWND hwnd = findWindow(id());
    if ( !hwnd )
	return FALSE;

    COPYDATASTRUCT data;
    data.dwData = 0;
    data.cbData = (message.length()+1) * sizeof(QChar);
    data.lpData = (void*)message.utf16();
    DWORD result;
    LRESULT res;
    QT_WA( {
	res = SendMessageTimeout(hwnd, WM_COPYDATA, 0/*hwnd sender*/, (LPARAM)&data, 
				 SMTO_ABORTIFHUNG,timeout,&result);
    }, {
	res = SendMessageTimeoutA(hwnd, WM_COPYDATA, 0/*hwnd sender*/, (LPARAM)&data, 
				  SMTO_ABORTIFHUNG,timeout,&result);
    } )
    return res != 0;
}
