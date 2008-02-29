/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef CONTEXTKEYMANAGER_H
#define CONTEXTKEYMANAGER_H

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

#include <qmap.h>
#include <qlist.h>

class ContextKeyManager : public QObject
{
    Q_OBJECT
public:
    ContextKeyManager();

    void updateContextLabels();
    QWidget *findTargetWidget(QWidget *w, int key, bool modal);
    bool haveLabelForWidget(QWidget *w, int key, bool modal);

    void setContextText(QWidget *w, int id, const QString &t, QSoftMenuBar::FocusState state);
    void setContextPixmap(QWidget *w, int id, const QString &p, QSoftMenuBar::FocusState state);
    void setContextStandardLabel(QWidget *w, int id, QSoftMenuBar::StandardLabel label, QSoftMenuBar::FocusState state);
    void clearContextLabel(QWidget *w, int key, QSoftMenuBar::FocusState state);
    void setClassText(const QString &className, int id, const QString &t, QSoftMenuBar::FocusState);
    void setClassPixmap(const QString &className, int id, const QString &p, QSoftMenuBar::FocusState);
    void setClassStandardLabel(const QString &className, int id, QSoftMenuBar::StandardLabel label, QSoftMenuBar::FocusState);

    struct ModalState {
        ModalState() : nmType(NoLabel), mType(NoLabel) {}

        enum Type { NoLabel, Custom, Standard };

        Type type(bool modal) const { return modal ? mType : nmType; }
        QString text(bool modal) const { return modal ? mText : nmText; }
        QString pixmap(bool modal) const { return modal ? mPixmap : nmPixmap; }
        QSoftMenuBar::StandardLabel label(bool modal) const { return modal ? mStandard : nmStandard; }

        QString nmText, mText;
        QString nmPixmap, mPixmap;
        QSoftMenuBar::StandardLabel nmStandard, mStandard;
        Type nmType, mType;
    };

    typedef QMap<int,ModalState> KeyMap;

    struct ClassModalState {
        ClassModalState(const QString &c)
            : className(c) {}
        ClassModalState() {}

        bool operator==(const ClassModalState &c) const {
            return className == c.className;
        }

        QString className;
        KeyMap keyMap;
    };

    void setText(QWidget *w, int key, const QString &text);
    void setPixmap(QWidget *w, int key, const QString &pm);
    void setStandard(QWidget *w, int key, QSoftMenuBar::StandardLabel label);
    void clearLabel(QWidget *w, int key);
    void setLabelType(QSoftMenuBar::LabelType);

    static ContextKeyManager *instance();
    static QString standardText(QSoftMenuBar::StandardLabel label);
    static QString standardPixmap(QSoftMenuBar::StandardLabel label);

    const QList<int> &keys() const { return buttons; }

private slots:
    void removeSenderFromWidgetContext();
    void updateLabelsForFocused();

private:
    bool updateContextLabel(QWidget *w, bool modal, int key);

private:
    QList<ClassModalState> contextClass;
    QMap<QWidget*,KeyMap> contextWidget;
    QList<int> buttons;
    QTimer *timer;
    QSoftMenuBar::LabelType labelType;
};

#endif

