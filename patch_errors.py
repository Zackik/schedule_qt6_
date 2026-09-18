import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

# Replace all emit googleSyncFinished(false, ...) to include response body
def repl(m):
    return """
            QByteArray errBody = reply->readAll();
            qWarning() << "Google API Error:" << reply->errorString() << errBody;
            QString details = reply->errorString();
            QJsonDocument errDoc = QJsonDocument::fromJson(errBody);
            if (!errDoc.isNull() && errDoc.object().contains("error")) {
                 QJsonObject errObj = errDoc.object()["error"].toObject();
                 details += " - " + errObj["message"].toString();
            }
            emit googleSyncFinished(false, "%s" + details);
""" % m.group(1)

content = re.sub(r'emit googleSyncFinished\(false,\s*"([^"]+)"\s*\+\s*reply->errorString\(\)\);', repl, content)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
