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



#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QLayout>
#include <QTextEdit>
#include <QCheckBox>
#include <QDateTime>
#include <QGroupBox>
#include <QToolButton>
#include <QScrollArea>

#ifdef QTOPIA_PHONE
#include "ui_searchviewbasephone.h"
#else
#include "ui_searchviewbase.h"
#endif
#include "search.h"
#include "email.h"
#include "account.h"
#include "emaillistitem.h"

class SearchView : public QDialog, public Ui::SearchViewBase
{
    Q_OBJECT
public:
    SearchView(bool query, QWidget* parent = 0, Qt::WFlags = 0);
    ~SearchView();
    Search *getSearch();
    void setSearch(Search *in);
    void setQueryBox(QString box);

public slots:
    void dateAfterChanged( const QDate & );
    void dateBeforeChanged( const QDate & );




private:
    void init();

private:
    QDate dateBefore, dateAfter;

    QScrollArea *sv;

    bool queryType;
};

#endif
