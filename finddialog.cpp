#include "finddialog.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

FindDialog::FindDialog(QWidget *parent, bool replaceMode)
    : QDialog(parent), m_replaceMode(replaceMode)
{
    setWindowTitle(m_replaceMode ? tr("替换") : tr("查找"));

    QLabel *findLabel = new QLabel(tr("查找内容："), this);
    findEdit = new QLineEdit(this);

    caseCheckBox = new QCheckBox(tr("区分大小写"), this);

    QPushButton *findButton = new QPushButton(tr("查找下一个"), this);
    findButton->setDefault(true);
    QPushButton *closeButton = new QPushButton(tr("关闭"), this);

    // 查找布局
    QHBoxLayout *findLayout = new QHBoxLayout;
    findLayout->addWidget(findLabel);
    findLayout->addWidget(findEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(findButton);
    buttonLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(findLayout);
    mainLayout->addWidget(caseCheckBox);

    // 替换模式：增加“替换”相关控件
    if (m_replaceMode) {
        QLabel *replaceLabel = new QLabel(tr("替换为："), this);
        replaceEdit = new QLineEdit(this);

        QPushButton *replaceButton = new QPushButton(tr("替换"), this);
        QPushButton *replaceAllButton = new QPushButton(tr("替换全部"), this);

        QHBoxLayout *replaceLayout = new QHBoxLayout;
        replaceLayout->addWidget(replaceLabel);
        replaceLayout->addWidget(replaceEdit);

        QHBoxLayout *replaceBtnLayout = new QHBoxLayout;
        replaceBtnLayout->addWidget(replaceButton);
        replaceBtnLayout->addWidget(replaceAllButton);

        mainLayout->addLayout(replaceLayout);
        mainLayout->addLayout(replaceBtnLayout);

        connect(replaceButton, &QPushButton::clicked,
                this, &FindDialog::onReplaceClicked);
        connect(replaceAllButton, &QPushButton::clicked,
                this, &FindDialog::onReplaceAllClicked);
    }

    mainLayout->addLayout(buttonLayout);

    connect(findButton, &QPushButton::clicked, this, &FindDialog::onFindClicked);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    findEdit->setFocus();
}

void FindDialog::setFindText(const QString &text)
{
    if (!text.isEmpty())
        findEdit->setText(text);
}

void FindDialog::onFindClicked()
{
    QTextDocument::FindFlags flags;
    if (caseCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    emit findNext(findEdit->text(), flags);
}

void FindDialog::onReplaceClicked()
{
    QTextDocument::FindFlags flags;
    if (caseCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    emit replace(findEdit->text(), replaceEdit->text(), flags);
}

void FindDialog::onReplaceAllClicked()
{
    QTextDocument::FindFlags flags;
    if (caseCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    emit replaceAll(findEdit->text(), replaceEdit->text(), flags);
}
