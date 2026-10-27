#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codeeditor.h"
#include "finddialog.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createActions();
    createMenus();
    createToolBars();

    // 状态栏：显示光标位置与字符数
    m_statusLabel = new QLabel(this);
    m_statusLabel->setMinimumWidth(240);
    statusBar()->addWidget(m_statusLabel);

    // 编辑状态 → Action 使能 / 窗口标题 / 状态栏
    connect(ui->textEdit, &QPlainTextEdit::copyAvailable,
            m_actCut, &QAction::setEnabled);
    connect(ui->textEdit, &QPlainTextEdit::copyAvailable,
            m_actCopy, &QAction::setEnabled);
    connect(ui->textEdit, &QPlainTextEdit::undoAvailable,
            m_actUndo, &QAction::setEnabled);
    connect(ui->textEdit, &QPlainTextEdit::redoAvailable,
            m_actRedo, &QAction::setEnabled);
    connect(ui->textEdit->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::documentWasModified);
    connect(ui->textEdit, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusBar);
    connect(ui->textEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::updateStatusBar);
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updateClipboardState);

    // 初始使能状态
    m_actCut->setEnabled(false);
    m_actCopy->setEnabled(false);
    m_actUndo->setEnabled(false);
    m_actRedo->setEnabled(false);
    updateClipboardState();
    updateStatusBar();

    setCurrentFile(QString());
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------- 文件操作 ----------------

void MainWindow::newFile()
{
    if (maybeSave()) {
        ui->textEdit->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"));
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::saveFile()
{
    if (curFile.isEmpty())
        return saveAs();
    return saveFile(curFile);
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("保存文件"),
            tr("无法写入文件 %1：\n%2")
                .arg(QDir::toNativeSeparators(fileName))
                .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << ui->textEdit->toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    ui->textEdit->document()->setModified(false);
    statusBar()->showMessage(tr("文件已保存：%1").arg(strippedName(fileName)), 2000);
    return true;
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"));
    if (fileName.isEmpty())
        return false;
    return saveFile(fileName);
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        // 打开不存在或异常文件时的合理处理
        QMessageBox::warning(this, tr("打开文件"),
            tr("无法读取文件 %1：\n%2")
                .arg(QDir::toNativeSeparators(fileName))
                .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->textEdit->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("文件已读取：%1").arg(strippedName(fileName)), 2000);
}

bool MainWindow::maybeSave()
{
    if (!ui->textEdit->document()->isModified())
        return true;

    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("未保存"),
        tr("文档“%1”已被修改，是否保存？").arg(strippedName(curFile)),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
        return saveFile();
    if (ret == QMessageBox::Cancel)
        return false;
    return true;   // Discard
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    ui->textEdit->document()->setModified(false);
    setWindowModified(false);

    QString shown = curFile.isEmpty() ? tr("未命名.txt") : strippedName(curFile);
    setWindowTitle(shown + QStringLiteral("[*]"));
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::documentWasModified()
{
    setWindowModified(ui->textEdit->document()->isModified());
}

void MainWindow::updateStatusBar()
{
    QTextCursor cur = ui->textEdit->textCursor();
    int line = cur.blockNumber() + 1;
    int col  = cur.positionInBlock() + 1;
    int total = ui->textEdit->document()->characterCount() - 1;
    m_statusLabel->setText(tr("行 %1，列 %2    字符数 %3").arg(line).arg(col).arg(total));
}

// ---------------- 查找 / 替换 ----------------

void MainWindow::find()
{
    if (!m_findDlg) {
        m_findDlg = new FindDialog(this, false);
        connect(m_findDlg, &FindDialog::findNext,
                this, &MainWindow::findNext);
    }
    m_findDlg->setFindText(ui->textEdit->textCursor().selectedText());
    m_findDlg->show();
    m_findDlg->raise();
    m_findDlg->activateWindow();
}

void MainWindow::replace()
{
    if (!m_replaceDlg) {
        m_replaceDlg = new FindDialog(this, true);
        connect(m_replaceDlg, &FindDialog::findNext,
                this, &MainWindow::findNext);
        connect(m_replaceDlg, &FindDialog::replace,
                this, &MainWindow::replaceOne);
        connect(m_replaceDlg, &FindDialog::replaceAll,
                this, &MainWindow::replaceAllText);
    }
    m_replaceDlg->setFindText(ui->textEdit->textCursor().selectedText());
    m_replaceDlg->show();
    m_replaceDlg->raise();
    m_replaceDlg->activateWindow();
}

void MainWindow::findNext(const QString &text, QTextDocument::FindFlags flags)
{
    if (text.isEmpty())
        return;

    bool found = ui->textEdit->find(text, flags);
    if (!found) {
        // 未找到：从文档头部再尝试一次（支持循环查找）
        QTextCursor cur = ui->textEdit->textCursor();
        cur.movePosition(QTextCursor::Start);
        ui->textEdit->setTextCursor(cur);
        found = ui->textEdit->find(text, flags);
    }

    if (found)
        statusBar()->showMessage(tr("已找到匹配"), 1500);
    else
        statusBar()->showMessage(tr("未找到：%1").arg(text), 3000);
}

void MainWindow::replaceOne(const QString &findText, const QString &replaceText,
                            QTextDocument::FindFlags flags)
{
    findNext(findText, flags);   // 先定位到下一处匹配
    QTextCursor cur = ui->textEdit->textCursor();
    if (cur.hasSelection() && cur.selectedText() == findText) {
        cur.insertText(replaceText);
        statusBar()->showMessage(tr("已替换"), 1500);
    }
}

void MainWindow::replaceAllText(const QString &findText, const QString &replaceText,
                                QTextDocument::FindFlags flags)
{
    if (findText.isEmpty())
        return;

    QTextCursor cur = ui->textEdit->textCursor();
    cur.beginEditBlock();
    cur.movePosition(QTextCursor::Start);
    ui->textEdit->setTextCursor(cur);

    int count = 0;
    while (ui->textEdit->find(findText, flags)) {
        QTextCursor c = ui->textEdit->textCursor();
        if (c.hasSelection() && c.selectedText() == findText) {
            c.insertText(replaceText);
            ++count;
        }
    }
    cur.endEditBlock();

    statusBar()->showMessage(tr("共替换 %1 处").arg(count), 3000);
}

// ---------------- 格式 ----------------

void MainWindow::setFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, ui->textEdit->font(), this, tr("选择字体"));
    if (ok)
        ui->textEdit->setFont(font);
}

void MainWindow::setFontColor()
{
    QColor color = QColorDialog::getColor(Qt::black, this, tr("选择字体颜色"));
    if (color.isValid()) {
        QTextCharFormat fmt;
        fmt.setForeground(color);
        ui->textEdit->mergeCurrentCharFormat(fmt);
    }
}

void MainWindow::setBgColor()
{
    QColor color = QColorDialog::getColor(
        ui->textEdit->palette().color(QPalette::Base), this, tr("选择背景色"));
    if (color.isValid()) {
        QPalette pal = ui->textEdit->palette();
        pal.setColor(QPalette::Base, color);
        ui->textEdit->setPalette(pal);
    }
}

void MainWindow::toggleWrap(bool on)
{
    ui->textEdit->setLineWrapMode(on ? QPlainTextEdit::WidgetWidth
                                     : QPlainTextEdit::NoWrap);
}

void MainWindow::toggleLineNumbers(bool on)
{
    ui->textEdit->setLineNumbersVisible(on);
}

void MainWindow::updateClipboardState()
{
    m_actPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("关于 简单文本编辑器"),
        tr("简单文本编辑器\n\n"
           "基于 Qt 6 的多窗口文本编辑器示例\n"
           "学号：2024414290114   姓名：赖柳燕\n\n"
           "支持：文件新建/打开/保存、撤销重做、剪切复制粘贴、\n"
           "查找替换、字体/颜色/背景、自动换行、行号显示等。"));
}

