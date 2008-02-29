/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef EMSCOMPOSER_H
#define EMSCOMPOSER_H

#include <qtextedit.h>

#include "composer.h"

class EMSComposer : public QTextEdit
{
    Q_OBJECT
public:
    EMSComposer( QWidget *parent = 0, const char *name = 0 );

protected:
    void keyPressEvent( QKeyEvent *e );
};

class EMSComposerInterface : public TextComposerInterface
{
    Q_OBJECT

public:
    EMSComposerInterface( QObject *parent = 0, const char *name = 0 );
    ~EMSComposerInterface();

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

private:
    EMSComposer *m_composer;
};

#endif
