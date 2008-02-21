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

#ifndef EMAILCOMPOSER_H
#define EMAILCOMPOSER_H

#include <QContent>
#include <QList>
#include <QTextEdit>

#include <qtopia/mail/qmailcomposer.h>
#include <qtopia/mail/qmailcomposerplugin.h>

class AddAttDialog;
class QLabel;
class QWidget;

class EmailComposer : public QTextEdit
{
    Q_OBJECT

public:
    EmailComposer( QWidget *parent = 0, const char* name = 0 );

    AddAttDialog *addAttDialog();

    using QTextEdit::setPlainText;

    void setPlainText( const QString& text, const QString& signature );

    bool isEmpty() const;

signals:
    void contentChanged();
    void attachmentsChanged();
    void finished();

protected slots:
    void selectAttachment();
    void updateLabel();
    void setCursorPosition();

protected:
    void keyPressEvent( QKeyEvent *e );

private:
    AddAttDialog *m_addAttDialog;
    int m_index;
};

class EmailComposerInterface : public QMailComposerInterface
{
    Q_OBJECT

public:
    EmailComposerInterface( QWidget *parent = 0 );
    ~EmailComposerInterface();

    bool isEmpty() const;
    QMailMessage message() const;

    QWidget *widget() const;

    virtual void addActions(QMenu* menu) const;

public slots:
    void setMessage( const QMailMessage &mail );
    void setText( const QString &txt, const QString &type );
    void clear();
    void attach( const QContent &lnk, QMailMessage::AttachmentsAction = QMailMessage::LinkToAttachments );
    void setSignature( const QString &sig );
    void attachmentsChanged();

private:
    EmailComposer *m_composer;
    QLabel *m_label;
    QWidget *m_widget;
    QList<QContent> m_temporaries;
    QString m_signature;
};

class EmailComposerPlugin : public QMailComposerPlugin
{
    Q_OBJECT

public:
    EmailComposerPlugin();

    virtual QString key() const;
    virtual QMailMessage::MessageType messageType() const;

    virtual QString name() const;
    virtual QString displayName() const;
    virtual QIcon displayIcon() const;

    QMailComposerInterface* create( QWidget* parent );
};

#endif
