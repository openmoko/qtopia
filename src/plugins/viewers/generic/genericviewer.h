/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef GENERICVIEWER_H
#define GENERICVIEWER_H

#include <QObject>
#include <QString>

#include <qtopia/mail/qmailviewer.h>
#include <qtopia/mail/qmailviewerplugin.h>

class QAction;
class QMailMessage;
class Browser;

// A generic viewer able to show email, SMS or basic MMS
class GenericViewer : public QMailViewerInterface
{
    Q_OBJECT

public:
    GenericViewer(QWidget* parent = 0);
    virtual ~GenericViewer();

    virtual void scrollToAnchor(const QString& a);

    virtual QWidget *widget() const;

    virtual void addActions(QMenu* menu) const;

signals:
    void finished();

public slots:
    virtual bool setMessage(const QMailMessage& mail);
    virtual void setResource(const QUrl& name, QVariant var);
    virtual void clear();

    virtual void action(QAction* action);

    virtual void linkClicked(const QUrl& link);

protected slots:
    virtual void linkHighlighted(const QUrl& link);

private:
    virtual void setPlainTextMode(bool plainTextMode);
    virtual void print() const;

    Browser* browser;
    QAction* plainTextModeAction;
    QAction* richTextModeAction;
    QAction* printAction;
    QAction* dialAction;
    const QMailMessage* message;
    bool plainTextMode;
    bool containsNumbers;
};

class GenericViewerPlugin : public QMailViewerPlugin
{
    Q_OBJECT

public:
    GenericViewerPlugin();

    virtual QString key() const;
    virtual bool isSupported( QMailViewerFactory::ContentType type ) const;

    QMailViewerInterface* create( QWidget* parent );
};

#endif // GENERICVIEWER_H
