#pragma once

#include <QDialog>
#include "UserGuideContent.h"

class QListWidget;
class QTextBrowser;

class UserGuideDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserGuideDialog(UserGuideSection initialSection = UserGuideSection::Overview,
                             QWidget *parent = nullptr);

private slots:
    void onEntrySelected(int row);

private:
    void setupUi();
    void showSection(UserGuideSection section);

    QListWidget   *m_list   = nullptr;
    QTextBrowser  *m_body   = nullptr;
};
