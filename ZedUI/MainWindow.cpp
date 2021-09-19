
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QString>
#include <QFile>
#include <QFileDialog>
#include <QScrollBar>

//QTimer::singleShot(0, this, SLOT(update())); 
// QTimer::singleShot(10, this, SLOT(update()));

#define UPDATE_TIMER_INTERVAL       100

//============================================================//

#define STRING_EVENT_LOG                    0
#define STRING_EVENT_SERIAL_OUT             1

#define STATIC_EVENT_DISPLAY_CPU_STATE      0

//============================================================//

ZedThread::ZedThread(MainWindow* mainWindow) :
    QThread(mainWindow)
{
#if 0
    m_zed.debugPrintCallback = std::bind(&ZedThread::zedDebugPrint, this, std::placeholders::_1);
    m_zed.errorPrintCallback = std::bind(&ZedThread::zedErrorPrint, this, std::placeholders::_1);
    m_zed.gamePrintCallback = std::bind(&ZedThread::zedGamePrint, this, std::placeholders::_1);
#else
    m_zed.debugPrintCallback = [&](const char* data) {
        emit zedDebugPrint(QString(data));
    };
    m_zed.errorPrintCallback = [&](const char* data) {
        emit zedErrorPrint(QString(data));
    };
    m_zed.gamePrintCallback = [&](const char* data) {
        emit zedGamePrint(QString(data));
    };
#endif
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

/*void ZedThread::zedDebugPrint(const char* str) {
    qDebug(str);
}

void ZedThread::zedErrorPrint(const char* str) {
    qDebug(str);
}

void ZedThread::zedGamePrint(const char* str) {
    qDebug(str);
}*/

bool ZedThread::loadStory(const QByteArray& data) {
    // try to lock the run mutex
    if (m_runMutex.tryLock()) {
        // if ok, setup the story
        m_zed.copyStory(reinterpret_cast<const uint8_t*>(data.data()), data.size());
        // unlock it
        m_runMutex.unlock();
        return true;
    }
    else {
        emit zedErrorPrint("Cannot load story while running");
        return false;
    }
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
    TextBuffer tb;
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

        // execute action
        m_runMutex.lock();
        if (reset) {
            m_zed.reset();
        }
        else if (singleStep) {
            m_zed.disasmCurInstruction(tb);
            puts(tb.buf); // TEMP
            emit zedAddDisasm(QString(tb.buf));
            m_zed.step();
        }
        else {
            // m_zed.run();
            bool ret = true;
            while (ret) {
                m_zed.disasmCurInstruction(tb);
                puts(tb.buf); // TEMP
                emit zedAddDisasm(QString(tb.buf));
                ret = m_zed.step();
            }
        }
        m_runMutex.unlock();

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

    m_gameText[0].setString(&m_gameStr[0]);
    m_gameText[1].setString(&m_gameStr[1]);
    m_debugText[0].setString(&m_debugStr[0]);
    m_debugText[1].setString(&m_debugStr[1]);

    connect(&m_zedThread, &ZedThread::zedDebugPrint, this, &MainWindow::zedDebugPrint);
    connect(&m_zedThread, &ZedThread::zedErrorPrint, this, &MainWindow::zedErrorPrint);
    connect(&m_zedThread, &ZedThread::zedGamePrint, this, &MainWindow::zedGamePrint);
    connect(&m_zedThread, &ZedThread::zedAddDisasm, this, &MainWindow::zedAddDisasm);

    ui->btnRun->setEnabled(false);
    ui->btnStep->setEnabled(false);
    ui->btnReset->setEnabled(false);
    ui->btnOk->setEnabled(false);

    QTimer::singleShot(UPDATE_TIMER_INTERVAL, this, SLOT(timerUpdateState()));

#if 1 // TEMP
    QString filename2 = "..\\..\\..\\..\\Data\\zork1-r88-s840726.z3";
    QTimer::singleShot(20, this, [=]() { QString filename = filename2; loadStory(filename); });
#endif
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::timerUpdateState() {
    // change buffer, for double buffering
    m_bufferMutex.lock();
    int cur = m_curBuffer;
    m_curBuffer ^= 1;
    m_bufferMutex.unlock();

    // copy stuff from zed instance
    // we don't care if we have race conditions here as it's just for displaying
    auto pc = m_zedThread.m_zed.m_state.pc;
    auto sp = m_zedThread.m_zed.m_state.sp;

    ui->txtPC->setPlainText(QString::number(pc, 16));
    ui->txtSP->setPlainText(QString::number(sp, 16));

    // set game/debug text
    if (!m_gameStr[cur].isEmpty()) {
        ui->txtLog->insertPlainText(m_gameStr[cur]);
        ui->txtLog->verticalScrollBar()->setValue(ui->txtLog->verticalScrollBar()->maximum());
        m_gameText[cur].reset();
        m_gameStr[cur].truncate(0);
    }
    if (!m_debugStr[cur].isEmpty()) {
        ui->txtSerialOut->insertPlainText(m_debugStr[cur]);
        ui->txtSerialOut->verticalScrollBar()->setValue(ui->txtSerialOut->verticalScrollBar()->maximum());
        m_debugText[cur].reset();
        m_debugStr[cur].truncate(0);
    }
    // add disasm instructions
    /*auto len = m_disasmQueue[cur].size();
    for (auto i = 0; i < len; ++i) {
        ui->lstDisasm->addItems(m_disasmQueue[cur]);
    }
    m_disasmQueue[cur].clear();*/
    m_disasmMutex.lock();
    int i = 0;
    // NOTE: adding a rate limit here
    while (!m_disasmQueue.isEmpty() && i < 10) {
        ui->lstDisasm->addItem(m_disasmQueue.dequeue());
        ++i;
    }
    m_disasmMutex.unlock();
    // scroll down
    if (i > 0)
        ui->lstDisasm->verticalScrollBar()->setValue(ui->lstDisasm->verticalScrollBar()->maximum());
    
    QTimer::singleShot(UPDATE_TIMER_INTERVAL, this, SLOT(timerUpdateState()));
}

void MainWindow::zedDebugPrint(const QString &str) {
    QMutexLocker locker(&m_bufferMutex);
    m_debugText[m_curBuffer] << str;
}

void MainWindow::zedErrorPrint(const QString& str) {
    QMessageBox msgBox;
    msgBox.setText(str);
    msgBox.setIcon(QMessageBox::Icon::Critical);
    msgBox.exec();
}

void MainWindow::zedGamePrint(const QString& str) {
    QMutexLocker locker(&m_bufferMutex);
    m_gameText[m_curBuffer] << str;
}

void MainWindow::zedAddDisasm(const QString& str) {
    /*QMutexLocker locker(&m_bufferMutex);
    m_disasmQueue[m_curBuffer].append(str);*/
    QMutexLocker locker(&m_disasmMutex);
    m_disasmQueue.enqueue(str);
}

void MainWindow::on_actionLoadStory_triggered() {
    // load story
    // const char* filename = "..\\..\\..\\..\\Data\\zork1-r119-s880429.z3";
    // const char* filename = "..\\..\\..\\..\\Data\\zork1-r88-s840726.z3u";

    QString dir = "../../../../Data";

    QString filename = QFileDialog::getOpenFileName(this, "Open story file", dir, "Z-Machine v3 stories (*.z3);;All files (*)");
    if (filename.isEmpty())
        return;
    loadStory(filename);
}

void MainWindow::loadStory(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox msgBox;
        msgBox.setText(QString("Could not open '%1'").arg(filename));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    QByteArray blob = file.readAll();

    if (m_zedThread.loadStory(blob)) {
        /*QMessageBox msgBox;
        msgBox.setText(QString("Loaded story '%0', %1 bytes").arg(filename).arg(blob.size()));
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.exec();*/
        ui->btnRun->setEnabled(true);
        ui->btnStep->setEnabled(true);
        ui->btnReset->setEnabled(true);
        ui->btnOk->setEnabled(true);
    }
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

void MainWindow::on_btnRun_clicked() {
    m_zedThread.continueStory();
}

void MainWindow::on_btnStep_clicked() {
    m_zedThread.stepStory();
}

void MainWindow::on_btnReset_clicked() {
    m_zedThread.resetStory();
}

void MainWindow::closeEvent(QCloseEvent *e) {

}
