
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QString>
#include <QFile>
#include <QFileDialog>

//QTimer::singleShot(0, this, SLOT(update())); 
// QTimer::singleShot(10, this, SLOT(update()));

//============================================================//

#define STRING_EVENT_LOG                    0
#define STRING_EVENT_SERIAL_OUT             1

#define STATIC_EVENT_DISPLAY_CPU_STATE      0

//============================================================//

ZedThread::ZedThread(MainWindow* mainWindow) :
    QThread(mainWindow)
{
    m_zed.debugPrintCallback = std::bind(&ZedThread::zedDebugPrint, this, std::placeholders::_1);
    m_zed.errorPrintCallback = std::bind(&ZedThread::zedErrorPrint, this, std::placeholders::_1);
    m_zed.gamePrintCallback = std::bind(&ZedThread::zedGamePrint, this, std::placeholders::_1);
}

ZedThread::~ZedThread() {
    // set abort
    m_mutex.lock();
    m_abort = true;
    // wake if thread is sleeping
    m_condition.wakeOne();
    m_mutex.unlock();
    // wait for thread
    wait();
}

void ZedThread::zedDebugPrint(const char* str) {
    qDebug(str);
}

void ZedThread::zedErrorPrint(const char* str) {
    qDebug(str);
}

void ZedThread::zedGamePrint(const char* str) {
    qDebug(str);
}

void ZedThread::loadStory(const QByteArray& data) {
    
    // TODO: handle multithreading

    m_zed.copyStory(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

void ZedThread::resetStory() {
    QMutexLocker locker(&m_mutex);
    m_reset = true;
    m_singleStep = false;
    // start/wake thread
    if (!isRunning()) {
        start();
    }
    else {
        m_condition.wakeOne();
    }
}

void ZedThread::stepStory() {
    QMutexLocker locker(&m_mutex);
    m_singleStep = true;
    m_reset = false;
    // start/wake thread
    if (!isRunning()) {
        start();
    }
    else {
        m_condition.wakeOne();
    }
}

void ZedThread::continueStory() {
    QMutexLocker locker(&m_mutex);
    m_reset = false;
    m_singleStep = false;
    // start/wake thread
    if (!isRunning()) {
        start();
    }
    else {
        m_condition.wakeOne();
    }
}

void ZedThread::run() {
    forever{
        // check
        if (m_abort)
            return;

        // check what to do
        m_mutex.lock();
        // copy flags
        bool reset = m_reset;
        bool singleStep = m_singleStep;
        // reset flags
        m_reset = false;
        m_singleStep = false;
        m_mutex.unlock();

        if (reset) {
            m_zed.reset();
        }
        else if (singleStep) {
            m_zed.step();
        }
        else {
            m_zed.run();
        }

        m_mutex.lock();
        // check again
        if (m_abort)
            return;
        // wait for next flags
        m_condition.wait(&m_mutex);
        m_mutex.unlock();
    }
}

//============================================================//

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_zedThread(this)
{
    ui->setupUi(this);

    QTimer::singleShot(10, this, SLOT(timerUpdateState()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerUpdateState() {
    // copy stuff from zed instance
    // we don't care if we have race conditions here as it's just for displaying
    auto pc = m_zedThread.m_zed.m_state.pc;
    auto sp = m_zedThread.m_zed.m_state.sp;

    ui->txtPC->setPlainText(QString::number(pc, 16));
    ui->txtSP->setPlainText(QString::number(sp, 16));
    
    QTimer::singleShot(10, this, SLOT(timerUpdateState()));
}

void MainWindow::on_btnRefresh_clicked()
{

}

void MainWindow::on_btnLoad_clicked()
{
    // load story
    // const char* filename = "..\\..\\..\\..\\Data\\zork1-r119-s880429.z3";
    // const char* filename = "..\\..\\..\\..\\Data\\zork1-r88-s840726.z3u";

    QString dir = "../../../../Data";

    QString filename = QFileDialog::getOpenFileName(this, "Open story file", dir, "Z-Machine v3 stories (*.z3);;All files (*)");
    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox msgBox;
        msgBox.setText(QString("Could not open '%1'").arg(filename));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    QByteArray blob = file.readAll();

    m_zedThread.loadStory(blob);

    QMessageBox msgBox;
    msgBox.setText(QString("Loaded story '%0', %1 bytes").arg(filename).arg(blob.size()));
    msgBox.setIcon(QMessageBox::Icon::Information);
    msgBox.exec();
}

void MainWindow::on_btnStep_clicked() {
    m_zedThread.stepStory();
}

void MainWindow::customEvent(QEvent *e) {
    if (e->type() == StringEvent<STRING_EVENT_LOG>::staticType()) {
        /*QMessageBox* box = new QMessageBox();
        box->setWindowTitle(QString("Warning"));
        box->setText(QString("CPU USAGE is over 10% !"));
        box->show();*/
        // addLogTextDirect(dynamic_cast<StringEvent<STRING_EVENT_LOG> *>(e)->value());
    }
    else if (e->type() == StringEvent<STRING_EVENT_SERIAL_OUT>::staticType()) {
        // addLogTextDirect(dynamic_cast<StringEvent<STRING_EVENT_SERIAL_OUT> *>(e)->value());
    }
    else if (e->type() == StaticEvent<STATIC_EVENT_DISPLAY_CPU_STATE>::staticType()) {
        // TODO

    }

    // QApplication::postEvent(this, new StaticEvent<STATIC_EVENT_DISPLAY_CPU_STATE>());
}

void MainWindow::on_btnRun_clicked()
{
    m_zedThread.continueStory();
}

void MainWindow::on_btnReset_clicked()
{
    m_zedThread.resetStory();
}

void MainWindow::on_btnDisasm_clicked()
{
    // update disasm output
    ui->lstDisasm->clear();
    ui->lstDisasm->addItem("lol");
    ui->lstDisasm->addItem("lol");
    ui->lstDisasm->addItem("lol");
    ui->lstDisasm->addItem("lol");
    // update disasm line
    ui->lstDisasm->setCurrentRow(2);
}

void MainWindow::closeEvent(QCloseEvent *e)
{

}
