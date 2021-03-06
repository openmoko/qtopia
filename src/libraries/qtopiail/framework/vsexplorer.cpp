/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include <QApplication>
#include <QObject>
#include <QTextStream>
#include <QFile>
#include <QSocketNotifier>
#include <cstdio>
#include <QMap>
#include <QDir>
#include <QStringList>
#include <QSet>

#include "qvaluespace.h"

#define USE_READLINE
#ifdef USE_READLINE
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <QThread>
#include <QMutex>
#include <QEvent>
#include <QWaitCondition>
#include <pthread.h>
#endif

static bool terminateRequested = false;

class VSExplorer : public QObject
{
Q_OBJECT
public:
    VSExplorer();

    void printError();
    void printHelp();
    void ls();
    void ls(const QByteArray &abs);
    void subscribe();
    void unsubscribe();
    void quit();
    void suppress();
    void listwatchers();
    void watch(const QByteArray &);
    void unwatch(const QByteArray &);
    void write(const QByteArray &, const QString &);
    void set(const QByteArray &, const QString &);
    void remove(const QByteArray &);
    void clear(const QByteArray &);
    void subscriptions();

    QByteArray itemName() const
    {
        return pwd.itemName().toAscii();
    }

public slots:
    void processLine(const QString &);

private slots:
    void contentsChanged();
    void removed(const QByteArray &attribute);
    void written(const QByteArray &attribute, const QVariant &newData);

private:
    void lsPath(QValueSpaceItem *, int = 0);

    bool isSuppress;
    QValueSpaceItem pwd;
    QValueSpaceObject prov;
    QSet<QValueSpaceItem *> subs;
    QSet<QValueSpaceObject *> watchers;
};
static VSExplorer * vse = 0;

class LineInput :
#ifdef USE_READLINE
    public QThread
#else
    public QObject
#endif
{
Q_OBJECT
public:
    LineInput();

signals:
    void line(const QString &);

#ifndef USE_READLINE
private slots:
    void readyRead();

private:
    QFile ts;
    QSocketNotifier * sock;

#else
protected:
    virtual bool event(QEvent *);
    virtual void run();

private:
    QMutex mutex;
    QWaitCondition wait;
#endif
};

VSExplorer::VSExplorer()
: isSuppress(false), pwd("/"), prov("/")
{
}

void VSExplorer::removed(const QByteArray &attribute)
{
    Q_ASSERT(sender());

    if(!isSuppress) {
        QValueSpaceObject * obj = static_cast<QValueSpaceObject *>(sender());
        fprintf(stdout, "\nRemoved: %s ... %s\n",
                obj->objectPath().toAscii().constData(), attribute.constData());
    }
}

void VSExplorer::written(const QByteArray &attribute, const QVariant &newData)
{
    Q_ASSERT(sender());
    if(!isSuppress) {
        QValueSpaceObject * obj = static_cast<QValueSpaceObject *>(sender());
        fprintf(stdout, "\nWritten: %s ... %s to '%s' (%s)\n",
                obj->objectPath().toAscii().constData(),
                attribute.constData(), newData.toString().toAscii().constData(),
                newData.typeName());
    }
}

void VSExplorer::contentsChanged()
{
    Q_ASSERT(sender());

    if(!isSuppress) {
        QValueSpaceItem * item = static_cast<QValueSpaceItem *>(sender());
        fprintf(stdout, "\nChanged: %s\n", item->itemName().constData());
    }
}

void VSExplorer::printError()
{
    fprintf(stdout, "Come again?\n");
    fflush(stdout);
}

void VSExplorer::printHelp()
{
    fprintf(stdout, "help/?: Print this message\n");
    fprintf(stdout, "quit: Exit VSExplorer\n");
    fprintf(stdout, "pwd: Print current working directory\n");
    fprintf(stdout, "ls: List contents of path\n");
    fprintf(stdout, "subscribe: Subscribe to path\n");
    fprintf(stdout, "unsubscribe: Unsubscribe from path\n");
    fprintf(stdout, "suppress: Toggle suppression of publish messages\n");
    fprintf(stdout, "subscriptions: List current subscriptions\n");
    fprintf(stdout, "set <key> <value>: Set app layer<key> to <value>\n");
    fprintf(stdout, "write <key> <value>: Set <key> to <value>\n");
    fprintf(stdout, "remove <key>: Clear <key>\n");
    fprintf(stdout, "clear <key>: Clear app layer <key>\n");
    fprintf(stdout, "cd <path>: Change working path\n");
    fprintf(stdout, "watch <path>: Add a watch for the path\n");
    fprintf(stdout, "unwatch <path>: Remove a watch for the path\n");
    fprintf(stdout, "watchers: List paths for which a watch is active\n");
    fflush(stdout);
}

