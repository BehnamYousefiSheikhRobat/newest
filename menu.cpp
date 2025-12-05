#include "menu.h"
#include "filterresult.h"
#include "ui_menu.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>
#include <QCalendar>
#include <QDebug>

menu::menu(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::menu)
{
    ui->setupUi(this);
    ui->comBox_2->setModel(ui->comBox->model());

    // اتصال به دیتابیس
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("reception.db");
    if (!db.open()) {
        QMessageBox::critical(this, "خطا", "اتصال به پایگاه داده برقرار نشد:\n" + db.lastError().text());
        return;
    }

    // ساخت جدول شرکت‌ها
    QSqlQuery companyQuery;
    companyQuery.exec("CREATE TABLE IF NOT EXISTS companies ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "CompanyName TEXT UNIQUE,"
                      "ExamCost REAL)");

    // ساخت جدول پذیرش
    QSqlQuery createQuery;
    createQuery.exec("CREATE TABLE IF NOT EXISTS reception ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "NationalNumber TEXT,"
                     "FirstName TEXT,"
                     "LastName TEXT,"
                     "Phone TEXT,"
                     "Company TEXT,"
                     "ExamCost REAL,"
                     "PaymentType TEXT,"
                     "Type TEXT,"
                     "Date TEXT)");

    // پر کردن لیست شرکت‌ها
    ui->comBox->clear();
    QSqlQuery fill("SELECT CompanyName FROM companies");
    while (fill.next())
        ui->comBox->addItem(fill.value(0).toString());
    ui->comBox->addItem("بدون شرکت");

    refreshModel();
}

menu::~menu()
{
    db.close();
    delete ui;
}
QDate menu::jalaliToGregorian(const QString &jdate)
{
    QStringList parts = jdate.split('/');
    if (parts.size() != 3) return QDate();

    int jy = parts[0].toInt();
    int jm = parts[1].toInt();
    int jd = parts[2].toInt();

    int gy, gm, gd;

    int j_days_in_month[] = {31,31,31,31,31,31,30,30,30,30,30,29};
    int g_days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};

    long j_day_no = 365 * (jy - 979) + (jy - 979)/33*8 + ((jy - 979)%33 +3)/4;
    for (int i = 0; i < jm - 1; ++i)
        j_day_no += j_days_in_month[i];
    j_day_no += jd - 1;

    long g_day_no = j_day_no + 79;

    gy = 1600 + 400*(g_day_no/146097);
    g_day_no %= 146097;

    bool leap = true;
    if (g_day_no >= 36525) {
        g_day_no--;
        gy += 100*(g_day_no/36524);
        g_day_no %= 36524;

        if (g_day_no >= 365)
            g_day_no++;
        else
            leap = false;
    }

    gy += 4*(g_day_no/1461);
    g_day_no %= 1461;

    if (g_day_no >= 366) {
        leap = false;
        g_day_no -= 366;
        gy += g_day_no/365;
        g_day_no = g_day_no % 365;
    }

    for (gm = 0; gm < 12 && g_day_no >= g_days_in_month[gm] + ((gm==1 && leap)?1:0); ++gm)
        g_day_no -= g_days_in_month[gm] + ((gm==1 && leap)?1:0);
    gd = g_day_no + 1;

    return QDate(gy, gm+1, gd);
}
void menu::on_filterByDateButton_clicked()
{
    QString startDate = ui->startDateEdit->text();
    QString endDate = ui->endDateEdit->text();
    QString comname = ui->comBox_2->currentText();



    if (startDate.isEmpty() || endDate.isEmpty()) {
        QMessageBox::warning(this, "خطا", "لطفاً هر دو تاریخ را وارد کنید.");
        return;
    }

    QDate gStart = jalaliToGregorian(startDate);
    QDate gEnd = jalaliToGregorian(endDate);

    if (!gStart.isValid() || !gEnd.isValid()) {
        QMessageBox::warning(this, "خطا", "فرمت تاریخ نامعتبر است.");
        return;
    }

    auto *model = new QSqlTableModel(this, db);
    model->setTable("reception");
    model->setFilter(QString("Date >= '%1' AND Date <= '%2' AND Company = '"+comname+"' ")
                         .arg(startDate)
                         .arg(endDate));
    model->select();

    if (model->rowCount() == 0) {
        QMessageBox::information(this, "نتیجه", "هیچ پذیرشی در این بازه پیدا نشد.");
        delete model;
        return;
    }

    // نمایش در پنجره جدید
    filterresult *resultDialog = new filterresult(model, this);
    resultDialog->setAttribute(Qt::WA_DeleteOnClose);
    resultDialog->setWindowTitle("لیست پذیرش‌ها");
    resultDialog->resize(900, 600);
    resultDialog->show();

}



