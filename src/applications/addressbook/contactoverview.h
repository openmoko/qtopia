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
#ifndef CONTACTOVERVIEW_H
#define CONTACTOVERVIEW_H

#include <qtopia/pim/qcontact.h>

#include <QWidget>

class QAbstractButton;
class QLabel;
class QScrollArea;

class ContactOverview : public QWidget
{
    Q_OBJECT

public:
    ContactOverview( QWidget *parent );
    virtual ~ContactOverview();

    QContact entry() const {return ent;}

    void setModel(QContactModel *);

public slots:
    void init( const QContact &entry );

    void updateCommands();

signals:
    void externalLinkActivated();
    void backClicked();
    void callContact();
    void textContact();
    void emailContact();
    void editContact();

protected:
    void keyPressEvent( QKeyEvent *e );
    void focusInEvent( QFocusEvent *e );
    void resizeEvent( QResizeEvent *e);

private:
    QContact ent;
    bool mInitedGui;
    QAbstractButton *mCall;
    QAbstractButton *mText;
    QAbstractButton *mEmail;
    QAbstractButton *mEdit;
    QList<QAbstractButton*> buttons;
    QLabel* mPortrait;
    QLabel *mNameLabel;
    QContactModel *mModel;
    QScrollArea *mScrollArea;
    bool mSetFocus;
};

#endif // CONTACTOVERVIEW_H