void VSExplorer::ls()
{
    lsPath(&pwd);
    fflush(stdout);
}

void VSExplorer::ls(const QByteArray &abs)
{
    QValueSpaceItem item(abs);
    lsPath(&item);
    fflush(stdout);
}


void VSExplorer::lsPath(QValueSpaceItem * p, int indent)
{
    QList<QString> paths = p->subPaths();

    QVariant var = p->value();
    bool spaceRequired = false;
    if(!var.isNull()) {
        fprintf(stdout, "Value: '%s' (%s)\n",
                var.toString().toAscii().constData(), var.typeName());
        spaceRequired = true;
    }

    foreach(QString path, paths) {
        if(spaceRequired) {
            fprintf(stdout, "\n");
            spaceRequired = false;
        }
        for(int ii = 0; ii < indent; ++ii) fprintf(stdout, "\t");
        fprintf(stdout, "%s/\n", path.toAscii().constData());
    }
}

void VSExplorer::listwatchers()
{
    if(watchers.isEmpty()) {
        fprintf(stdout, "No watchers.\n");
    } else {
        fprintf(stdout, "Current watchers:\n");
        foreach(QValueSpaceObject *obj, watchers)
            fprintf(stdout, "\t%s\n", obj->objectPath().toAscii().constData());
    }

    fflush(stdout);
}

void VSExplorer::subscriptions()
{
    if(subs.isEmpty()) {
        fprintf(stdout, "No subscriptions.\n");
    } else {
        fprintf(stdout, "Current subscriptions:\n");

        foreach(QValueSpaceItem *item, subs)
            fprintf(stdout, "\t%s\n", item->itemName().constData());
    }

    fflush(stdout);
}

void VSExplorer::subscribe()
{
    QValueSpaceItem *item = new QValueSpaceItem(pwd);
    QObject::connect(item, SIGNAL(contentsChanged()),
                     this, SLOT(contentsChanged()));
    subs.insert(item);

    fprintf(stdout, "OK\n");
    fflush(stdout);
}

void VSExplorer::unsubscribe()
{
    foreach(QValueSpaceItem *item, subs) {
        if(item->itemName() == pwd.itemName()) {
            subs.remove(item);
            delete item;
            fprintf(stdout, "OK\n");
            fflush(stdout);
            return;
        }
    }

    fprintf(stdout, "No subscription.\n");
    fflush(stdout);
}

void VSExplorer::quit()
{
    fprintf(stdout, "Bye, bye.\n");
    fflush(stdout);
    terminateRequested = true;
}

void VSExplorer::watch(const QByteArray &path)
{
    foreach(QValueSpaceObject *obj, watchers) {
        if(obj->objectPath().toUtf8() == path)
            return;
    }
    QValueSpaceObject * newObject = new QValueSpaceObject(path);
    watchers.insert(newObject);
    QObject::connect(newObject, SIGNAL(itemRemove(QByteArray)),
                     this, SLOT(removed(QByteArray)));
    QObject::connect(newObject, SIGNAL(itemSetValue(QByteArray,QVariant)),
                     this, SLOT(written(QByteArray,QVariant)));
}

void VSExplorer::unwatch(const QByteArray &path)
{
    foreach(QValueSpaceObject *obj, watchers) {
        if(obj->objectPath().toUtf8() == path) {
            watchers.remove(obj);
            delete obj;
            return;
        }
    }
}

void VSExplorer::suppress()
{
    if(isSuppress) {
        isSuppress = false;
        fprintf(stdout, "Suppression off.\n");
    } else {
        isSuppress = true;
        fprintf(stdout, "Suppression on.\n");
    }
    fflush(stdout);
}

