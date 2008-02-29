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

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <QWidget>
#include <QTimeLine>
class PopupWindow;
class OptionsWindow;
class Board;
class WordPredict;
class AcceptWindow;

class KeyboardWidget : public QWidget
{
Q_OBJECT
public:
    enum BoardType { NonAlphabet, Numeric, UpperCase, LowerCase }; 
    static void instantiatePopupScreen();

    struct Config
    {
        int minimumStrokeMotionPerPeriod;
        int strokeMotionPeriod;
        int maximumClickStutter;
        int maximumClickTime;
        qreal minimumStrokeLength;
        qreal minimumStrokeDirectionRatio;

        QSize keyAreaSize;

        int selectCircleDiameter;
        int selectCircleOffset;

        int boardChangeTime;

        int leftSquishPoint;
        qreal leftSquishScale;
        int rightSquishPoint;
        qreal rightSquishScale;

        QSize keySize;
        int keyMargin;
        int bottomMargin;

        QString trieFile;
        int maxGuesses;

        int optionWordSpacing;
        int optionsWindowHeight;

        int reallyNoMoveSensitivity;
        int moveSensitivity;
        int excludeDistance;
    };

    KeyboardWidget(const Config &, QWidget *parent = 0);
    virtual ~KeyboardWidget();

    void setSelectionHeight(int);

    void addBoard(const QStringList &, BoardType);

    void autoCapitalizeNextWord(bool);

    virtual QSize sizeHint() const;
    bool hasText();
    void reset();

    void setAcceptDest(const QPoint &);
    virtual bool filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);

signals:
    void backspace();
    void preedit(const QString &);
    void commit(const QString &);

protected:
    virtual void paintEvent(QPaintEvent *);

    virtual void resizeEvent(QResizeEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void moveEvent(QMoveEvent *);

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    virtual void timerEvent(QTimerEvent *);

public slots:
    void acceptWord();
    virtual void setHint(const QString& hint);
    void doBackspace();
    QRect rectForCharacter(const QChar &) const;
    QRect rectForWord(const QString &);

private:
    Config m_config;

    void clear();
    void resetToHistory();
    QStringList m_words;

    void dumpPossibleMotion();

    void startMouseTimer();
    void speedMouseTimer();
    void stopMouseTimer();
    int m_mouseTimer;
    bool m_speedMouseTimer;

    void mouseClick(const QPoint &);
    enum Stroke { NoStroke, StrokeLeft, StrokeRight, StrokeUp, StrokeDown };
    void stroke(Stroke);
    void pressAndHold();
    void pressAndHoldChar(const QChar &);

    QChar closestCharacter(const QPoint &, Board * = 0) const;

    QList<Board *> m_boards;
    int m_currentBoard;
    QSize m_boardSize;
    QRect m_boardRect;
    QPoint toBoardPoint(const QPoint &) const;

    enum Motion { Left = 0x01, Right = 0x02, Up = 0x04, Down = 0x08 };
    Motion m_possibleMotion;
    QPoint m_mouseMovePoint;
    QPoint m_lastSamplePoint;
    QPoint m_mousePressPoint;
    bool m_mouseClick;
    bool m_pressAndHold;
    QChar m_pressAndHoldChar;
    bool m_animate_accept;

    QPoint windowPosForChar() const;
    PopupWindow *m_charWindow;

    void setBoardByType(BoardType newBoard);
    QTimeLine m_boardChangeTimeline;
    int m_oldBoard;
    bool m_boardUp;

    bool m_specialDelete;

    bool m_ignoreMouse;
    bool m_ignore;

    void positionOptionsWindow();
    OptionsWindow *m_options;

    bool m_notWord;
    bool m_alphabetSet;
    WordPredict *m_predict;
    void updateWords();
    QString closestWord();
    bool m_autoCap;
    bool m_autoCapitaliseEveryWord;
    bool m_preeditSpace;
    bool m_dontAddPreeditSpace;

    QStringList fixupCase(const QStringList &) const;

    struct KeyOccurance {
        enum Type { MousePress, CharSelect };
        Type type;
        QPoint widPoint;
        QChar explicitChar;
        int board;

        QString freezeWord;
    };
    QList<KeyOccurance> m_occuranceHistory;
};

#endif // _KEYBOARD_H_

