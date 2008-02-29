
#ifndef WPICKER_H
#define WPICKER_H

#include "picker.h"
#include <qstring.h>
#include <qstringlist.h>

class QTOPIA_INPUTMATCH_EXPORT WordPicker : public Picker
{
    Q_OBJECT
public:
    WordPicker(QWidget *parent=0);
    ~WordPicker();

    void setChoices(const QStringList &);

signals:
    void wordChosen(const QString &);

protected:
    void drawCell(QPainter *p, int, int, bool);

private slots:
    void sendWordChoice(int, int);

private:
    QStringList choices;
};

#endif

