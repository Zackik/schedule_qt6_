#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QString s1 = "2026-09-13T10:00:00Z";
    QString s2 = "2026-09-13T10:00:00+07:00";
    QString s3 = "2026-09-13";
    qDebug() << "s1:" << QDateTime::fromString(s1, Qt::ISODate) << QDateTime::fromString(s1, Qt::ISODate).isValid();
    qDebug() << "s2:" << QDateTime::fromString(s2, Qt::ISODate) << QDateTime::fromString(s2, Qt::ISODate).isValid();
    qDebug() << "s3:" << QDateTime::fromString(s3, Qt::ISODate) << QDateTime::fromString(s3, Qt::ISODate).isValid();
    return 0;
}
