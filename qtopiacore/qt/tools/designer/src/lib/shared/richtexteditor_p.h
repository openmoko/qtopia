/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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

#ifndef RICHTEXTEDITOR_H
#define RICHTEXTEDITOR_H

#include <QtGui/QTextEdit>
#include <QtGui/QDialog>
#include "shared_global_p.h"

class QToolBar;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT RichTextEditor : public QTextEdit
{
    Q_OBJECT
public:
    RichTextEditor(QWidget *parent = 0);
    void setDefaultFont(const QFont &font);

    QToolBar *createToolBar(QWidget *parent = 0);

public slots:
    void setFontBold(bool b);
    void setFontPointSize(double);
    void setText(const QString &text);
    QString text(Qt::TextFormat format) const;

signals:
    void textChanged();

private:
    Qt::TextFormat detectFormat() const;
};

class QDESIGNER_SHARED_EXPORT RichTextEditorDialog : public QDialog
{
    Q_OBJECT
public:
    RichTextEditorDialog(QWidget *parent = 0);
    RichTextEditor *editor();

private:
    RichTextEditor *m_editor;
};

} // namespace qdesigner_internal

#endif // RITCHTEXTEDITOR_H
