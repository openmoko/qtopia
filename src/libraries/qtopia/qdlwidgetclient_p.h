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

#ifndef QDL_WIDGET_CLIENT_PRIVATE_H
#define QDL_WIDGET_CLIENT_PRIVATE_H

// Qt includes
#include <QTextEdit>
#include <QTextBrowser>

// Forward class declarations
class QDLClient;
class QDLLink;
class QDLWidgetClientPrivate;

// ============================================================================
//
// QDLClientWidgets
//
// ============================================================================

union QDLClientWidgets
{
    QTextEdit *te;
    QTextBrowser *tb;
};

// ============================================================================
//
// QDLWidgetClientPrivate
//
// ============================================================================

class QDLWidgetClientPrivate
{
public:
    explicit QDLWidgetClientPrivate( QWidget *widget );
    ~QDLWidgetClientPrivate();

    // Modification
    void insertText( const QString& text );
    void setText( const QString& text );
    void verifyLinks( QDLClient* client );

    // Access
    bool isValid() const;
    QString text() const;
    QWidget* widget() const;
    QString determineHint() const;

private:

    // Modification
    void setWidget( QWidget* widget );
    void setWidgetType( const QString& type );
    void breakLink( QDLClient* client, const int linkId );

    // Access
    QString widgetType() const;

    // Data members
    bool mValid;
    QDLClientWidgets mWidget;
    QString mWidgetType;
};

#endif //QDL_WIDGET_CLIENT_PRIVATE_H
