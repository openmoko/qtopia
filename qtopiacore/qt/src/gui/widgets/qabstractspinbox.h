/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QABSTRACTSPINBOX_H
#define QABSTRACTSPINBOX_H

#include <QtGui/qwidget.h>
#include <QtGui/qvalidator.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_SPINBOX

class QLineEdit;

class QAbstractSpinBoxPrivate;
class QStyleOptionSpinBox;

class Q_GUI_EXPORT QAbstractSpinBox : public QWidget
{
    Q_OBJECT

    Q_ENUMS(ButtonSymbols)
    Q_ENUMS(CorrectionMode)
    Q_PROPERTY(bool wrapping READ wrapping WRITE setWrapping)
    Q_PROPERTY(bool frame READ hasFrame WRITE setFrame)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(ButtonSymbols buttonSymbols READ buttonSymbols WRITE setButtonSymbols)
    Q_PROPERTY(QString specialValueText READ specialValueText WRITE setSpecialValueText)
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(bool accelerated READ isAccelerated WRITE setAccelerated)
    Q_PROPERTY(CorrectionMode correctionMode READ correctionMode WRITE setCorrectionMode)
    Q_PROPERTY(bool acceptableInput READ hasAcceptableInput)
    Q_PROPERTY(bool keyboardTracking READ keyboardTracking WRITE setKeyboardTracking)
public:
    explicit QAbstractSpinBox(QWidget *parent = 0);
    ~QAbstractSpinBox();

    enum StepEnabledFlag { StepNone = 0x00, StepUpEnabled = 0x01,
                           StepDownEnabled = 0x02 };
    Q_DECLARE_FLAGS(StepEnabled, StepEnabledFlag)

    enum ButtonSymbols { UpDownArrows, PlusMinus, NoButtons };

    ButtonSymbols buttonSymbols() const;
    void setButtonSymbols(ButtonSymbols bs);

    enum CorrectionMode  { CorrectToPreviousValue, CorrectToNearestValue };

    void setCorrectionMode(CorrectionMode cm);
    CorrectionMode correctionMode() const;

    bool hasAcceptableInput() const;
    QString text() const;

    QString specialValueText() const;
    void setSpecialValueText(const QString &txt);

    bool wrapping() const;
    void setWrapping(bool w);

    void setReadOnly(bool r);
    bool isReadOnly() const;

    void setKeyboardTracking(bool kt);
    bool keyboardTracking() const;

    void setAlignment(Qt::Alignment flag);
    Qt::Alignment alignment() const;

    void setFrame(bool);
    bool hasFrame() const;

    void setAccelerated(bool on);
    bool isAccelerated() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void interpretText();
    bool event(QEvent *event);

    virtual QValidator::State validate(QString &input, int &pos) const;
    virtual void fixup(QString &input) const;

    virtual void stepBy(int steps);
public Q_SLOTS:
    void stepUp();
    void stepDown();
    void selectAll();
    virtual void clear();
protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
    void hideEvent(QHideEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void timerEvent(QTimerEvent *event);
    void paintEvent(QPaintEvent *event);
    void showEvent(QShowEvent *event);
    void initStyleOption(QStyleOptionSpinBox *option) const;

    QLineEdit *lineEdit() const;
    void setLineEdit(QLineEdit *edit);

    virtual StepEnabled stepEnabled() const;
Q_SIGNALS:
    void editingFinished();
protected:
    QAbstractSpinBox(QAbstractSpinBoxPrivate &dd, QWidget *parent = 0);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_editorTextChanged(const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_editorCursorPositionChanged(int, int))

    Q_DECLARE_PRIVATE(QAbstractSpinBox)
    Q_DISABLE_COPY(QAbstractSpinBox)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractSpinBox::StepEnabled)

#endif // QT_NO_SPINBOX

QT_END_NAMESPACE

QT_END_HEADER

#endif // QABSTRACTSPINBOX_H
