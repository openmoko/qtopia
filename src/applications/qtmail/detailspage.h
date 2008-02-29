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

#ifndef DETAILSPAGE_H
#define DETAILSPAGE_H

#include <qwidget.h>
#include <qstring.h>
#include <qstringlist.h>

#include <qtopia/mail/mailmessage.h>

class QLineEdit;
class QHBoxLayout;
class QComboBox;
class QToolButton;
class QLabel;
class QCheckBox;

class DetailsPage : public QWidget
{
    Q_OBJECT

public:

    DetailsPage( QWidget *parent, const char *name = 0 );

    void setTo( const QString &a_to );
    QString to() const;

    void setBcc( const QString &a_bcc );
    QString bcc() const;

    void setCc( const QString &a_cc );
    QString cc() const;

    QString subject() const;
    void setSubject( const QString &sub );

    QString from() const;
    void setFrom( const QString &from );
    void setFromFields( const QStringList &from );

    void setType( int t );

    void getDetails( MailMessage &mail );

    bool eventFilter(QObject* obj, QEvent* event);

public slots:
    void clear();

signals:
    void recipientsChanged();

private slots:
    void editRecipients();
    void copy();
    void paste();

private:
    bool m_allowPhoneNumbers, m_allowEmails;
    bool m_ignoreFocus;
    int m_type;
    QCheckBox *m_readReplyField;
    QCheckBox *m_deliveryReportField;
    QLineEdit *m_subjectField;
    QLabel *m_subjectFieldLabel, *m_fromFieldLabel;
    QComboBox *m_fromField;
    QLabel *m_toFieldLabel, *m_ccFieldLabel, *m_bccFieldLabel;
    QLineEdit *m_ccField, *m_bccField, *m_toField;
    QHBoxLayout *m_toBox, *m_ccBox, *m_bccBox;
    QToolButton *m_toPicker, *m_ccPicker, *m_bccPicker;
};

#endif
