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

#ifndef QFONTDIALOG_H
#define QFONTDIALOG_H

#include <QtGui/qwindowdefs.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_FONTDIALOG

#include <QtGui/qdialog.h>
#include <QtGui/qfont.h>

class QFontDialogPrivate;

class Q_GUI_EXPORT QFontDialog: public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QFontDialog)

public:
    // ### Qt5: merge overloads
    static QFont getFont(bool *ok, const QFont &def, QWidget* parent, const QString &caption);
    static QFont getFont(bool *ok, const QFont &def, QWidget* parent=0);
    static QFont getFont(bool *ok, QWidget* parent=0);

#ifdef QT3_SUPPORT
    static QFont getFont(bool *ok, const QFont &def, QWidget* parent, const char* name)
        { Q_UNUSED(name); return getFont(ok, def, parent); }
    static QFont getFont(bool *ok, QWidget* parent, const char* name)
        { Q_UNUSED(name); return getFont(ok, parent); }
#endif

private:
    static QFont getFont(bool *ok, const QFont *def, QWidget* parent=0);

    explicit QFontDialog(QWidget* parent=0, bool modal=false, Qt::WindowFlags f=0);
    ~QFontDialog();

    QFont font() const;
    void setFont(const QFont &font);

    bool eventFilter(QObject *, QEvent *);

    void updateFamilies();
    void updateStyles();
    void updateSizes();

private:
    Q_PRIVATE_SLOT(d_func(), void _q_sizeChanged(const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_familyHighlighted(int))
    Q_PRIVATE_SLOT(d_func(), void _q_writingSystemHighlighted(int))
    Q_PRIVATE_SLOT(d_func(), void _q_styleHighlighted(int))
    Q_PRIVATE_SLOT(d_func(), void _q_sizeHighlighted(int))
    Q_PRIVATE_SLOT(d_func(), void _q_updateSample())

    Q_DISABLE_COPY(QFontDialog)
};

#endif // QT_NO_FONTDIALOG

QT_END_HEADER

#endif // QFONTDIALOG_H