void VSExplorer::write(const QByteArray &name, const QString &value)
{
    pwd.setValue(name, value);
    pwd.sync();
}

void VSExplorer::set(const QByteArray &name, const QString &value)
{
    if('/' == *name.constData())
        prov.setAttribute(name, value);
    else if(pwd.itemName().endsWith("/"))
        prov.setAttribute(pwd.itemName() + name, value);
    else
        prov.setAttribute(pwd.itemName() + "/" + name, value);
}

void VSExplorer::remove(const QByteArray &name)
{
    pwd.remove(name);
    pwd.sync();
}

void VSExplorer::clear(const QByteArray &name)
{
    if('/' == *name.constData())
        prov.removeAttribute(name);
    else if(pwd.itemName().endsWith("/"))
        prov.removeAttribute(pwd.itemName() + name);
    else
        prov.removeAttribute(pwd.itemName() + "/" + name);
}

void VSExplorer::processLine(const QString &line)
{
    QStringList cmds = line.trimmed().split(' ');

    if(cmds.isEmpty()) {
        return;
    }

    const QString & cmd = cmds.at(0);
    if(cmd == "ls" && 1 == cmds.count()) {
        ls();
    } else if(cmd == "ls" && 2 <= cmds.count()) {
        QStringList newCmds = cmds;
        newCmds.removeFirst();
        QString newPath = newCmds.join(" ");
        if(newPath.startsWith("\"") && newPath.endsWith("\"")) {
            newPath = newPath.mid(1);
            newPath = newPath.left(newPath.length() - 1);
        }
        if(newPath.startsWith("/")) {
            ls(newPath.toAscii());
        } else {
            QString oldPath = pwd.itemName();
            if(!oldPath.endsWith("/"))
                oldPath.append("/");
            oldPath.append(newPath);
            oldPath = QDir::cleanPath(oldPath);
            ls(oldPath.toAscii());
        }

    } else if(cmd == "cd" && 2 <= cmds.count()) {
        QStringList newCmds = cmds;
        newCmds.removeFirst();
        QString newPath = newCmds.join(" ");
        if(newPath.startsWith("\"") && newPath.endsWith("\"")) {
            newPath = newPath.mid(1);
            newPath = newPath.left(newPath.length() - 1);
        }
        if(newPath.startsWith("/")) {
            pwd = QValueSpaceItem(newPath.toAscii());
        } else {
            QString oldPath = pwd.itemName();
            if(!oldPath.endsWith("/"))
                oldPath.append("/");
            oldPath.append(newPath);
            oldPath = QDir::cleanPath(oldPath);
            pwd = QValueSpaceItem(oldPath.toAscii());
        }
    } else if(cmd == "unwatch" && 2 <= cmds.count()) {
        QStringList newCmds = cmds;
        newCmds.removeFirst();
        QString newPath = newCmds.join(" ");
        QString finalPath;
        if(newPath.startsWith("\"") && newPath.endsWith("\"")) {
            newPath = newPath.mid(1);
            newPath = newPath.left(newPath.length() - 1);
        }
        if(newPath.startsWith("/")) {
            finalPath = QValueSpaceItem(newPath.toAscii()).itemName();
        } else {
            QString oldPath = pwd.itemName();
            if(!oldPath.endsWith("/"))
                oldPath.append("/");
            oldPath.append(newPath);
            oldPath = QDir::cleanPath(oldPath);
            finalPath = QValueSpaceItem(oldPath.toAscii()).itemName();
        }
        unwatch(finalPath.toUtf8());
    } else if(cmd == "watch" && 2 <= cmds.count()) {
        QStringList newCmds = cmds;
        newCmds.removeFirst();
        QString newPath = newCmds.join(" ");
        QString finalPath;
        if(newPath.startsWith("\"") && newPath.endsWith("\"")) {
            newPath = newPath.mid(1);
            newPath = newPath.left(newPath.length() - 1);
        }
        if(newPath.startsWith("/")) {
            finalPath = QValueSpaceItem(newPath.toAscii()).itemName();
        } else {
            QString oldPath = pwd.itemName();
            if(!oldPath.endsWith("/"))
                oldPath.append("/");
            oldPath.append(newPath);
            oldPath = QDir::cleanPath(oldPath);
            finalPath = QValueSpaceItem(oldPath.toAscii()).itemName();
        }
        watch(finalPath.toUtf8());
    } else if(cmd == "write" && 3 == cmds.count()) {
        write(cmds.at(1).trimmed().toAscii(), cmds.at(2).trimmed());
    } else if(cmd == "set" && 3 == cmds.count()) {
        set(cmds.at(1).trimmed().toAscii(), cmds.at(2).trimmed());
    } else if(cmd == "clear" && 2 == cmds.count()) {
        clear(cmds.at(1).trimmed().toAscii());
    } else if(cmd == "remove" && 2 == cmds.count()) {
        remove(cmds.at(1).trimmed().toAscii());
    } else if((cmd == "subscribe" || cmd == "sub") && 1 == cmds.count()) {
        subscribe();
    } else if((cmd == "unsubscribe" || cmd == "unsub") && 1 == cmds.count()) {
        unsubscribe();
    } else if((cmd == "?" || cmd == "help") && 1 == cmds.count()) {
        printHelp();
    } else if((cmd == "quit" || cmd == "exit") && 1 == cmds.count()) {
        quit();
    } else if((cmd == "suppress") && 1 == cmds.count()) {
        suppress();
    } else if((cmd == "watchers") && 1 == cmds.count()) {
        listwatchers();
    } else if((cmd == "subscriptions") && 1 == cmds.count()) {
        subscriptions();
    } else if(cmd == "") {
    } else {
        printError();
    }
}