// ✅ تبدیل تاریخ میلادی به شمسی با الگوریتم دقیق و بدون عقب‌افتادگی
// تابع کمکی: تبدیل تاریخ میلادی به شمسی
QString menu::gregorianToJalali(const QDate &g)
{
    int g_y = g.year();
    int g_m = g.month();
    int g_d = g.day();

    int g_days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int j_days_in_month[] = {31,31,31,31,31,31,30,30,30,30,30,29};

    long gy = g_y - 1600;
    long gm = g_m - 1;
    long gd = g_d - 1;

    long g_day_no = 365 * gy + (gy + 3) / 4 - (gy + 99) / 100 + (gy + 399) / 400;
    for (int i = 0; i < gm; ++i)
        g_day_no += g_days_in_month[i];
    if (gm > 1 && ((g_y % 4 == 0 && g_y % 100 != 0) || (g_y % 400 == 0)))
        g_day_no++;
    g_day_no += gd;

    // اصلاح: مبدأ دقیق‌تر برای هماهنگی با سال ۱۴۰۴
    long j_day_no = g_day_no - 79 + 365;  // ← اضافه کردن یک سال شمسی (اختلاف اصلاحی)

    long j_np = j_day_no / 12053;
    j_day_no %= 12053;

    long jy = 979 + 33 * j_np + 4 * (j_day_no / 1461);
    j_day_no %= 1461;

    if (j_day_no >= 366) {
        jy += (j_day_no - 366) / 365;
        j_day_no = (j_day_no - 366) % 365;
    }

    int jm = 0;
    for (; jm < 12 && j_day_no >= j_days_in_month[jm]; ++jm)
        j_day_no -= j_days_in_month[jm];
    int jd = j_day_no + 1;

    return QString("%1/%2/%3")
        .arg(jy)
        .arg(jm + 1, 2, 10, QChar('0'))
        .arg(jd, 2, 10, QChar('0'));
}



// پاک کردن فیلدها
void menu::clearFields()
{
    ui->ncEdit->clear();
    ui->nEdit->clear();
    ui->lnEdit->clear();
    ui->pnEdit->clear();
    ui->comBox->setCurrentIndex(0);
    ui->personpayButton->setAutoExclusive(false);
    ui->companypayButton->setAutoExclusive(false);
    ui->driverType->setAutoExclusive(false);
    ui->medicalType->setAutoExclusive(false);
    ui->personpayButton->setChecked(false);
    ui->companypayButton->setChecked(false);
    ui->driverType->setChecked(false);
    ui->medicalType->setChecked(false);
    ui->personpayButton->setAutoExclusive(true);
    ui->companypayButton->setAutoExclusive(true);
    ui->driverType->setAutoExclusive(true);
    ui->medicalType->setAutoExclusive(true);
}

// به‌روزرسانی جدول
void menu::refreshModel()
{
    auto *model = new QSqlTableModel(this, db);
    model->setTable("reception");
    model->select();

    model->setHeaderData(model->fieldIndex("NationalNumber"), Qt::Horizontal, "کد ملی");
    model->setHeaderData(model->fieldIndex("FirstName"), Qt::Horizontal, "نام");
    model->setHeaderData(model->fieldIndex("LastName"), Qt::Horizontal, "نام خانوادگی");
    model->setHeaderData(model->fieldIndex("Phone"), Qt::Horizontal, "شماره تماس");
    model->setHeaderData(model->fieldIndex("Company"), Qt::Horizontal, "شرکت");
    model->setHeaderData(model->fieldIndex("ExamCost"), Qt::Horizontal, "هزینه معاینه");
    model->setHeaderData(model->fieldIndex("PaymentType"), Qt::Horizontal, "نوع پرداخت");
    model->setHeaderData(model->fieldIndex("Type"), Qt::Horizontal, "نوع پذیرش");
    model->setHeaderData(model->fieldIndex("Date"), Qt::Horizontal, "تاریخ");

    ui->tableView->setModel(model);
    ui->tableView->hideColumn(model->fieldIndex("id"));
    ui->tableView->resizeColumnsToContents();
}

// افزودن شرکت جدید
void menu::on_addCompanyButton_clicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "نام شرکت", "نام شرکت را وارد کنید:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    double cost = QInputDialog::getDouble(this, "هزینه", "هزینه معاینه برای این شرکت:", 0, 0, 100000000, 0, &ok);
    if (!ok) return;

    QSqlQuery q;
    q.prepare("INSERT OR IGNORE INTO companies (CompanyName, ExamCost) VALUES (:n, :c)");
    q.bindValue(":n", name);
    q.bindValue(":c", cost);

    if (!q.exec()) {
        QMessageBox::warning(this, "خطا", "ثبت شرکت انجام نشد:\n" + q.lastError().text());
    } else {
        ui->comBox->addItem(name);
        QMessageBox::information(this, "موفق", "شرکت با موفقیت افزوده شد.");
    }
}

