#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef LINUX
#include <QMainWindow>
#include <QEvent>
#else
#include <QtWidgets/QMainWindow>
#include <QtCore/QEvent>
#endif
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTextStream>
#include <QStringList>
#include <QQueue>
#include "../Zed/Zed.h"

// prototypes
namespace Ui {
    class MainWindow;
}
class MainWindow;

//============================================================//

class ZedThread : public QThread {
    Q_OBJECT
public:
    ZedThread(MainWindow* mainWindow);
    ~ZedThread();

    Zed m_zed;

    void stepStory();
    void continueStory();
    void resetStory();

public:
    bool loadStory(const QByteArray &bytes);

signals:
    void zedDebugPrint(const QString &str);
    void zedErrorPrint(const QString& str);
    void zedGamePrint(const QString& str);
    void zedAddDisasm(const QString& str);

protected:
    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_singleStep = false;
    bool m_reset = false;
    bool m_abort = false;
    QMutex m_runMutex;

    void run() override;
};

//============================================================//

class MainWindow: public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    inline const Ui::MainWindow *getUi() const { return ui; }
    inline Ui::MainWindow *getUi() { return ui; }

    void customEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);

    ZedThread m_zedThread;

private slots:
    void timerUpdateState();

    void zedDebugPrint(const QString& str);
    void zedErrorPrint(const QString& str);
    void zedGamePrint(const QString& str);
    void zedAddDisasm(const QString& str);

    void on_actionLoadStory_triggered();
    void on_btnRun_clicked();
    void on_btnStep_clicked();
    void on_btnReset_clicked();

private:
    Ui::MainWindow *ui;
    
    // using double buffering here
    QTextStream m_gameText[2];
    QTextStream m_debugText[2];
    QString m_gameStr[2];
    QString m_debugStr[2];
    int m_curBuffer = 0;
    QMutex m_bufferMutex;
    
    // QStringList m_disasmQueue[2];

    QMutex m_disasmMutex;
    QQueue<QString> m_disasmQueue;
};

//============================================================//

template <unsigned int id>
class StringEvent : public QEvent
{
    QString m_str;
public:
    explicit StringEvent(const QString val) : QEvent(staticType()), m_str(val)
    {
    }

    void setvalue(QString val)
    {
        m_str = val;
    }

    QString value() const
    {
        return m_str;
    }

    static QEvent::Type staticType()
    {
        static int type = QEvent::registerEventType();
        return static_cast<QEvent::Type>(type);

    }

    static bool is(const QEvent * ev)
    {
        return ev->type() == staticType();
    }
};

//============================================================//

template <unsigned int id>
class StaticEvent : public QEvent
{
public:
    explicit StaticEvent() : QEvent(staticType())
    {
    }

    static QEvent::Type staticType()
    {
        static int type = QEvent::registerEventType();
        return static_cast<QEvent::Type>(type);

    }

    static bool is(const QEvent * ev)
    {
        return ev->type() == staticType();
    }
};

#endif // MAINWINDOW_H
