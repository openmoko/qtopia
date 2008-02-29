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

#ifndef SXEMONITOR_H
#define SXEMONITOR_H

#include <qobject.h>
#include <qstring.h>
#include <qfilesystemwatcher.h>
#include <qfile.h>
#include <qtopiaservices.h>
#include <qvaluespace.h>

struct SxeLogEntry
{
        SxeLogEntry();
        enum LogEntryType{ Breach, FARExceeded, Incomplete, NonSxe};
        int pid;
        int progId;
        QString request;
        QString exe;
        QString cmdline;
        LogEntryType logEntryType;
        bool isValid()
        {
            return ((pid != -1 &&
                     progId != -1 &&
                     !exe.isEmpty() &&
                     logEntryType == Breach)
                     ||
                     (logEntryType == FARExceeded)
                     );
        }
        void reset();
    SxeLogEntry& parseLogEntry( QString logEntry );

    private:
        static QString stampFormat();
        static const QString sxeTag;
        static const QString farExceeded;
};

class SxeMonitor : public QObject
{
    Q_OBJECT
public:
    SxeMonitor();
    ~SxeMonitor();

    struct AppInfo
    {
        enum AppType
        {
            System,
            Sandboxed,
            QuicklaunchSystem,
            //QuicklaunchSandboxed (not supported yet)
            AllSandboxed,
            Other,
            Incomplete
        };

        AppInfo();
        AppType appType;
        QString identifier;
        QString userVisibleName;
        QString executable;
        int pid;
        bool isValid() {
            return ( (appType != AllSandboxed
                        && appType != Incomplete
                        && !identifier.isEmpty()
                        && !userVisibleName.isEmpty()
                        && !executable.isEmpty()
                        && pid != -1)
                     ||
                     (appType == AllSandboxed)
                   );
        }

        AppInfo& getAppInfo( const int procId, const QString exePath,
                                            const QString cmdline="" );
        QString toString() const;
        void reset();
    };


private slots:
    void init();
    void logUpdated();
    void sendDelayedReq();
    void discardKilledPid();

private:
    enum MessageType
    {
      DialogBreach,
      DialogLockdown,
      SmsPackageAppBreach,
      SmsSystemAppBreach,
      SmsLockdown
    };

    SxeMonitor( const SxeMonitor & );
    SxeMonitor &operator=( SxeMonitor const & );

    void killApplication( AppInfo app );
    void disableSandboxedApplication( AppInfo app ) const;
    void lockdown();
    void dispatchMessage( const MessageType type, const QStringList &args, bool delay );

    void processLogFile();
    QString readCmdline( int pid ) const;

    QString packagemanagerName() const;
    QFileSystemWatcher *logWatcher;

    QString logPath;
    QFile logFile;

    QList<QtopiaServiceRequest> delayedRequests;
    QString smsProgName;
    QString smsRequest;

    QList<QVariant> killedPids;
    QValueSpaceObject sxeVso;


    static const QString lidsTag;
    static const QString disabledTag;

    static const int maxRetries;
#ifdef QT_NO_QWS_VFB
    static qint64 maxLogSize;
    QString lidsStampFormat;
#endif

#ifdef TEST_SXEMONITOR
    friend class tst_SxeMonitor;
#endif
};

#endif // SXEMONITOR_H

