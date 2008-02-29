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

#ifndef HAVE_PDACOMPOSER_H
#define HAVE_PDACOMPOSER_H

#include "ui_writemailbase.h"
#include <qstring.h>

class AddAttDialog;
class QContent;
class MailMessage;
class QStringList;
class PDAComposer : public QWidget, Ui::WriteMailBase
{
    Q_OBJECT
public:
    PDAComposer( QWidget *parent, const char *name = 0 );
    virtual ~PDAComposer();

    QString to() const;
    void setTo( const QString &txt );
    QString cc() const;
    void setCc( const QString &txt );
    QString bcc() const;
    void setBcc( const QString &txt );
    QString subject() const;
    void setSubject( const QString &txt );
    void setFrom( const QString &txt );
    QString from() const;
    void setFromFields( const QStringList &fields );

    void setText( const QString &txt );
    QString text() const;

    bool hasContent() const;
    void getContent( MailMessage &mail ) const;

public slots:
    void clear();
    AddAttDialog *addAttDialog() const;
    void attach( const QContent &lnk );
    void attach( const QString &fileName );

    void toggleCc( bool t );
    void toggleBcc( bool t );
    void toggleFrom( bool t );
    void setWordWrap( bool w );
    void selectAttachments();

protected slots:
    void editRecipients();
    void updateAddAtt();

signals:
    void recipientsChanged();
    void contentChanged();

private:
    AddAttDialog *m_addAttDialog;

};

#endif
