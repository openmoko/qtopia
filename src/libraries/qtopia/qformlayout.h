/****************************************************************************
**
** Copyright (C) 2004-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qt Toolkit.
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

#ifndef QFORMLAYOUT_H
#define QFORMLAYOUT_H


#include <qtopiaglobal.h>
#include <QLayout>

class QWidget;
class QLabel;
class QFormLayoutPrivate;

class QTOPIA_EXPORT QFormLayout : public QLayout
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QFormLayout)
    QDOC_PROPERTY(int horizontalSpacing READ horizontalSpacing WRITE setHorizontalSpacing)
    QDOC_PROPERTY(int verticalSpacing READ verticalSpacing WRITE setVerticalSpacing)

public:
    explicit QFormLayout(QWidget *parent);
    QFormLayout();
    ~QFormLayout();

    void addRow(const QString &label, QWidget *field);
    void addRow(const QString &label, QLayout *layout);
    void insertRow(int index, const QString &label, QWidget *field);
    void insertRow(int index, const QString &label, QLayout *layout);

    void addRow(QWidget *label, QWidget *field);
    void addRow(QWidget *label, QLayout *layout);
    void insertRow(int index, QWidget *label, QWidget *field);
    void insertRow(int index, QWidget *label, QLayout *layout);

    void addRow(QWidget *widget);
    void addRow(QLayout *layout);
    void insertRow(int index, QWidget *widget);
    void insertRow(int index, QLayout *layout);

    enum FormStyle {
        QtopiaDefaultStyle,
        QtopiaTwoLineStyle
    };

    void setHorizontalSpacing(int spacing);
    int horizontalSpacing() const;
    void setVerticalSpacing(int spacing);
    int verticalSpacing() const;

    //reimp
    void addItem(QLayoutItem *item);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);

    void setGeometry(const QRect &rect);
    QSize minimumSize() const;
    QSize sizeHint() const;
    void invalidate();

    bool hasHeightForWidth() const;
    int heightForWidth(int width) const;
    Qt::Orientations expandingDirections() const;
    int count() const;
    
    int rowCount() const;
};
#endif
