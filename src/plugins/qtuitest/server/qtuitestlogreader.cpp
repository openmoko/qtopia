/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "qtuitestlogreader_p.h"

#include <QProcess>
#include <QStringList>

class QtUiTestLogReaderPrivate
{
public:
    void _q_onReadyRead();
    void _q_onError();
    void _q_onFinished();

    void startNextCommand();

    QtUiTestLogReader*          q;
    bool                        running;
    QStringList                 commands;
    QStringList::const_iterator currentCommand;
    QString                     error;
    QProcess                    process;
};

/*
    QtUiTestLogReader provides an interface for sending system logs to a connected system test.

    QtUiTestLogReader runs one or more commands and pipes the output to the system test.
    The commands which are run are determined by the system test and may be device specific.

    Most devices will use a command list like ("logread", "logread -f") which will retrieve the
    entire current and future log contents.
*/

/*
    Constructs a new log reader.
*/
QtUiTestLogReader::QtUiTestLogReader(QObject* parent)
    : QObject(parent)
    , d(new QtUiTestLogReaderPrivate)
{
    d->q        = this;
    d->running  = false;
    d->process.setParent(this);
    d->process.setObjectName("qtuitestlogreader_process");
    if (!connect(&d->process, SIGNAL(readyRead()), this, SLOT(_q_onReadyRead())))
        Q_ASSERT(0);
    if (!connect(&d->process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(_q_onFinished())))
        Q_ASSERT(0);
    if (!connect(&d->process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(_q_onError())))
        Q_ASSERT(0);
}

QtUiTestLogReader::~QtUiTestLogReader()
{
    d->q = 0;
    delete d;
    d = 0;
}

QString QtUiTestLogReader::errorString() const
{ return d->error; }

/*
    Starts running the log read \a commands in the background.

    After this function has been called, log() will be repeatedly
    emitted for log messages read.
    If the log read commands cannot be run or some other error occurs, error()
    will be emitted with the error string.
*/
void QtUiTestLogReader::start(QStringList const& commands)
{
    if (d->running) {
        d->error = "start() called while already running.";
        emit error(d->error);
        return;
    }
    if (!commands.count()) {
        d->error = "No log read commands have been set.";
        emit error(d->error);
        emit finished();
        return;
    }

    d->running = true;
    d->commands = commands;
    d->currentCommand = d->commands.constBegin();
    d->startNextCommand();
}

void QtUiTestLogReaderPrivate::_q_onReadyRead()
{
    QStringList lines;
    while (process.canReadLine()) {
        QByteArray line(process.readLine());
        line.chop(1);
        lines << line;
    }

    emit q->log(lines);
}

void QtUiTestLogReaderPrivate::_q_onFinished()
{
    int code                    = process.exitCode();
    QProcess::ExitStatus status = process.exitStatus();

    if (code != 0) {
        error = QString("%1 exited with exit code %2").arg(*currentCommand).arg(code);
        emit q->error(error);
        if (running) {
            running = false;
            emit q->finished();
        }
    } else if (status != QProcess::NormalExit) {
        error = QString("%1 exited abnormally").arg(*currentCommand);
        emit q->error(error);
        if (running) {
            running = false;
            emit q->finished();
        }
    } else {
        ++currentCommand;
        startNextCommand();
    }
}

void QtUiTestLogReaderPrivate::_q_onError()
{
    QString errorString;

    switch (process.error()) {
    case QProcess::FailedToStart:
        errorString = "command not found or not executable";
        break;
    case QProcess::Crashed:
        errorString = "command crashed";
        break;
    default:
        errorString = process.errorString();
    }

    error = QString("Error running %1: %2").arg(*currentCommand).arg(errorString);
    emit q->error(error);
    if (running) {
        running = false;
        emit q->finished();
    }
}

void QtUiTestLogReaderPrivate::startNextCommand()
{
    if (currentCommand == commands.constEnd()) {
        if (running) {
            running = false;
            emit q->finished();
        }
        return;
    }
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(*currentCommand, QIODevice::ReadOnly);
    process.closeWriteChannel();
}

#include "moc_qtuitestlogreader_p.cpp"

