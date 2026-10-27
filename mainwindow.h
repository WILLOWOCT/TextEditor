#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextDocument>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FindDialog;
class QLabel;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFile(const QString &fileName);
    bool saveAs();
    void about();

    void documentWasModified();
    void updateStatusBar();

    void find();
    void replace();
    void findNext(const QString &text, QTextDocument::FindFlags flags);
    void replaceOne(const QString &findText, const QString &replaceText,
                    QTextDocument::FindFlags flags);
    void replaceAllText(const QString &findText, const QString &replaceText,
                        QTextDocument::FindFlags flags);

    void setFont();
    void setFontColor();
    void setBgColor();
    void toggleWrap(bool on);
    void toggleLineNumbers(bool on);

    void updateClipboardState();

private:
    void createActions();
    void createMenus();
    void createToolBars();

    void loadFile(const QString &fileName);
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    Ui::MainWindow *ui;
    QString curFile;

    FindDialog *m_findDlg = nullptr;
    FindDialog *m_replaceDlg = nullptr;

    QLabel *m_statusLabel = nullptr;

    // 文件
    QAction *m_actNew = nullptr;
    QAction *m_actOpen = nullptr;
    QAction *m_actSave = nullptr;
    QAction *m_actSaveAs = nullptr;
    QAction *m_actExit = nullptr;
    // 编辑
    QAction *m_actUndo = nullptr;
    QAction *m_actRedo = nullptr;
    QAction *m_actCut = nullptr;
    QAction *m_actCopy = nullptr;
    QAction *m_actPaste = nullptr;
    QAction *m_actSelectAll = nullptr;
    QAction *m_actFind = nullptr;
    QAction *m_actReplace = nullptr;
    // 格式
    QAction *m_actFont = nullptr;
    QAction *m_actFontColor = nullptr;
    QAction *m_actBgColor = nullptr;
    QAction *m_actWrap = nullptr;
    // 视图
    QAction *m_actLineNumbers = nullptr;
    QAction *m_actToggleToolBar = nullptr;
    QAction *m_actToggleStatusBar = nullptr;
    // 帮助
    QAction *m_actAbout = nullptr;
};

#endif // MAINWINDOW_H
