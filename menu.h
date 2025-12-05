#ifndef MENU_H
#define MENU_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
namespace Ui { class menu; }
QT_END_NAMESPACE

class menu : public QMainWindow
{
    Q_OBJECT

public:
    explicit menu(QWidget *parent = nullptr);
    ~menu();

private slots:
    void on_registerButton_clicked();
    void on_addCompanyButton_clicked();
    void on_searchButton_clicked();
    void on_deleteButton_clicked();
    void on_filterAll_clicked();
    void on_filterDriver_clicked();
    void on_filterMedical_clicked();
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_filterByDateButton_clicked();
    void on_ncEdit_editingFinished();
    void on_ageediting_Finished();



    void on_medicalType_2_clicked();

    void on_lineEdit_editingFinished();

    void on_ageedit_editingFinished();

    void on_ageedit_cursorPositionChanged(int arg1, int arg2);

private:
    Ui::menu *ui;
    QSqlDatabase db;
    QDate jalaliToGregorian(const QString &jdate); // برای مقایسه بین دو تاریخ شمسی

    void refreshModel();
    void clearFields();
    QString gregorianToJalali(const QDate &gdate);
};

#endif // MENU_H
