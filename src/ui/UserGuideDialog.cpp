#include "UserGuideDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

UserGuideDialog::UserGuideDialog(UserGuideSection initialSection, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("TimeGrapher — User Guide"));
    setMinimumSize(720, 520);
    resize(860, 600);
    setupUi();
    showSection(initialSection);
}

void UserGuideDialog::setupUi()
{
    auto *root = new QVBoxLayout(this);

    auto *bodyRow = new QHBoxLayout;
    m_list = new QListWidget(this);
    m_list->setFixedWidth(220);

    const auto all = UserGuideContent::entries();
    QString currentGroup;
    for (const auto &entry : all) {
        if (entry.group != currentGroup) {
            currentGroup = entry.group;
            auto *header = new QListWidgetItem(currentGroup, m_list);
            header->setFlags(Qt::NoItemFlags);
            QFont f = header->font();
            f.setBold(true);
            header->setFont(f);
            header->setForeground(QColor("#6b7280"));
        }
        auto *item = new QListWidgetItem(entry.title, m_list);
        item->setData(Qt::UserRole, static_cast<int>(entry.section));
    }

    m_body = new QTextBrowser(this);
    m_body->setOpenExternalLinks(true);

    bodyRow->addWidget(m_list);
    bodyRow->addWidget(m_body, 1);
    root->addLayout(bodyRow, 1);

    auto *closeBtn = new QPushButton(tr("Close"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    root->addWidget(closeBtn, 0, Qt::AlignRight);

    connect(m_list, &QListWidget::currentRowChanged,
            this, &UserGuideDialog::onEntrySelected);
}

void UserGuideDialog::showSection(UserGuideSection section)
{
    const int idx = UserGuideContent::indexOf(section);
    int row = 0;
    int entryIndex = 0;
    for (int r = 0; r < m_list->count(); ++r) {
        const auto flags = m_list->item(r)->flags();
        if (!(flags & Qt::ItemIsSelectable))
            continue;
        if (entryIndex == idx) {
            row = r;
            break;
        }
        ++entryIndex;
    }
    m_list->setCurrentRow(row);
}

void UserGuideDialog::onEntrySelected(int row)
{
    if (row < 0)
        return;
    const QListWidgetItem *item = m_list->item(row);
    if (!item || !(item->flags() & Qt::ItemIsSelectable))
        return;

    const auto section = static_cast<UserGuideSection>(item->data(Qt::UserRole).toInt());
    const int idx = UserGuideContent::indexOf(section);
    const auto all = UserGuideContent::entries();
    if (idx >= 0 && idx < all.size())
        m_body->setHtml(all[idx].html);
}
