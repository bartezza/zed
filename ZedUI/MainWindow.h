#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef LINUX
#include <QMainWindow>
#include <QEvent>
#else
#include <QtWidgets/QMainWindow>
#include <QtCore/QEvent>
#endif
//#include "ConcurrentQueue.h"
#include "SimpleConcurrentQueue.h"

// prototypes
namespace Ui {
    class MainWindow;
}
class MainWindow;

//============================================================//

class GuiUpdater: public QObject {
    Q_OBJECT
protected:
    MainWindow *m_parent;

    QString m_tempLog;
    QString m_tempSerial;

    //moodycamel::ConcurrentQueue<QChar> m_logQueue;
    //moodycamel::ConcurrentQueue<QChar> m_serialQueue;
    SimpleConcurrentQueue<QChar> m_logQueue;
    SimpleConcurrentQueue<QChar> m_serialQueue;

public:
    GuiUpdater(MainWindow *parent): QObject(), m_parent(parent) {}

    void addLogText(const QString &text);
    void addSerialText(const QString &text);

public slots:
    void start();

private slots:
    void update();
};

//============================================================//

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void doRefreshRoms();

    inline const Ui::MainWindow *getUi() const { return ui; }
    inline Ui::MainWindow *getUi() { return ui; }

    void customEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);

    //void displayCpuState(BartBoy::GBCpuState *state);

    void addLogText(const QString &text) { m_guiUpdater.addLogText(text); }
    void addSerialText(const QString &text) { m_guiUpdater.addSerialText(text); }

    void addLogTextDirect(const QString &text);
    void addSerialTextDirect(const QString &text);

    void reqDisplayCpuState();

    void updateBreakpoints();

private slots:
    void on_btnRefresh_clicked();

    void on_btnLoad_clicked();
    void on_btnStep_clicked();

    void on_btnRun_clicked();
    void on_btnPause_clicked();
    
    void on_btnDisasm_clicked();

    //void on_MainWindow_destroyed();

    //void on_MainWindow_destroyed(QObject *arg1);

private:
    Ui::MainWindow *ui;

    GuiUpdater m_guiUpdater;
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