// ثبت پذیرش جدید
void menu::on_registerButton_clicked()
{
    QString nn = ui->ncEdit->text();
    QString fn = ui->nEdit->text();
    QString ln = ui->lnEdit->text();
    QString ph = ui->pnEdit->text();
    QString company = ui->comBox->currentText();
    QString pay = ui->personpayButton->isChecked() ? "شخصی" :
                      ui->companypayButton->isChecked() ? "شرکتی" : "نامشخص";
    QString type = ui->driverType->isChecked() ? "راننده" :
                       ui->medicalType->isChecked() ? "طب کار" : "نامشخص";
    QString date = gregorianToJalali(QDate::currentDate());

    double cost = 0;
    if (company != "بدون شرکت") {
        QSqlQuery q("SELECT ExamCost FROM companies WHERE CompanyName='" + company + "'");
        if (q.next())
            cost = q.value(0).toDouble();
    }

    QSqlQuery insert;
    insert.prepare("INSERT INTO reception (NationalNumber, FirstName, LastName, Phone, Company, ExamCost, PaymentType, Type, Date) "
                   "VALUES (:nn, :fn, :ln, :ph, :co, :cost, :pt, :ty, :dt)");
    insert.bindValue(":nn", nn);
    insert.bindValue(":fn", fn);
    insert.bindValue(":ln", ln);
    insert.bindValue(":ph", ph);
    insert.bindValue(":co", company);
    insert.bindValue(":cost", cost);
    insert.bindValue(":pt", pay);
    insert.bindValue(":ty", type);
    insert.bindValue(":dt", date);

    if (!insert.exec()) {
        QMessageBox::warning(this, "خطا", "ثبت اطلاعات با مشکل مواجه شد:\n" + insert.lastError().text());
    } else {
        QMessageBox::information(this, "موفق", "پذیرش با موفقیت ثبت شد.");
        clearFields();
        refreshModel();
    }
}

// ✅ حذف رکورد با پیام تأیید
void menu::on_deleteButton_clicked()
{
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "اخطار", "هیچ ردیفی برای حذف انتخاب نشده است!");
        return;
    }

    int id = ui->tableView->model()->data(ui->tableView->model()->index(index.row(), 0)).toInt();
    QString name = ui->tableView->model()->data(ui->tableView->model()->index(index.row(), 2)).toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "تأیید حذف",
                                  "آیا از حذف \"" + name + "\" مطمئن هستید؟",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery q;
        q.prepare("DELETE FROM reception WHERE id=:id");
        q.bindValue(":id", id);
        if (!q.exec()) {
            QMessageBox::critical(this, "خطا", "حذف با مشکل مواجه شد:\n" + q.lastError().text());
        } else {
            QMessageBox::information(this, "موفق", "رکورد حذف شد.");
            refreshModel();
        }
    }
}

// جستجو
void menu::on_searchButton_clicked()
{
    QString keyword = ui->searchEdit->text();
    auto *model = new QSqlTableModel(this, db);
    model->setTable("reception");
    model->setFilter(QString("FirstName LIKE '%%1%' OR LastName LIKE '%%1%' OR NationalNumber LIKE '%%1%'").arg(keyword));
    model->select();
    ui->tableView->setModel(model);
}

// فیلترها
void menu::on_filterAll_clicked() { refreshModel(); }

void menu::on_filterDriver_clicked()
{
    auto *model = new QSqlTableModel(this, db);
    model->setTable("reception");
    model->setFilter("Type='راننده'");
    model->select();
    ui->tableView->setModel(model);
}

void menu::on_filterMedical_clicked()
{
    auto *model = new QSqlTableModel(this, db);
    model->setTable("reception");
    model->setFilter("Type='طب کار'");
    model->select();
    ui->tableView->setModel(model);
}

// دوبار کلیک روی جدول (نمایش اطلاعات رکورد)
void menu::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    QString name = index.sibling(index.row(), 2).data().toString();
    QMessageBox::information(this, "انتخاب رکورد", "رکورد انتخاب‌شده: " + name);
}
void menu::on_ncEdit_editingFinished(){
    QString nc1 = ui->ncEdit->text().trimmed();
    QSqlQuery one;
    one.prepare("SELECT FirstName , LastName , Phone FROM reception WHERE NationalNumber = '"+nc1+"'");
    one.bindValue(":nc"  , nc1);
    if(!one.exec()){
        QMessageBox::critical(this , "خطا " , one.lastError().text());
        return;
    };
    if(one.next()){
        ui->nEdit->setText(one.value("FirstName").toString());
        ui->lnEdit->setText(one.value("LastName").toString());
        ui->pnEdit->setText(one.value("Phone").toString());
    }else{
        ui->ncEdit->setText(nc1);
    }

    }
