import re

with open('src/ai/GeminiService.cpp', 'r') as f:
    content = f.read()

target = """    QString urlStr =
        "https://generativelanguage.googleapis.com/v1beta/models/"
        "gemini-2.5-flash:generateContent?key=" +
        m_apiKey;
    QUrl url(urlStr);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");"""

replacement = """    QString urlStr =
        "https://generativelanguage.googleapis.com/v1beta/models/"
        "gemini-2.5-flash:generateContent";
    QUrl url(urlStr);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-goog-api-key", m_apiKey.toUtf8());"""

if target in content:
    content = content.replace(target, replacement)
else:
    # Try a regex approach if exact match fails
    content = re.sub(
        r'    QString urlStr =\s*"https://generativelanguage\.googleapis\.com/v1beta/models/"\s*"gemini-[^:]+:generateContent\?key=" \+\s*m_apiKey;\s*QUrl url\(urlStr\);\s*QNetworkRequest request\(url\);\s*request\.setHeader\(QNetworkRequest::ContentTypeHeader, "application/json"\);',
        r'    QString urlStr =\n        "https://generativelanguage.googleapis.com/v1beta/models/"\n        "gemini-2.5-flash:generateContent";\n    QUrl url(urlStr);\n\n    QNetworkRequest request(url);\n    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");\n    request.setRawHeader("x-goog-api-key", m_apiKey.toUtf8());',
        content,
        flags=re.MULTILINE
    )

with open('src/ai/GeminiService.cpp', 'w') as f:
    f.write(content)
