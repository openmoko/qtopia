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

#ifndef EMAILCOMPOSER_H
#define EMAILCOMPOSER_H

#include <QTextEdit>

#include "composer.h"

class AddAttDialog;

class EmailComposer : public QTextEdit
{
    Q_OBJECT
public:
    EmailComposer( QWidget *parent = 0, const char *name = 0 );

    AddAttDialog *addAttDialog();

signals:
    void contentChanged();

protected slots:
    void selectAttachment();

protected:
    void keyPressEvent( QKeyEvent *e );

private:
    AddAttDialog *m_addAttDialog;
};

class EmailComposerInterface : public TextComposerInterface
{
    Q_OBJECT

public:
    EmailComposerInterface( QObject *parent = 0, const char *name = 0 );
    ~EmailComposerInterface();

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
    EmailComposer *m_composer;
};



#endif
