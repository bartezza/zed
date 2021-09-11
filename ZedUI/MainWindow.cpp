
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QString>

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

}

void ZedThread::run() {

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

//============================================================//

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnRefresh_clicked()
{

}

void MainWindow::on_btnLoad_clicked()
{
    // load story
    // const char* filename = "..\\..\\..\\..\\Data\\zork1-r119-s880429.z3";
    const char* filename = "..\\..\\..\\..\\Data\\zork1-r88-s840726.z3";
    // open story file
    FILE* fp = fopen(filename, "rb");
    if (fp == nullptr) {
        QMessageBox msgBox;
        msgBox.setText(QString::asprintf("Could not load '%s'", filename));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    // get total size
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // alloc memory
    std::vector<uint8_t> mem(size);
    // read the file
    if (fread(&mem[0], 1, size, fp) != size) {
        QMessageBox msgBox;
        msgBox.setText(QString::asprintf("Could not read % i bytes of story file", size));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    fclose(fp);

    // copy story
    // zed.copyStory(mem.data(), mem.size());
    // TODO

    QMessageBox msgBox;
    msgBox.setText(QString::asprintf("Loaded story '%s'", filename));
    msgBox.setIcon(QMessageBox::Icon::Information);
    msgBox.exec();
}

void MainWindow::on_btnStep_clicked() {

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
    
}

void MainWindow::on_btnReset_clicked()
{
    
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
