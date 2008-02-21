/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _PHONETHEMEVIEW_H_
#define _PHONETHEMEVIEW_H_

#include <themedview.h>
#include <QWidget>
#include <QSet>

class PhoneThemedView : public ThemedView {
Q_OBJECT
public:
    PhoneThemedView(QWidget *parent=0, Qt::WFlags f=0);
    virtual ~PhoneThemedView();

    static QSet<PhoneThemedView *> themedViews();
    QWidget *newWidget(ThemeWidgetItem *, const QString &);

private slots:
    void myItemClicked(ThemeItem *item);

private:
    static QSet<PhoneThemedView *> m_themedViews;
};

#endif // _PHONETHEMEVIEW_H_
