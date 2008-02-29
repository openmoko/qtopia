/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "composer.h"

class AddAttDialog;
class QTextEdit;
class QLabel;
class QAction;
class GenericComposer : public QWidget
{
    Q_OBJECT
public:
    GenericComposer( QWidget *parent = 0, const char *name = 0 );
    ~GenericComposer();

    void setText( const QString &t );
    QString text() const;

signals:
    void contentChanged();

protected slots:
    void updateSmsLimitIndicator();
    void templateText();
#ifdef QTOPIA_PHONE
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

class GenericComposerInterface : public TextComposerInterface
{
    Q_OBJECT

public:
    GenericComposerInterface( QObject *parent = 0, const char *name = 0 );
    ~GenericComposerInterface();

    /*
    static QCString id();
    static ComposerType type();
    static QString displayName();
    */

    bool hasContent() const;
    void getContent( MailMessage &mail ) const;
    void setMailMessage( Email &mail );

    void setText( const QString &txt );

    QWidget *widget() const;

public slots:
    void clear();
    void attach( const QContent &lnk );
    void attach( const QString &fileName );

private:
    GenericComposer *m_composer;
};

#endif
