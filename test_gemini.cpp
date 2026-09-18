#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QString urlStr = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent";
    QUrl url(urlStr);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-goog-api-key", "FAKE_KEY");
    
    std::cout << "URL: " << request.url().toString().toStdString() << std::endl;
    std::cout << "Header: x-goog-api-key = " << request.rawHeader("x-goog-api-key").toStdString() << std::endl;
    return 0;
}
