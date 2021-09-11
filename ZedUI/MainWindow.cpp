
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QTimer>


//============================================================//

#define STRING_EVENT_LOG                    0
#define STRING_EVENT_SERIAL_OUT             1

#define STATIC_EVENT_DISPLAY_CPU_STATE      0

//#define QUEUE_TYPE 1 // moodycamels'
#define QUEUE_TYPE 2 // easy queue


//============================================================//

void GuiUpdater::addLogText(const QString &text) {
#if QUEUE_TYPE == 1
#if 0
        m_logQueue.enqueue_bulk(&text[0], text.size());
#else
    for (uint i = 0; i < text.size(); ++i) {
        m_logQueue.enqueue(text[i]);
    }
#endif
#elif QUEUE_TYPE == 2
    for (uint i = 0; i < text.size(); ++i) {
        m_logQueue.push(text[i]);
    }
#endif
}

void GuiUpdater::addSerialText(const QString &text) {
#if QUEUE_TYPE == 1
    m_serialQueue.enqueue_bulk(&text[0], text.size());
#elif QUEUE_TYPE == 2
    for (uint i = 0; i < text.size(); ++i) {
        m_serialQueue.push(text[i]);
    }
#endif
}

void GuiUpdater::start() {
    QTimer::singleShot(0, this, SLOT(update()));
}

void GuiUpdater::update() {
    //Utils::log("asdasd\n");
    //m_parent->addLogTextDirect("aisdjaijdsiajsdi\n");

#if QUEUE_TYPE == 1
    size_t logSize = m_logQueue.size_approx();
    m_tempLog.reserve(logSize + 1);
    //m_tempLog.clear();

#if 0
    size_t actualSize = m_logQueue.try_dequeue_bulk(&m_tempLog[0], logSize);
    if (actualSize > 0) {
        m_tempLog[(uint) actualSize] = QChar::Null;
        m_parent->addLogTextDirect(m_tempLog);
    }
#else
    uint i = 0;
    QChar ch;
    while (m_logQueue.try_dequeue(ch) && (i < logSize)) {
        m_tempLog[i] = ch;
        ++i;
    }
    if (i > 0) {
        m_tempLog[i] = QChar::Null;
        m_parent->addLogTextDirect(m_tempLog);
    }
#endif

#elif QUEUE_TYPE == 2
    size_t logSize = m_logQueue.size();
    if (m_tempLog.size() < (logSize + 1))
        m_tempLog.resize(logSize + 1);
    //m_tempLog.clear();

#if 0
    size_t actualSize = m_logQueue.try_dequeue_bulk(&m_tempLog[0], logSize);
    if (actualSize > 0) {
        m_tempLog[(uint) actualSize] = QChar::Null;
        m_parent->addLogTextDirect(m_tempLog);
    }
#else
    uint i = 0;
    QChar ch;
    while (i < logSize) {
        m_logQueue.pop(ch);
        m_tempLog[i] = ch;
        ++i;
    }
    if (i > 0) {
        //m_tempLog[i] = QChar::Null;
        m_tempLog.truncate(i);
        m_parent->addLogTextDirect(m_tempLog);
    }
#endif
#endif

#if 0
    size_t serialSize = m_serialQueue.size_approx();
    m_tempSerial.reserve(serialSize + 1);
    //m_tempSerial.clear();
    actualSize = m_serialQueue.try_dequeue_bulk(&m_tempSerial[0], serialSize);
    if (actualSize > 0) {
        m_tempSerial[(uint) actualSize] = QChar::Null;
        m_parent->addSerialTextDirect(m_tempSerial);
    }
#endif


#if QUEUE_TYPE == 2
    logSize = m_serialQueue.size();
    if (m_tempSerial.size() < (logSize + 1))
        m_tempSerial.resize(logSize + 1);
    //m_tempLog.clear();

    for (i = 0; i < logSize; ++i) {
        m_serialQueue.pop(ch);
        m_tempSerial[i] = ch;
    }
    if (i > 0) {
        m_tempSerial.truncate(i);
        m_parent->addSerialTextDirect(m_tempSerial);
    }
#endif

    /*if (m_parent->getArch()->m_emuThread) {
        m_parent->displayCpuState(&m_parent->getArch()->m_cpuState);
    }*/

    //QTimer::singleShot(0, this, SLOT(update())); 
    QTimer::singleShot(10, this, SLOT(update())); 
}

