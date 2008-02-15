
#ifndef MPICKER_H
#define MPICKER_H

#include <qtopia/inputmatch/picker.h>
#include <qstringlist.h>

class InputMatcherSet;

class ModePicker : public Picker
{
    Q_OBJECT
public:
    ModePicker(InputMatcherSet *, QWidget *parent=0);
    ~ModePicker();

signals:
    void modeSelected(const QString &, bool);

protected slots:
    void setModeFor(int, int);

protected:
    void showEvent(QShowEvent *ev);
    void drawCell(QPainter *p, int, int, bool);

private:
    void updateModeList();

    QStringList list;
    InputMatcherSet *set;
};

#endif

