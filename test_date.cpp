#include <QCoreApplication>
#include <QDateTime>
#include <QDate>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QString startStr = "2026-09-14";
    QDateTime dt = QDateTime::fromString(startStr, Qt::ISODate);
    QDate d = QDate::fromString(startStr, Qt::ISODate);
    std::cout << "dt valid: " << dt.isValid() << std::endl;
    std::cout << "d valid: " << d.isValid() << std::endl;
    return 0;
}
