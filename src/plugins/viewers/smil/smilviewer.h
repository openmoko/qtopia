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

#ifndef SMILVIEWER_H
#define SMILVIEWER_H

#include <QEvent>
#include <QMap>
#include <QString>
#include <QVariant>

#include <qtopia/mail/qmailviewer.h>
#include <qtopia/mail/qmailviewerplugin.h>

class QIODevice;
class QUrl;
class QWidget;

class QMailMessage;
class SmilDataSource;
class SmilView;

// A viewer able to playback a SMIL MMS mail
class SmilViewer : public QMailViewerInterface
{
    Q_OBJECT

public:
    SmilViewer(QWidget* parent = 0);
    virtual ~SmilViewer();

    virtual QWidget *widget() const;

public slots:
    virtual bool setMessage(const QMailMessage& mail);
    virtual void clear();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event);

private slots:
    void requestTransfer(SmilDataSource* dataSource, const QString &src);
    void cancelTransfer(SmilDataSource *dataSource, const QString &src);

private:
    QString findStartMarker() const;
    void tweakView();
    void advanceSlide();

    SmilView* view;
    QMap<SmilDataSource*,QIODevice*> transfers;
    const QMailMessage* mail;
};

class SmilViewerPlugin : public QMailViewerPlugin
{
    Q_OBJECT

public:
    SmilViewerPlugin();

    virtual QString key() const;
    virtual bool isSupported( QMailViewerFactory::ContentType type ) const;

    QMailViewerInterface* create( QWidget* parent );
};

#endif // SMILVIEWER_H

