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

#ifndef PRINTSERVER_H
#define PRINTSERVER_H

#include <qtopiaabstractservice.h>
#include <QVariant>
#include <QList>
#include <QPrinter>

class QtopiaApplication;
class QtopiaPrinterInterface;
class QPluginManager;
class PrintServerPrivate;
class QListWidgetItem;

enum PrintJobType { PPK, File, Html };

typedef struct PrintJobInfo {
    PrintJobType type;
    QVariant properties;
};

class PrintServer : public QObject
{
    Q_OBJECT
    friend class PrintService;

public:
    PrintServer(QObject *parent);
    ~PrintServer();

protected:
    void enqueuePPKPrintJob(const QVariant &properties);
    void enqueueFilePrintJob(const QString &fileName);
    void enqueueFilePrintJob(const QString &fileName, const QString &mimeType);
    void enqueueHtmlPrintJob(const QString &html);

private slots:
    void receive(const QString &, const QByteArray &);
    void pluginSelected(QListWidgetItem *);
    void dispatchPrintJob(QtopiaPrinterInterface *);
    void done( bool );
    void cancelJob();

private:
    void selectPrinterPlugin();

private:
    PrintServerPrivate *d;
};

class PrintService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class PrintServer;

private:
    PrintService( PrintServer *parent )
        : QtopiaAbstractService( "Print", parent )
        { this->parent = parent; publishAll(); }
public:
    ~PrintService();

public slots:
    void print(QString);
    void print(QString,QString);
    void printHtml(QString);

private:
    PrintServer *parent;
};

#endif
