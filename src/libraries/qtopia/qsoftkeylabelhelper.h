/****************************************************************************
  **
  ** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
  **
  ** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef QSOFTKEYLABELHELPER_H
#define QSOFTKEYLABELHELPER_H

#include <QObject>
#include <QWidget>
#include "qsoftkeylabelhelper_p.h"
#include <QByteArray>
#include <QLineEdit>
#include <QSoftMenuBar>
#include "contextkeymanager_p.h"

class QSoftKeyLabelHelperManager;
class QSoftKeyLabelHelperPrivate;

class QTOPIA_EXPORT QAbstractSoftKeyLabelHelper : public QObject
{
    Q_OBJECT
    friend class QSoftKeyLabelHelperPrivate;

public:
    QAbstractSoftKeyLabelHelper(QWidget* widget);
    QAbstractSoftKeyLabelHelper(const QString &className);
    virtual ~QAbstractSoftKeyLabelHelper();

    virtual bool focusIn(QWidget*) = 0;
    virtual bool focusOut(QWidget*);
    virtual bool enterEditFocus(QWidget*);
    virtual bool leaveEditFocus(QWidget*);

    virtual QString className() const;
    void setCurrentWidget(QWidget* widget);

public slots:
    virtual void updateAllLabels(QWidget* widget = 0) = 0;

protected:
    virtual void setClass(const QString& className);
    virtual void currentWidgetChangeNotification(QWidget* newWidget, QWidget* oldWidget);
    QWidget* currentWidget();

    // Label manipulation functions
    void setStandardLabel(int key,  QSoftMenuBar::StandardLabel);
    void setLabelText(int key, const QString &text);
    void setLabelPixmap(int key, const QString &pm);
    void clearLabel(int key);

private:
    QSoftKeyLabelHelperPrivate *d;
    QWidget* m_widget;
    QString m_className;
};

class QTOPIA_EXPORT QSoftKeyLabelHelper : public QAbstractSoftKeyLabelHelper
{
    Q_OBJECT
public:
    QSoftKeyLabelHelper(QWidget* widget);
    QSoftKeyLabelHelper(const QString &className);
    virtual ~QSoftKeyLabelHelper();
    virtual bool focusIn(QWidget*);
    virtual bool focusOut(QWidget*);
    virtual bool enterEditFocus(QWidget*);
    virtual bool leaveEditFocus(QWidget*);
    virtual void updateAllLabels(QWidget *widget);
private:
    QWidget* m_widget;
};

class QTOPIA_EXPORT QLineEditSoftKeyLabelHelper : public QAbstractSoftKeyLabelHelper
{
    Q_OBJECT
public:
    QLineEditSoftKeyLabelHelper(QString className="QLineEdit");
    QLineEditSoftKeyLabelHelper(QLineEdit* l);
    ~QLineEditSoftKeyLabelHelper();

    virtual bool focusIn(QWidget*);
    virtual bool focusOut(QWidget*);
    virtual bool enterEditFocus(QWidget*);
    virtual bool leaveEditFocus(QWidget*);

public slots:
    virtual void updateAllLabels(QWidget* widget=0);
    virtual void textChanged(QString newText);
    virtual void cursorPositionChanged(int old, int current);
protected:
    virtual void currentWidgetChangeNotification(QWidget* newWidget, QWidget* oldWidget);
    virtual bool eventFilter(QObject* watched, QEvent *event);
    QLineEdit* m_l;
private:
    bool m_preeditTextFlag;
};
#endif // QSOFTKEYLABELHELPER_H
