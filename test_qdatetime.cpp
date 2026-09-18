#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    qDebug() << QDateTime::currentDateTime().toString(Qt::ISODate);
    qDebug() << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    return 0;
}
