/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FLAGBOX_P_H
#define FLAGBOX_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "propertyeditor_global.h"
#include "flagbox_model_p.h"

#include <QtGui/QComboBox>

namespace qdesigner_internal {

class QT_PROPERTYEDITOR_EXPORT FlagBox: public QComboBox
{
    Q_OBJECT
public:
    FlagBox(QWidget *parent = 0);
    virtual ~FlagBox();

    inline FlagBoxModelItem itemAt(int index) const
    { return m_model->itemAt(index); }

    inline FlagBoxModelItem &item(int index)
    { return m_model->item(index); }

    inline QList<FlagBoxModelItem> items() const
    { return m_model->items(); }

    inline void setItems(const QList<FlagBoxModelItem> &items)
    { m_model->setItems(items); }

    inline void hidePopup() {}

private slots:
    void slotActivated(int index);

private:
    FlagBoxModel *m_model;
};

}  // namespace qdesigner_internal

#endif // FLAGBOX_P_H
