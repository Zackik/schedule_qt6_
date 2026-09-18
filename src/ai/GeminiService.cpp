#include "src/ai/GeminiService.h"

#include <QRegularExpression>
#include <QUuid>

GeminiService::GeminiService(const QString &apiKey, QObject *parent)
    : QObject(parent), m_apiKey(apiKey) {
    m_networkManager = new QNetworkAccessManager(this);
}

void GeminiService::setApiKey(const QString &apiKey) {
    m_apiKey = apiKey;
}

void GeminiService::sendGeminiRequest(const QString &systemInstruction, const QString &prompt,
                                      std::function<void(bool success, QString result)> callback) {
    if (m_apiKey.isEmpty()) {
        qWarning() << "Gemini API Error: Missing API Key.";
        callback(false, "Vui lòng nhập API Key của Gemini trong phần Cài đặt.");
        return;
    }

    QString urlStr =
        "https://generativelanguage.googleapis.com/v1beta/models/"
        "gemini-3.5-flash-lite:generateContent";
    QUrl url(urlStr);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-goog-api-key", m_apiKey.toUtf8());

    QJsonObject rootObj;

    // System instruction nếu có
    if (!systemInstruction.isEmpty()) {
        QJsonObject sysObj;
        QJsonObject sysPart;
        sysPart["text"] = systemInstruction;
        QJsonArray sysParts;
        sysParts.append(sysPart);
        sysObj["parts"] = sysParts;
        rootObj["systemInstruction"] = sysObj;
    }

    // Contents
    QJsonArray contentsArray;
    QJsonObject contentObj;
    QJsonArray partsArray;
    QJsonObject partObj;
    partObj["text"] = prompt;
    partsArray.append(partObj);
    contentObj["parts"] = partsArray;
    contentsArray.append(contentObj);
    rootObj["contents"] = contentsArray;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(rootObj).toJson());

    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            try {
                QJsonObject root = jsonDoc.object();
                QJsonArray candidates = root["candidates"].toArray();
                if (!candidates.isEmpty()) {
                    QJsonObject candObj = candidates[0].toObject();
                    QJsonObject content = candObj["content"].toObject();
                    QJsonArray parts = content["parts"].toArray();
                    if (!parts.isEmpty()) {
                        QString resultText = parts[0].toObject()["text"].toString();
                        callback(true, resultText);
                        reply->deleteLater();
                        return;
                    }
                }
                qWarning() << "Gemini API Error: No valid content found in response.";
                callback(false, "Không tìm thấy nội dung phản hồi từ Gemini.");
            } catch (...) {
                qWarning() << "Gemini API Error: Failed to parse JSON.";
                callback(false, "Lỗi phân tích JSON từ AI.");
            }
        } else {
            qWarning() << "Gemini API Connection Error:" << reply->errorString();
            callback(false, "Lỗi kết nối Gemini API: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

void GeminiService::askGemini(const QString &prompt,
                              std::function<void(bool success, QString response)> callback) {
    QString sys =
        "Bạn là trợ lý quản lý lịch trình thông minh tích hợp trên Qt6. Hãy trả lời ngắn gọn, súc "
        "tích, định dạng markdown đẹp và hữu ích bằng tiếng Việt.";
    sendGeminiRequest(sys, prompt, callback);
}

void GeminiService::parseNaturalLanguageSchedule(
    const QString &userText, const QDateTime &referenceDateTime,
    std::function<void(bool success, QList<ScheduleEvent> events, QString explanation)> callback) {
    QString sys =
        "Bạn là AI chuyên phân tích ngôn ngữ tự nhiên thành lịch trình chi tiết theo định dạng "
        "JSON.\n"
        "Thời gian hiện tại tham chiếu là: " +
        referenceDateTime.toString("yyyy-MM-dd HH:mm:ss") + " (Thứ " +
        QString::number(referenceDateTime.date().dayOfWeek() + 1) +
        ").\n"
        "Hãy trích xuất tất cả các sự kiện và trả về JSON thuần túy (không dùng markdown block "
        "hoặc ```json):\n"
        "{\n"
        "  \"explanation\": \"Giải thích ngắn về các sự kiện được trích xuất\",\n"
        "  \"events\": [\n"
        "    {\n"
        "      \"title\": \"Tên sự kiện\",\n"
        "      \"description\": \"Mô tả chi tiết\",\n"
        "      \"location\": \"Địa điểm nếu có\",\n"
        "      \"startTime\": \"YYYY-MM-DDTHH:MM:SS\",\n"
        "      \"endTime\": \"YYYY-MM-DDTHH:MM:SS\",\n"
        "      \"category\": \"Study|Work|Personal|Health|Other\",\n"
        "      \"priority\": \"Low|Medium|High\",\n"
        "      \"cost\": 0\n"
        "    }\n"
        "  ]\n"
        "}";

    sendGeminiRequest(sys, "Hãy trích xuất sự kiện từ văn bản sau: " + userText,
                      [callback](bool success, QString result) {
                          if (!success) {
                              callback(false, {}, result);
                              return;
                          }

                          // Dọn dẹp markdown nếu AI lỡ bọc trong ```json ... ```
                          QString cleaned = result.trimmed();
                          if (cleaned.startsWith("```json")) {
                              cleaned.remove(0, 7);
                          } else if (cleaned.startsWith("```")) {
                              cleaned.remove(0, 3);
                          }
                          if (cleaned.endsWith("```")) {
                              cleaned.chop(3);
                          }
                          cleaned = cleaned.trimmed();

                          QJsonDocument doc = QJsonDocument::fromJson(cleaned.toUtf8());
                          if (!doc.isObject()) {
                              callback(false, {},
                                       "Không thể phân tích dữ liệu JSON từ AI: " + result);
                              return;
                          }

                          QJsonObject root = doc.object();
                          QString explanation = root["explanation"].toString();
                          QJsonArray eventsArray = root["events"].toArray();

                          QList<ScheduleEvent> eventsList;
                          for (const auto &val : eventsArray) {
                              eventsList.append(ScheduleEvent::fromJson(val.toObject()));
                          }

                          callback(true, eventsList, explanation);
                      });
}

void GeminiService::generateDailyBriefing(
    const QList<ScheduleEvent> &events, const QDate &targetDate,
    std::function<void(bool success, QString briefing)> callback) {
    QString eventsText;
    if (events.isEmpty()) {
        eventsText = "Không có sự kiện nào được ghi nhận cho ngày này.";
    } else {
        for (const auto &e : events) {
            eventsText += QString(
                              "- [%1 -> %2] %3 (Phân loại: %4, Mức ưu tiên: %5, Chi phí: %6 VNĐ, "
                              "Địa điểm: %7)\n")
                              .arg(e.startTime.toString("HH:mm"))
                              .arg(e.endTime.toString("HH:mm"))
                              .arg(e.title)
                              .arg(CalendarManager::categoryToString(e.category))
                              .arg(CalendarManager::priorityToString(e.priority))
                              .arg(e.cost)
                              .arg(e.location.isEmpty() ? "Không có" : e.location);
        }
    }

    QString prompt =
        QString(
            "Hãy tạo một bản tóm tắt điều hành lịch trình (Daily Executive Briefing) chuyên nghiệp "
            "và truyền cảm hứng cho ngày %1:\n\n"
            "Danh sách sự kiện:\n%2\n\n"
            "Bản tóm tắt cần có:\n"
            "1. 🎯 **Mục tiêu & Điểm nóng trọng tâm trong ngày**\n"
            "2. ⏱️ **Phân bổ thời gian & Khung giờ tập trung Deep Work (Pomodoro)**\n"
            "3. ⚠️ **Cảnh báo xung đột hoặc khoảng cách di chuyển giữa các sự "
            "kiện**\n"
            "4. 💡 **Lời khuyên quản trị năng lượng & chi phí**")
            .arg(targetDate.toString("dd/MM/yyyy"))
            .arg(eventsText);

    sendGeminiRequest(
        "Bạn là chuyên gia cố vấn năng suất cá nhân hàng đầu. Trả lời bằng tiếng Việt chuyên "
        "nghiệp, tinh tế và truyền cảm hứng.",
        prompt, callback);
}

void GeminiService::optimizeSchedule(const QList<ScheduleEvent> &events, const QDate &targetDate,
                                     std::function<void(bool success, QString advice)> callback) {
    QString eventsText;
    for (const auto &e : events) {
        eventsText += QString("- %1 (%2 - %3)\n")
                          .arg(e.title)
                          .arg(e.startTime.toString("HH:mm"))
                          .arg(e.endTime.toString("HH:mm"));
    }

    QString prompt =
        QString(
            "Dựa vào lịch trình ngày %1 sau đây:\n%2\n"
            "Hãy đưa ra đề xuất tối ưu hóa:\n"
            "- Các khoảng trống lý tưởng để chèn phiên Pomodoro 25 phút.\n"
            "- Cách phân chia công việc theo ma trận Eisenhower (Khẩn cấp / Quan trọng).\n"
            "- Đề xuất giảm bớt chi phí hoặc gom các công việc gần nhau.")
            .arg(targetDate.toString("dd/MM/yyyy"))
            .arg(eventsText);

    sendGeminiRequest("Bạn là trợ lý tối ưu hóa thời gian và năng suất làm việc.", prompt,
                      callback);
}
