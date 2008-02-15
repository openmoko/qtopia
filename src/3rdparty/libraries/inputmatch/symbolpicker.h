
#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "picker.h"
#include <qbitmap.h>

class QTOPIA_INPUTMATCH_EXPORT SymbolPicker : public Picker
{
    Q_OBJECT
public:
    SymbolPicker(QWidget *parent=0);
    ~SymbolPicker();

signals:
    void symbolClicked(int unicode, int keycode);

protected:
    void drawCell(QPainter *p, int, int, bool);

private slots:
    void sendSymbolForPos(int, int);

private:
    QString chars;
    int mapRows;
    bool havePress;
    QFont appFont;
    QBitmap crBitmap;
    QChar **symbols;
};

#endif

