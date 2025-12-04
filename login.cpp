#include "login.h"
#include "ui_login.h"
#include "menu.h"
#include <QMessageBox>

login::login(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::login)
{
    ui->setupUi(this);

    // placeholder texts and password echo
    ui->username_edit->setPlaceholderText(QStringLiteral("نام کاربری"));
    ui->password_edit->setPlaceholderText(QStringLiteral("رمز عبور"));
    ui->password_edit->setEchoMode(QLineEdit::Password);

    connect(ui->login_button, &QPushButton::clicked,
            this, &login::on_login_button_clicked);
}

login::~login()
{
    delete ui;
}

void login::on_login_button_clicked()
{
    QString username = ui->username_edit->text().trimmed();
    QString password = ui->password_edit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("هشدار"), QStringLiteral("لطفاً نام کاربری و رمز را وارد کنید."));
        return;
    }

    if (username == "admin" && password == "1234") {
        menu *m = new menu();
        m->show();
        this->close();
    } else {
        QMessageBox::warning(this, QStringLiteral("خطا"), QStringLiteral("نام کاربری یا رمز عبور اشتباه است!"));
    }
}