#ifdef USE_READLINE
extern "C" {
    char * item_generator(const char * text, int num);
    char * command_generator(const char * text, int num);
    char ** item_completion(const char * text, int start, int end);
}
#endif

LineInput::LineInput()
{
#ifndef USE_READLINE
    ts.open(stdin, QIODevice::ReadOnly);
    sock = new QSocketNotifier(ts.handle(), QSocketNotifier::Read, this);
    QObject::connect(sock, SIGNAL(activated(int)), this, SLOT(readyRead()));

    fprintf(stdout, "%s > ", vse->itemName().constData());
    fflush(stdout);
#else
    rl_completion_append_character = '\0';
    rl_attempted_completion_function = item_completion;
    rl_completer_quote_characters = "\"";
    rl_basic_word_break_characters = " \t\n\"\\'`@$><=;|&(";
    rl_filename_quote_characters = " ";
    QObject::connect(this, SIGNAL(finished()), qApp, SLOT(quit()));
    QObject::connect(this, SIGNAL(terminated()), qApp, SLOT(quit()));
    start();
#endif
}

#ifndef USE_READLINE
void LineInput::readyRead()
{
    QByteArray line = ts.readLine();

    emit this->line(line);

    if(terminateRequested)
        exit(0);

    fprintf(stdout, "%s > ", vse->itemName().constData());
    fflush(stdout);
}
#endif

#ifdef USE_READLINE
#define TEXTEVENTTYPE (QEvent::User + 10)
struct TextEvent : public QEvent
{
    TextEvent(char *line)
        : QEvent((QEvent::Type)TEXTEVENTTYPE), data(line) {}

    char * data;
};

bool LineInput::event(QEvent *e)
{
    if(e->type() == TEXTEVENTTYPE) {
        TextEvent * te = static_cast<TextEvent *>(e);
        emit line(te->data);
        free(te->data);

        mutex.lock();
        wait.wakeAll();
        mutex.unlock();

        return true;
    } else {
        return QThread::event(e);
    }
}

char ** item_completion(const char * text, int start, int end)
{
    char ** matches = (char **)NULL;
    const char * non_space = text;
    while(*non_space == ' ') ++non_space;


    if(start == non_space - text)
        matches = rl_completion_matches(text, command_generator);
    else
        matches = rl_completion_matches(text, item_generator);

    rl_attempted_completion_over = 1;
    return (matches);
}

