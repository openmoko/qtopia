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
#ifndef QPASSWORDDIALOG_H
#define QPASSWORDDIALOG_H

#include <QDialog>

#include <qtopiaglobal.h>
#include <qstring.h>

class QPasswordWidget;

class QTOPIA_EXPORT QPasswordDialog : public QDialog
{
    Q_OBJECT
public:
    enum InputMode { Crypted, Plain, Pin };

    explicit QPasswordDialog( QWidget* parent = 0, Qt::WFlags flags = 0 );
    ~QPasswordDialog();

    void setPrompt( const QString& prompt );
    QString prompt() const;

    void setInputMode( QPasswordDialog::InputMode mode );
    QPasswordDialog::InputMode inputMode() const;

    void reset();
    QString password() const;

    static QString getPassword( QWidget* parent,
                                const QString& prompt,
                                InputMode mode = QPasswordDialog::Crypted,
                                bool last = true );

    static void authenticateUser( QWidget* parent,
                                  bool atPowerOn = false );

    static bool authenticateUser( const QString &text );

private:
    QPasswordWidget* m_passw;
};

#endif // QPASSWORDDIALOG_H
