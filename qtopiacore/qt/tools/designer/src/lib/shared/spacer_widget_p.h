/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//


#ifndef SPACER_WIDGET_H
#define SPACER_WIDGET_H

#include "shared_global_p.h"

#include <QtGui/QWidget>
#include <QtGui/QSizePolicy>

class QDesignerFormWindowInterface;

class QDESIGNER_SHARED_EXPORT Spacer: public QWidget
{
    Q_OBJECT

    Q_ENUMS(SizeType)

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(QSizePolicy::Policy sizeType READ sizeType WRITE setSizeType)
    Q_PROPERTY(QSize sizeHint READ sizeHint WRITE setSizeHint DESIGNABLE true STORED true)

public:

    Spacer(QWidget *parent = 0);

    QSize minimumSize() const;

    QSize sizeHint() const;
    void setSizeHint(const QSize &s);

    QSizePolicy::Policy sizeType() const;
    void setSizeType(QSizePolicy::Policy t);

    Qt::Alignment alignment() const;
    Qt::Orientation orientation() const;

    void setOrientation(Qt::Orientation o);
    void setInteraciveMode(bool b) { interactive = b; };

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent* e);
    void updateMask();

private:
    QDesignerFormWindowInterface *m_formWindow;
    Qt::Orientation orient;
    bool interactive;
    QSize sh;
};

#endif // SPACER_WIDGET_H
