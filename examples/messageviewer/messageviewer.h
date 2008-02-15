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

#ifndef MESSAGEVIEWER_H
#define MESSAGEVIEWER_H
#include "ui_messageviewerbase.h"

class QMailId;

class MessageViewer : public QWidget, public Ui_MessageViewerBase
{
    Q_OBJECT
public:
    MessageViewer( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~MessageViewer();

private slots:
    void showMessageList();
    void viewMessage(const QMailId&);
    void showContactList();

private:
    QWidget* contactSelector;
    QWidget* messageSelector;
};

#endif // MESSAGEVIEWER_H
