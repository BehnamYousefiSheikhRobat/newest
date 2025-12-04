#include "filterresult.h"
#include "ui_filterresult.h"
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QTextDocument>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QtPrintSupport/QPrinter>
#include <QTextDocument>
#include <QMessageBox>


filterresult::filterresult(QSqlTableModel *model, QWidget *parent)
    : QMainWindow(parent),    // 👈 چون از QMainWindow ارث می‌بری
    ui(new Ui::filterresult),
    tableModel(model)
{
    ui->setupUi(this);
    ui->tableView->setModel(tableModel);
    ui->tableView->resizeColumnsToContents();
}

filterresult::~filterresult()
{
    delete ui;
}


void filterresult::on_printButton_clicked()
{
    // پرسیدن مسیر ذخیره PDF
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "ذخیره به‌عنوان PDF",
        QDir::homePath() + "/گزارش_پذیرش‌ها.pdf",
        "PDF Files (*.pdf)"
        );

    if (filePath.isEmpty())
        return; // کاربر لغو کرد

    if (!filePath.endsWith(".pdf", Qt::CaseInsensitive))
        filePath += ".pdf";

    // تنظیم چاپگر برای خروجی PDF
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);

    // ساخت HTML از جدول
    QString html;
    html += "<html><head><meta charset='UTF-8'></head><body>";
    html += "<h3 align='center'>گزارش پذیرش‌ها</h3>";
    html += "<table border='1' cellspacing='0' cellpadding='4' align='center'>";
    html += "<tr>";
    for (int c = 0; c < tableModel->columnCount(); ++c)
        html += "<th>" + tableModel->headerData(c, Qt::Horizontal).toString() + "</th>";
    html += "</tr>";

    for (int r = 0; r < tableModel->rowCount(); ++r) {
        html += "<tr>";
        for (int c = 0; c < tableModel->columnCount(); ++c)
            html += "<td>" + tableModel->data(tableModel->index(r, c)).toString() + "</td>";
        html += "</tr>";
    }
    html += "</table></body></html>";

    // چاپ به PDF
    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "ذخیره شد", "فایل PDF با موفقیت ساخته شد:\n" + filePath);

    // باز کردن فایل PDF بعد از ساخت
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

