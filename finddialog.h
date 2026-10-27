#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QTextDocument>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QCheckBox;
class QPushButton;
QT_END_NAMESPACE

// 查找/替换对话框：replaceMode=true 时为替换对话框，false 时为查找对话框
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr, bool replaceMode = false);

    // 打开对话框时用当前选中的文本预填“查找内容”
    void setFindText(const QString &text);

signals:
    // 每次点击“查找下一个”时发出
    void findNext(const QString &text, QTextDocument::FindFlags flags);
    // 点击“替换”时发出（替换当前匹配并移动到下一处）
    void replace(const QString &findText, const QString &replaceText,
                 QTextDocument::FindFlags flags);
    // 点击“替换全部”时发出
    void replaceAll(const QString &findText, const QString &replaceText,
                    QTextDocument::FindFlags flags);

private slots:
    void onFindClicked();
    void onReplaceClicked();
    void onReplaceAllClicked();

private:
    QLineEdit *findEdit;
    QLineEdit *replaceEdit;
    QCheckBox *caseCheckBox;
    bool       m_replaceMode;
};

#endif // FINDDIALOG_H
