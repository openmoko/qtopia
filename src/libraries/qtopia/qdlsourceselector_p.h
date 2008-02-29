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

#ifndef QDL_SOURCE_SELECTOR_PRIVATE_H
#define QDL_SOURCE_SELECTOR_PRIVATE_H

// Qt includes
#include <QDialog>
#include <QList>
#include <QString>

// Qtopia includes
#include <QDSServiceInfo>

// Forward class declarations
class QDLSourceSelectorPrivate;

// ============================================================================
//
// QDLSourceSelector
//
// ============================================================================

class QDLSourceSelector : public QDialog
{
    Q_OBJECT
public:
    explicit QDLSourceSelector( const QMimeType& responseDataType,
                                QWidget *parent = 0,
                                Qt::WindowFlags fl = 0 );

    ~QDLSourceSelector();

    QList<QDSServiceInfo> selected() const;
    QSize sizeHint() const;

protected slots:
    void accept();

signals:
    void selected( const QList<QDSServiceInfo>& );

private:
    QDLSourceSelectorPrivate* d;
};

#endif //QDL_SOURCE_SELECTOR_PRIVATE_H
