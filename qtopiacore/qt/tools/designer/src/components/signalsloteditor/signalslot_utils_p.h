/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef SIGNALSLOTUTILS_P_H
#define SIGNALSLOTUTILS_P_H

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

#include <QtCore/QString>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;

namespace qdesigner_internal {

enum MemberType { SignalMember, SlotMember };

// member to class name
QMap<QString, QString> getSignals(QDesignerFormEditorInterface *core, QObject *object, bool showAll);
QMap<QString, QString> getMatchingSlots(QDesignerFormEditorInterface *core, QObject *object,
            const QString &signalSignature, bool showAll);

bool memberFunctionListContains(QDesignerFormEditorInterface *core, QObject *object, MemberType type, const QString &signature);

// Members functions listed by class they were inherited from
struct ClassMemberFunctions
{
    ClassMemberFunctions() {}
    ClassMemberFunctions(const QString &_class_name);

    QString m_className;
    QStringList m_memberList;
};

typedef QList<ClassMemberFunctions> ClassesMemberFunctions;

// Return classes and members in reverse class order to
// populate of the combo of the ToolWindow.

ClassesMemberFunctions reverseClassesMemberFunctions(const QString &obj_name, MemberType member_type,
                                                     const QString &peer, QDesignerFormWindowInterface *form);

bool signalMatchesSlot(QDesignerFormEditorInterface *core, const QString &signal, const QString &slot);

QString realObjectName(QDesignerFormEditorInterface *core, QObject *object);

bool isQt3Signal(const QString &className, const QString &signalSignature);
bool isQt3Slot(const QString &className, const QString &signalSignature);

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SIGNALSLOTUTILS_P_H
