

#include <qpe/taskbarappletinterface.h>

class CardMonitor;
class QPixmap;

class CardMonitorImpl : public TaskbarNamedAppletInterface
{
public:
    CardMonitorImpl();
    virtual ~CardMonitorImpl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT

    virtual QWidget *applet( QWidget *parent );
    virtual int position() const;
    virtual QString name() const;
    virtual QPixmap icon() const;

private:
    CardMonitor *cardMonitor;
    QPixmap *icn;
    ulong ref;
};
