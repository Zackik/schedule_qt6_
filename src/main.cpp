#include <QApplication>
#include <QProcessEnvironment>
#include <QStyleFactory>

#include "src/ai/GeminiService.h"
#include "src/calendar/CalendarManager.h"
#include "src/services/companion/AssistantService.h"
#include "src/ui/ScheduleWidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Smart Schedule & Google Calendar Manager");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("AIStudio");

    // Áp dụng Fusion style để có giao diện hiện đại đồng nhất trên mọi hệ điều hành
    app.setStyle(QStyleFactory::create("Fusion"));

    CalendarManager manager;

    // Lấy API key từ biến môi trường nếu có
    QString apiKey = QProcessEnvironment::systemEnvironment().value("GEMINI_API_KEY", "");
    GeminiService gemini(apiKey);

    ScheduleWidget window(&manager, &gemini);
    window.setWindowTitle("Smart Schedule & Google Calendar Manager (Qt6 + Gemini AI)");
    window.resize(1100, 750);
    window.show();

    // Khởi tạo AI Desktop Companion (Floating Chibi Mascot)
    AssistantService *companion = new AssistantService(&app);
    companion->start();

    return app.exec();
}
