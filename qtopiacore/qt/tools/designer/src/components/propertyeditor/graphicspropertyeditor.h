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

#ifndef GRAPHICSPROPERTYSEDITOR_H
#define GRAPHICSPROPERTYSEDITOR_H

#include "qpropertyeditor_items_p.h"

#include <QtGui/QWidget>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>

class QComboBox;
class QToolButton;

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class IconProperty : public AbstractProperty<QIcon>
{
public:
    IconProperty(QDesignerFormEditorInterface *core, const QIcon &value, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;
    QVariant decoration() const;

    QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver) const;
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);

    QDesignerFormEditorInterface *core() const { return m_core; }

private:
    QDesignerFormEditorInterface *m_core;
};

class PixmapProperty : public AbstractProperty<QPixmap>
{
public:
    PixmapProperty(QDesignerFormEditorInterface *core, const QPixmap &pixmap, const QString &name);

    void setValue(const QVariant &value);
    QString toString() const;
    QVariant decoration() const;

    QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver) const;
    void updateEditorContents(QWidget *editor);
    void updateValue(QWidget *editor);

    QDesignerFormEditorInterface *core() const { return m_core; }

private:
    QDesignerFormEditorInterface *m_core;
};

// This handles editing of pixmap and icon properties
class GraphicsPropertyEditor : public QWidget
{
    Q_OBJECT

public:
    GraphicsPropertyEditor(QDesignerFormEditorInterface *core, const QIcon &pm, QWidget *parent);
    GraphicsPropertyEditor(QDesignerFormEditorInterface *core, const QPixmap &pixmap, QWidget *parent);
    ~GraphicsPropertyEditor();

    void setIcon(const QIcon &pm);
    void setPixmap(const QPixmap &pm);
    QIcon icon() const { return m_mode == Icon ? m_icon : QIcon(); }
    QPixmap pixmap() const { return m_mode == Pixmap ? m_pixmap : QPixmap(); }

signals:
    void iconChanged(const QIcon &pm);
    void pixmapChanged(const QPixmap &pm);

private slots:
    void showDialog();
    void comboActivated(int idx);

private:
    void init();
    void populateCombo();
    int indexOfIcon(const QIcon &icon);
    int indexOfPixmap(const QPixmap &pixmap);

    enum Mode { Icon, Pixmap };
    const Mode m_mode;

    QDesignerFormEditorInterface *m_core;
    QComboBox *m_combo;
    QToolButton *m_button;
    QIcon m_icon;
    QPixmap m_pixmap;
};
}

#endif
