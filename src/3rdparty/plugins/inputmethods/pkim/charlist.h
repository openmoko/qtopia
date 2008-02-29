
#ifndef CHARLIST_H
#define CHARLIST_H

#include <qwidget.h>
#include <qbitmap.h>

class QFontMetrics;

class CharList : public QWidget
{
    Q_OBJECT
public:
    CharList(QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
    ~CharList();

    void setMicroFocus( int x, int y );
    void setAppFont(const QFont &f) { appFont = f; }
    void setChars(const QStringList &ch);
    void setCurrent(const QString &ch);

protected:
    void paintEvent(QPaintEvent*);

private:
    int cellHeight;
    int cellWidth;
    QFont appFont;
    QFontMetrics *fm;
    QStringList chars;
    QString current;
};

#endif