// ---------------- 动作 / 菜单 / 工具栏 ----------------

void MainWindow::createActions()
{
    m_actNew = new QAction(tr("新建(&N)"), this);
    m_actNew->setShortcut(QKeySequence::New);
    connect(m_actNew, &QAction::triggered, this, &MainWindow::newFile);

    m_actOpen = new QAction(tr("打开(&O)..."), this);
    m_actOpen->setShortcut(QKeySequence::Open);
    connect(m_actOpen, &QAction::triggered, this, &MainWindow::openFile);

    m_actSave = new QAction(tr("保存(&S)"), this);
    m_actSave->setShortcut(QKeySequence::Save);
    connect(m_actSave, &QAction::triggered, this, [this]() { saveFile(); });

    m_actSaveAs = new QAction(tr("另存为(&A)..."), this);
    m_actSaveAs->setShortcut(QKeySequence::SaveAs);
    connect(m_actSaveAs, &QAction::triggered, this, &MainWindow::saveAs);

    m_actExit = new QAction(tr("退出(&X)"), this);
    m_actExit->setShortcut(QKeySequence::Quit);
    connect(m_actExit, &QAction::triggered, this, &QWidget::close);

    m_actUndo = new QAction(tr("撤销(&U)"), this);
    m_actUndo->setShortcut(QKeySequence::Undo);
    connect(m_actUndo, &QAction::triggered, ui->textEdit, &QPlainTextEdit::undo);

    m_actRedo = new QAction(tr("重做(&R)"), this);
    m_actRedo->setShortcut(QKeySequence::Redo);
    connect(m_actRedo, &QAction::triggered, ui->textEdit, &QPlainTextEdit::redo);

    m_actCut = new QAction(tr("剪切(&T)"), this);
    m_actCut->setShortcut(QKeySequence::Cut);
    connect(m_actCut, &QAction::triggered, ui->textEdit, &QPlainTextEdit::cut);

    m_actCopy = new QAction(tr("复制(&C)"), this);
    m_actCopy->setShortcut(QKeySequence::Copy);
    connect(m_actCopy, &QAction::triggered, ui->textEdit, &QPlainTextEdit::copy);

    m_actPaste = new QAction(tr("粘贴(&P)"), this);
    m_actPaste->setShortcut(QKeySequence::Paste);
    connect(m_actPaste, &QAction::triggered, ui->textEdit, &QPlainTextEdit::paste);

    m_actSelectAll = new QAction(tr("全选(&A)"), this);
    m_actSelectAll->setShortcut(QKeySequence::SelectAll);
    connect(m_actSelectAll, &QAction::triggered, ui->textEdit, &QPlainTextEdit::selectAll);

    m_actFind = new QAction(tr("查找(&F)..."), this);
    m_actFind->setShortcut(QKeySequence::Find);
    connect(m_actFind, &QAction::triggered, this, &MainWindow::find);

    m_actReplace = new QAction(tr("替换(&R)..."), this);
    m_actReplace->setShortcut(QKeySequence::Replace);
    connect(m_actReplace, &QAction::triggered, this, &MainWindow::replace);

    m_actFont = new QAction(tr("字体(&F)..."), this);
    connect(m_actFont, &QAction::triggered, this, &MainWindow::setFont);

    m_actFontColor = new QAction(tr("字体颜色"), this);
    connect(m_actFontColor, &QAction::triggered, this, &MainWindow::setFontColor);

    m_actBgColor = new QAction(tr("背景色"), this);
    connect(m_actBgColor, &QAction::triggered, this, &MainWindow::setBgColor);

    m_actWrap = new QAction(tr("自动换行"), this);
    m_actWrap->setCheckable(true);
    m_actWrap->setChecked(true);
    connect(m_actWrap, &QAction::toggled, this, &MainWindow::toggleWrap);

    m_actLineNumbers = new QAction(tr("显示行号"), this);
    m_actLineNumbers->setCheckable(true);
    m_actLineNumbers->setChecked(true);
    connect(m_actLineNumbers, &QAction::toggled, this, &MainWindow::toggleLineNumbers);

    m_actToggleToolBar = ui->mainToolBar->toggleViewAction();

    m_actToggleStatusBar = new QAction(tr("状态栏"), this);
    m_actToggleStatusBar->setCheckable(true);
    m_actToggleStatusBar->setChecked(true);
    connect(m_actToggleStatusBar, &QAction::toggled,
            statusBar(), &QWidget::setVisible);

    m_actAbout = new QAction(tr("关于(&A)"), this);
    connect(m_actAbout, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::createMenus()
{
    // 文件
    QMenu *fileMenu = menuBar()->addMenu(tr("文件(&F)"));
    fileMenu->addAction(m_actNew);
    fileMenu->addAction(m_actOpen);
    fileMenu->addAction(m_actSave);
    fileMenu->addAction(m_actSaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actExit);

    // 编辑
    QMenu *editMenu = menuBar()->addMenu(tr("编辑(&E)"));
    editMenu->addAction(m_actUndo);
    editMenu->addAction(m_actRedo);
    editMenu->addSeparator();
    editMenu->addAction(m_actCut);
    editMenu->addAction(m_actCopy);
    editMenu->addAction(m_actPaste);
    editMenu->addAction(m_actSelectAll);
    editMenu->addSeparator();
    editMenu->addAction(m_actFind);
    editMenu->addAction(m_actReplace);

    // 格式
    QMenu *formatMenu = menuBar()->addMenu(tr("格式(&O)"));
    formatMenu->addAction(m_actFont);
    formatMenu->addAction(m_actFontColor);
    formatMenu->addAction(m_actBgColor);
    formatMenu->addSeparator();
    formatMenu->addAction(m_actWrap);

    // 视图
    QMenu *viewMenu = menuBar()->addMenu(tr("视图(&V)"));
    viewMenu->addAction(m_actToggleToolBar);
    viewMenu->addAction(m_actToggleStatusBar);
    viewMenu->addSeparator();
    viewMenu->addAction(m_actLineNumbers);

    // 帮助
    QMenu *helpMenu = menuBar()->addMenu(tr("帮助(&H)"));
    helpMenu->addAction(m_actAbout);
}

void MainWindow::createToolBars()
{
    // 工具栏按功能分组
    ui->mainToolBar->addAction(m_actNew);
    ui->mainToolBar->addAction(m_actOpen);
    ui->mainToolBar->addAction(m_actSave);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(m_actUndo);
    ui->mainToolBar->addAction(m_actRedo);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(m_actCut);
    ui->mainToolBar->addAction(m_actCopy);
    ui->mainToolBar->addAction(m_actPaste);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(m_actFind);
    ui->mainToolBar->addAction(m_actReplace);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}
