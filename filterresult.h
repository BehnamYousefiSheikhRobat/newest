#ifndef FILTERRESULT_H
#define FILTERRESULT_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QPrinter>

namespace Ui {
class filterresult;
}

class filterresult : public QMainWindow

{
    Q_OBJECT

public:
    explicit filterresult(QSqlTableModel *model, QWidget *parent = nullptr);
    ~filterresult();

private slots:
    void on_printButton_clicked();

private:
    Ui::filterresult *ui;
    QSqlTableModel *tableModel;   // ✅ این خط باید وجود داشته باشه

};

#endif // FILTERRESULT_H
