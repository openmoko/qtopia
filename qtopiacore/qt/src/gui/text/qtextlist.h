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

#ifndef QTEXTLIST_H
#define QTEXTLIST_H

#include <QtGui/qtextobject.h>
#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QTextListPrivate;
class QTextCursor;

class Q_GUI_EXPORT QTextList : public QTextBlockGroup
{
    Q_OBJECT
public:
    explicit QTextList(QTextDocument *doc);
    ~QTextList();

    int count() const;

    inline bool isEmpty() const
    { return count() == 0; }

    QTextBlock item(int i) const;

    int itemNumber(const QTextBlock &) const;
    QString itemText(const QTextBlock &) const;

    void removeItem(int i);
    void remove(const QTextBlock &);

    void add(const QTextBlock &block);

    inline void setFormat(const QTextListFormat &format);
    QTextListFormat format() const { return QTextObject::format().toListFormat(); }

private:
    Q_DISABLE_COPY(QTextList)
    Q_DECLARE_PRIVATE(QTextList)
};

inline void QTextList::setFormat(const QTextListFormat &aformat)
{ QTextObject::setFormat(aformat); }

QT_END_HEADER

#endif // QTEXTLIST_H
