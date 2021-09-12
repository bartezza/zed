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

public slots:
    void loadStory(const QByteArray &bytes);

    void zedDebugPrint(const char* str);
    void zedErrorPrint(const char* str);
    void zedGamePrint(const char* str);

protected:
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
    void on_btnRefresh_clicked();

    void on_btnLoad_clicked();
    void on_btnStep_clicked();

    void on_btnRun_clicked();
    void on_btnReset_clicked();
    
    void on_btnDisasm_clicked();

private:
    Ui::MainWindow *ui;
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