//============================================================//

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_guiUpdater(this)
{
    ui->setupUi(this);

    // TODO: init

    m_guiUpdater.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnRefresh_clicked()
{
    // ui->lstRoms->clear();
    ui->lstStories->addItem("blabla");
}

void MainWindow::on_btnLoad_clicked()
{
#if 0
    // load a rom
    if (ui->lstRoms->currentItem() != nullptr) {
        std::string str = ui->lstRoms->currentItem()->text().toStdString();
        printf("%s\n", str.c_str());
    }
#endif

    // load rom
#ifdef LINUX
    std::string romFilename;
    if (ui->lstRoms->currentItem() != nullptr) {
        romFilename = ui->lstRoms->currentItem()->text().toStdString();
    } else {
        QMessageBox box(QMessageBox::Critical, "Error", "No ROM selected");
        box.exec();
        return;
    }
#else
    //int ret = m_arch.loadRom("../Data/T.bin");
    //int ret = m_arch.loadRomRaw("../Data/DMG_ROM.bin", 0);
    //m_arch.loadRom("../Data/SML.bin");
    //m_arch.loadRom("../Data/DK.bin");
    //int ret = m_arch.loadRom("../Docs/Blargg test ROMs/cpu_instrs/cpu_instrs.gb");
    //int ret = m_arch.loadRom("../Docs/Blargg test ROMs/cpu_instrs/individual/01-special.gb");
    //std::string romFilename = "../Data/cpu_instrs.gb";
    std::string romFilename = "../Data/04-op r,imm.gb";
#endif

}

void MainWindow::on_btnStep_clicked() {
    addSerialText("test serial text");
}

/*void MainWindow::addLogText(const QString &text) {
    QApplication::postEvent(this, new StringEvent<STRING_EVENT_LOG>(text));
}*/

void MainWindow::addLogTextDirect(const QString &text) {
    QPlainTextEdit *txt = getUi()->txtLog;
    QTextCursor prev_cursor = txt->textCursor();
    txt->moveCursor(QTextCursor::End);
    txt->insertPlainText(text);
    txt->setTextCursor(prev_cursor);
}

/*void MainWindow::addSerialText(const QString &text) {
    QApplication::postEvent(this, new StringEvent<STRING_EVENT_SERIAL_OUT>(text));
}*/

void MainWindow::addSerialTextDirect(const QString &text) {
    QPlainTextEdit *txt = getUi()->txtSerialOut;
    QTextCursor prev_cursor = txt->textCursor();
    txt->moveCursor(QTextCursor::End);
    txt->insertPlainText(text);
    txt->setTextCursor(prev_cursor);
}

void MainWindow::customEvent(QEvent *e) {
    if (e->type() == StringEvent<STRING_EVENT_LOG>::staticType()) {
        /*QMessageBox* box = new QMessageBox();
        box->setWindowTitle(QString("Warning"));
        box->setText(QString("CPU USAGE is over 10% !"));
        box->show();*/
        addLogTextDirect(dynamic_cast<StringEvent<STRING_EVENT_LOG> *>(e)->value());
    }
    else if (e->type() == StringEvent<STRING_EVENT_SERIAL_OUT>::staticType()) {
        addLogTextDirect(dynamic_cast<StringEvent<STRING_EVENT_SERIAL_OUT> *>(e)->value());
    }
    else if (e->type() == StaticEvent<STATIC_EVENT_DISPLAY_CPU_STATE>::staticType()) {
        // TODO

    }
}

void MainWindow::reqDisplayCpuState() {
    QApplication::postEvent(this, new StaticEvent<STATIC_EVENT_DISPLAY_CPU_STATE>());
}

/*void MainWindow::displayCpuState(BartBoy::GBCpuState *state) {
    ui->txtA->setPlainText(Utils::formatStringFast("%02X", state->A).c_str());
    ui->txtF->setPlainText(Utils::formatStringFast("%02X", state->F).c_str());
    ui->txtB->setPlainText(Utils::formatStringFast("%02X", state->B).c_str());
    ui->txtC->setPlainText(Utils::formatStringFast("%02X", state->C).c_str());
    ui->txtD->setPlainText(Utils::formatStringFast("%02X", state->D).c_str());
    ui->txtE->setPlainText(Utils::formatStringFast("%02X", state->E).c_str());
    ui->txtH->setPlainText(Utils::formatStringFast("%02X", state->H).c_str());
    ui->txtL->setPlainText(Utils::formatStringFast("%02X", state->L).c_str());
    ui->txtSP->setPlainText(Utils::formatStringFast("%04X", state->SP).c_str());
    ui->txtPC->setPlainText(Utils::formatStringFast("%04X", state->PC).c_str());

    ui->chkZ->setChecked((state->F & GB_FLAG_Z) != 0);
    ui->chkN->setChecked((state->F & GB_FLAG_N) != 0);
    ui->chkH->setChecked((state->F & GB_FLAG_H) != 0);
    ui->chkC->setChecked((state->F & GB_FLAG_C) != 0);

    ui->txtCycles->setPlainText(Utils::formatStringFast("%lu", state->numCycles).c_str());

    ui->txtMemPC->clear();
    for (int i = 0; i < 8; ++i) {
        BartBoy::uint8 b1 = m_arch.m_memory.read8(state->PC + i * 4 + 0x00);
        BartBoy::uint8 b2 = m_arch.m_memory.read8(state->PC + i * 4 + 0x01);
        BartBoy::uint8 b3 = m_arch.m_memory.read8(state->PC + i * 4 + 0x02);
        BartBoy::uint8 b4 = m_arch.m_memory.read8(state->PC + i * 4 + 0x03);
        ui->txtMemPC->appendPlainText(Utils::formatStringFast("%02X %02X %02X %02X", b1, b2, b3, b4).c_str());
    }

    ui->txtMemSP->clear();
    for (int i = 0; i < 8; ++i) {
        BartBoy::uint8 b1 = m_arch.m_memory.read8(state->SP - i * 1 - 0x00);
        //uint8 b2 = m_arch.m_memory.read8(state->SP - i * 4 - 0x01);
        //uint8 b3 = m_arch.m_memory.read8(state->SP - i * 4 - 0x02);
        //uint8 b4 = m_arch.m_memory.read8(state->SP - i * 4 - 0x03);
        //ui->txtPC->appendPlainText(Utils::formatStringFast("%02X %02X %02X %02X", b1, b2, b3, b4).c_str());
        ui->txtMemSP->appendPlainText(Utils::formatStringFast("%02X", b1).c_str());
    }

    // update disasm
    int line = m_arch.m_cpu.getDisasmLine(state->PC);
    ui->lstDisasm->setCurrentRow(line);
}*/

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
