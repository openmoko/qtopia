/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QITEMEDITORFACTORY_H
#define QITEMEDITORFACTORY_H

#include <QtCore/qbytearray.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_ITEMVIEWS

class QWidget;

class Q_GUI_EXPORT QItemEditorCreatorBase
{
public:
    virtual ~QItemEditorCreatorBase() {}

    virtual QWidget *createWidget(QWidget *parent) const = 0;
    virtual QByteArray valuePropertyName() const = 0;
};

template <class T>
class QItemEditorCreator : public QItemEditorCreatorBase
{
public:
    inline QItemEditorCreator(const QByteArray &valuePropertyName);
    inline QWidget *createWidget(QWidget *parent) const { return new T(parent); }
    inline QByteArray valuePropertyName() const { return propertyName; }

private:
    QByteArray propertyName;
};

template <class T>
class QStandardItemEditorCreator: public QItemEditorCreatorBase
{
public:
    inline QStandardItemEditorCreator()
        : propertyName(T::staticMetaObject.userProperty().name())
    {}
    inline QWidget *createWidget(QWidget *parent) const { return new T(parent); }
    inline QByteArray valuePropertyName() const { return propertyName; }

private:
    QByteArray propertyName;
};


template <class T>
Q_INLINE_TEMPLATE QItemEditorCreator<T>::QItemEditorCreator(const QByteArray &avaluePropertyName)
    : propertyName(avaluePropertyName) {}

class Q_GUI_EXPORT QItemEditorFactory
{
public:
    inline QItemEditorFactory() {}
    virtual ~QItemEditorFactory();

    virtual QWidget *createEditor(QVariant::Type type, QWidget *parent) const;
    virtual QByteArray valuePropertyName(QVariant::Type type) const;

    void registerEditor(QVariant::Type type, QItemEditorCreatorBase *creator);

    static const QItemEditorFactory *defaultFactory();
    static void setDefaultFactory(QItemEditorFactory *factory);

private:
    QHash<QVariant::Type, QItemEditorCreatorBase *> creatorMap;
};

#endif // QT_NO_ITEMVIEWS

QT_END_HEADER

#endif // QITEMEDITORFACTORY_H
