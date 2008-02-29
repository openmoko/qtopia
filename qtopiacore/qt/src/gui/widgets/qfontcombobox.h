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

#ifndef QFONTCOMBOBOX_H
#define QFONTCOMBOBOX_H

#include <QtGui/qcombobox.h>
#include <QtGui/qfontdatabase.h>

#ifndef QT_NO_FONTCOMBOBOX

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QFontComboBoxPrivate;

class Q_GUI_EXPORT QFontComboBox : public QComboBox
{
    Q_OBJECT
    Q_FLAGS(FontFilters)
    Q_PROPERTY(QFontDatabase::WritingSystem writingSystem READ writingSystem WRITE setWritingSystem)
    Q_PROPERTY(FontFilters fontFilters READ fontFilters WRITE setFontFilters)
    Q_PROPERTY(QFont currentFont READ currentFont WRITE setCurrentFont NOTIFY currentFontChanged)
    Q_ENUMS(FontSelection)

public:
    explicit QFontComboBox(QWidget *parent = 0);
    ~QFontComboBox();

    void setWritingSystem(QFontDatabase::WritingSystem);
    QFontDatabase::WritingSystem writingSystem() const;

    enum FontFilter {
        AllFonts = 0,
        ScalableFonts = 0x1,
        NonScalableFonts = 0x2,
        MonospacedFonts = 0x4,
        ProportionalFonts = 0x8
    };
    Q_DECLARE_FLAGS(FontFilters, FontFilter)

    void setFontFilters(FontFilters filters);
    FontFilters fontFilters() const;

    QFont currentFont() const;
    QSize sizeHint() const;

public Q_SLOTS:
    void setCurrentFont(const QFont &f);

Q_SIGNALS:
    void currentFontChanged(const QFont &f);

protected:
    bool event(QEvent *e);

private:
    Q_DISABLE_COPY(QFontComboBox)
    Q_DECLARE_PRIVATE(QFontComboBox)
    Q_PRIVATE_SLOT(d_func(), void _q_currentChanged(const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_updateModel())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFontComboBox::FontFilters)

QT_END_HEADER

#endif // QT_NO_FONTCOMBOBOX
#endif