char * command_generator(const char * t, int num)
{
    static QList<QByteArray> children;

    if(0 == num) {
        children.clear();

        // Command
        static char * commands[] = { "help ",
                                     "quit ",
                                     "pwd ",
                                     "ls ",
                                     "subscribe ",
                                     "unsubscribe ",
                                     "suppress ",
                                     "subscriptions ",
                                     "set ",
                                     "clear ",
                                     "cd " };

        for(int ii = 0; ii < sizeof(commands) / sizeof(char *); ++ii)
            if(0 == ::strncmp(commands[ii], t, strlen(t)))
                children.append(commands[ii]);
    }

    if(children.isEmpty())
        return 0;

    char * rv = (char *)malloc(children.at(0).length() + 1);
    ::memcpy(rv, children.at(0).constData(), children.at(0).length() + 1);
    children.removeFirst();

    return rv;
}

char * item_generator(const char * t, int num)
{
    static QList<QByteArray> children;

    rl_filename_completion_desired = 1;
    rl_filename_quoting_desired = 1;
    if(0 == num) {

        children.clear();

        // Path
        QByteArray text = t;
        QByteArray textExt;
        QByteArray textBase;

        int last = text.lastIndexOf('/');
        if(-1 == last) {
            textExt = text;
        } else {
            textBase = text.left(last);
            textExt = text.mid(last + 1);
        }

        QByteArray vsBase;

        if(*textBase.constData() != '/') {
            QByteArray in = vse->itemName();
            if(!in.endsWith("/"))
                vsBase = in + "/" + textBase;
            else
                vsBase = in + textBase;
        } else {
            vsBase = textBase;
        }

        QValueSpaceItem item(vsBase);

        QList<QString> schildren = item.subPaths();

        foreach(QString child, schildren) {
            if(child.startsWith(textExt)) {
                QByteArray completion;
                completion.append(textBase);
                if(!completion.isEmpty())
                    completion.append("/");
                completion.append(child.toAscii());
                completion.append("/");
                children.append(completion);
            }
        }
    }

    if(children.isEmpty())
        return 0;

    char * rv = (char *)malloc(children.at(0).length() + 1);
    ::memcpy(rv, children.at(0).constData(), children.at(0).length() + 1);
    children.removeFirst();

    return rv;
}


void LineInput::run()
{
    while(true) {
        /* Get a line from the user. */
        mutex.lock();
        QByteArray prompt = vse->itemName();
        prompt.append(" > ");
        mutex.unlock();
        char *line_read = readline (prompt.constData());

        /* If the line has any text in it,
           save it on the history. */
        if (line_read && *line_read)
            add_history (line_read);

        mutex.lock();
        TextEvent * e = new TextEvent(line_read);
        QApplication::postEvent(this, e);
        wait.wait(&mutex);
        if(terminateRequested) {
            mutex.unlock();
            return;
        } else {
            mutex.unlock();
        }
    }
}
#endif

void usage(char * app)
{
    fprintf(stderr, "Usage: %s [-s] [-d]\n", app);
    exit(-1);
}

void dodump(QValueSpaceItem * item)
{
    QList<QString> children = item->subPaths();
    foreach(QString child, children) {
        QValueSpaceItem subitem(*item, child);
        dodump(&subitem);
    }
    QVariant var = item->value();
    fprintf(stdout, "%s '%s' %s\n",
            item->itemName().constData(),
            var.toString().toAscii().constData(),
            var.typeName());
}


int main(int argc, char ** argv)
{
    QCoreApplication app(argc, argv);

    bool manager = false;
    bool dump = false;
    for(int ii = 1; ii < argc; ++ii) {
        if(0 == ::strcmp(argv[ii], "-s"))
            manager = true;
        else if(0 == ::strcmp(argv[ii], "-d"))
            dump = true;
        else
            usage(argv[0]);
    }

    if(manager)
        QValueSpace::initValuespaceManager();

    if(dump) {
        QValueSpaceItem item("/");
        dodump(&item);
        return 0;
    } else {
        vse = new VSExplorer;
        LineInput li;


        QObject::connect(&li, SIGNAL(line(QString)),
                         vse, SLOT(processLine(QString)));

        int rv = app.exec();
        delete vse;
        return rv;
    }
}

#include "vsexplorer.moc"
