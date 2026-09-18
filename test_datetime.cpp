#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    qDebug() << QDateTime::currentDateTime().toString(Qt::ISODate);
    return 0;
}
