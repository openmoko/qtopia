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

#ifndef GENERICCOMPOSER_H
#define GENERICCOMPOSER_H

#include <QLineEdit>

#include <qtopia/mail/qmailcomposer.h>
#include <qtopia/mail/qmailcomposerplugin.h>

class AddAttDialog;
class QTextEdit;
class QLabel;
class QAction;

class GenericComposer : public QWidget
{
    Q_OBJECT
public:
    GenericComposer( QWidget *parent = 0 );
    ~GenericComposer();

    void setText( const QString &t, const QString &type );
    QString text() const;
    bool isVCard() const { return m_vCard; }

    void addActions(QMenu* menu);

signals:
    void contentChanged();
    void finished();

protected slots:
    void updateSmsLimitIndicator();
    void templateText();
    void textChanged();

#ifndef QTOPIA_NO_SMS
    void smsLengthInfo(uint& estimatedBytes, bool& isUnicode);
    int smsCountInfo();
#endif

private:
    QTextEdit *m_textEdit;
    QLabel *m_smsLimitIndicator;
    QAction *m_showLimitAction;
    QAction *m_templateTextAction;
    bool m_vCard;
    QString m_vCardData;
};

class GenericComposerInterface : public QMailComposerInterface
{
    Q_OBJECT

public:
    GenericComposerInterface( QWidget *parent = 0 );
    ~GenericComposerInterface();

    bool isEmpty() const;
    QMailMessage message() const;

    QWidget *widget() const;

    virtual void addActions(QMenu* menu) const;

public slots:
    void setMessage( const QMailMessage &mail );
    void setText( const QString &txt, const QString &type );
    void clear();
    void attach( const QContent &lnk, QMailMessage::AttachmentsAction action );

private:
    GenericComposer *m_composer;
};

class GenericComposerPlugin : public QMailComposerPlugin
{
    Q_OBJECT

public:
    GenericComposerPlugin();

    virtual QString key() const;
    virtual QMailMessage::MessageType messageType() const;

    virtual QString name() const;
    virtual QString displayName() const;
    virtual QIcon displayIcon() const;

    virtual bool isSupported( QMailMessage::MessageType type ) const;

    QMailComposerInterface* create( QWidget* parent );
};

#endif
