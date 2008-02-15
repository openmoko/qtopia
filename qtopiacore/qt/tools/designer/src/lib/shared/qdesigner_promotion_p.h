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

#ifndef QDESIGNERPROMOTION_H
#define QDESIGNERPROMOTION_H

#include "shared_global_p.h"

#include <QtDesigner/QDesignerPromotionInterface>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

    class  QDESIGNER_SHARED_EXPORT  QDesignerPromotion : public QDesignerPromotionInterface
    {
    public:
        QDesignerPromotion(QDesignerFormEditorInterface *core);

        virtual PromotedClasses promotedClasses() const;

        virtual QSet<QString> referencedPromotedClassNames() const;

        virtual bool addPromotedClass(const QString &baseClass,
                                      const QString &className,
                                      const QString &includeFile,
                                      QString *errorMessage);

        virtual bool removePromotedClass(const QString &className, QString *errorMessage);

        virtual bool changePromotedClassName(const QString &oldclassName, const QString &newClassName, QString *errorMessage);

        virtual bool setPromotedClassIncludeFile(const QString &className, const QString &includeFile, QString *errorMessage);

        virtual QList<QDesignerWidgetDataBaseItemInterface *> promotionBaseClasses() const;

    private:
        bool canBePromoted(const QDesignerWidgetDataBaseItemInterface *) const;
        void refreshObjectInspector();

        QDesignerFormEditorInterface *m_core;
    };
}

#endif // QDESIGNERPROMOTION_H
