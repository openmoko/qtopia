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

#ifndef _e1_HEADER_H_
#define _e1_HEADER_H_

#include <QWidget>
#include <QStringList>
#include <qvaluespace.h>
#include <QMap>

class E1HeaderAlertButton;
class E1Header : public QWidget
{
    Q_OBJECT
public:
    E1Header(QWidget * = 0, Qt::WFlags = 0);

protected:
    virtual QSize sizeHint() const;
    virtual void paintEvent(QPaintEvent *);

private slots:
    void setAlertEnabled( bool e );
    void alertClicked();
    void clicked(const QString &);
    void missedCallsChanged();
    void newMessagesChanged();

private:
    QValueSpaceItem m_missedCallsVS;
    QValueSpaceItem m_newMessagesVS;
    QStringList m_alertStack;
    E1HeaderAlertButton* m_alertButton;
};

#endif // _e1_HEADER_H_

