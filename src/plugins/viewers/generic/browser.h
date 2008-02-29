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

#ifndef BROWSER_H
#define BROWSER_H

#include <QList>
#include <QMailAddress>
#include <QMap>
#include <QString>
#include <QTextBrowser>
#include <QUrl>
#include <QVariant>

class QWidget;
class QMailMessage;
class QMailMessagePart;

class Browser: public QTextBrowser
{
    Q_OBJECT

public:
    Browser(QWidget *parent = 0);
    virtual ~Browser();

    void setResource( const QUrl& name, QVariant var );
    void clearResources();

    void setMessage( const QMailMessage& mail, bool plainTextMode );

    void scrollBy(int dx, int dy);

    virtual QVariant loadResource(int type, const QUrl& name);

    QList<QString> embeddedNumbers() const;

signals:
    void finished();

public slots:
    virtual void setSource(const QUrl &name);

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    void displayPlainText(const QMailMessage* mail);
    void displayHtml(const QMailMessage* mail);

    void setTextResource(const QUrl& name, const QString& textData);
    void setImageResource(const QUrl& name, const QByteArray& imageData);
    void setPartResource(const QMailMessagePart& part);

    QString renderPart(const QMailMessagePart& part);
    QString renderAttachment(const QMailMessagePart& part);

    QString describeMailSize(uint bytes) const;
    QString formatText(const QString& txt) const;
    QString smsBreakReplies(const QString& txt) const;
    QString noBreakReplies(const QString& txt) const;
    QString handleReplies(const QString& txt) const;
    QString buildParagraph(const QString& txt, const QString& prepend, bool preserveWs = false) const;
    QString encodeUrlAndMail(const QString& txt) const;
    QString listRefMailTo(const QList<QMailAddress>& list) const;
    QString refMailTo(const QMailAddress& address) const;
    QString refNumber(const QString& number) const;

    QMap<QUrl, QVariant> resourceMap;
    QString (Browser::*replySplitter)(const QString&) const;

    mutable QList<QString> numbers;
};

#endif // BROWSER_H

