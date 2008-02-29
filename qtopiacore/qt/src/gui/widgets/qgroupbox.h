/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGROUPBOX_H
#define QGROUPBOX_H

#include <QtGui/qframe.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_GROUPBOX

class QGroupBoxPrivate;
class Q_GUI_EXPORT QGroupBox : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool flat READ isFlat WRITE setFlat)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY toggled USER true)
public:
    explicit QGroupBox(QWidget* parent=0);
    explicit QGroupBox(const QString &title, QWidget* parent=0);
    ~QGroupBox();

    QString title() const;
    void setTitle(const QString &title);

    Qt::Alignment alignment() const;
    void setAlignment(int alignment);

    QSize minimumSizeHint() const;

    bool isFlat() const;
    void setFlat(bool flat);
    bool isCheckable() const;
    void setCheckable(bool checkable);
    bool isChecked() const;

public Q_SLOTS:
    void setChecked(bool checked);

Q_SIGNALS:
    void clicked(bool checked = false);
    void toggled(bool);

protected:
    bool event(QEvent *event);
    void childEvent(QChildEvent *event);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void focusInEvent(QFocusEvent *event);
    void changeEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);  

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QGroupBox(QWidget* parent, const char* name);
    QT3_SUPPORT_CONSTRUCTOR QGroupBox(const QString &title, QWidget* parent, const char* name);
#endif

private:
    Q_DISABLE_COPY(QGroupBox)
    Q_DECLARE_PRIVATE(QGroupBox)
    Q_PRIVATE_SLOT(d_func(), void _q_setChildrenEnabled(bool b))
};

#endif // QT_NO_GROUPBOX

QT_END_HEADER

#endif // QGROUPBOX_H
