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

#include <qevent.h>

#include "emscomposer.h"

EMSComposer::EMSComposer( QWidget *parent, const char *name )
    : QMultiLineEdit( parent, name )
{
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                                    QSizePolicy::Expanding ) );
}

void EMSComposer::keyPressEvent( QKeyEvent *e )
{
    if( e->key() == Key_Back )
        e->ignore();
    else
        QMultiLineEdit::keyPressEvent( e );
}

EMSComposerInterface::EMSComposerInterface( QObject *parent, const char *name )
    : TextComposerInterface( parent, name )
{
    QWidget *par = 0;
    if( parent && parent->isWidgetType() )
        par = (QWidget *)parent;
    m_composer = new EMSComposer( par, name );
    connect( m_composer, SIGNAL(textChanged()), this, SIGNAL(contentChanged()) );
}

EMSComposerInterface::~EMSComposerInterface()
{
    delete m_composer;
}

/*
QCString EMSComposerInterface::id()
{
    QCString t = QString::number( type() ).toLatin1();
    return t + "-Default"; // default ems composer
}

ComposerInterface::ComposerType EMSComposerInterface::type()
{
    return ComposerInterface::EMS;
}

QString EMSComposerInterface::displayName()
{
    return tr("Extended SMS");
}
*/

bool EMSComposerInterface::hasContent() const
{
    return !m_composer->text().isEmpty();
}

void EMSComposerInterface::setMailMessage( Email &mail )
{
    setText( mail.plainTextBody() );
}

void EMSComposerInterface::getContent( MailMessage &mail ) const
{
    if( !hasContent() )
        return;
    mail.setPlainTextBody( m_composer->text() );
    // TODO : set EMS specific headers
}


void EMSComposerInterface::clear()
{
    m_composer->clear();
}

void EMSComposerInterface::setText( const QString &txt )
{
    m_composer->setText( txt );
}

QWidget *EMSComposerInterface::widget() const
{
    return m_composer;
}
