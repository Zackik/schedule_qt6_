import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: 800
    height: 600
    color: "#f8fafc" // Slate 50

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        Text {
            text: "Smart Schedule Overview"
            font.pixelSize: 32
            font.bold: true
            font.family: "Arial"
            color: "#0f172a" // Slate 900
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            // Metric Card 1
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                color: "#ffffff"
                radius: 12
                border.color: "#e2e8f0"
                border.width: 1

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8
                    Text { text: "Total Events"; color: "#64748b"; font.pixelSize: 14; Layout.alignment: Qt.AlignHCenter }
                    Text { id: txtTotal; text: "0"; color: "#3b82f6"; font.pixelSize: 36; font.bold: true; Layout.alignment: Qt.AlignHCenter }
                }
            }

            // Metric Card 2
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                color: "#ffffff"
                radius: 12
                border.color: "#e2e8f0"
                border.width: 1

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8
                    Text { text: "Completed"; color: "#64748b"; font.pixelSize: 14; Layout.alignment: Qt.AlignHCenter }
                    Text { id: txtCompleted; text: "0"; color: "#10b981"; font.pixelSize: 36; font.bold: true; Layout.alignment: Qt.AlignHCenter }
                }
            }

            // Metric Card 3
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                color: "#ffffff"
                radius: 12
                border.color: "#e2e8f0"
                border.width: 1

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8
                    Text { text: "Total Cost"; color: "#64748b"; font.pixelSize: 14; Layout.alignment: Qt.AlignHCenter }
                    Text { id: txtCost; text: "0 VNĐ"; color: "#f59e0b"; font.pixelSize: 24; font.bold: true; Layout.alignment: Qt.AlignHCenter }
                }
            }
        }

        // A placeholder for a modern chart or list
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#ffffff"
            radius: 12
            border.color: "#e2e8f0"
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: "✨ Modern QML Interface integrated ✨\n\n(Dữ liệu có thể được liên kết trực tiếp từ C++ qua Q_PROPERTY)"
                font.pixelSize: 18
                color: "#94a3b8"
                horizontalAlignment: Text.AlignHCenter
                lineHeight: 1.5
            }
        }
    }

    // This function can be called from C++ to update the UI
    function updateMetrics(total, completed, cost) {
        txtTotal.text = total.toString()
        txtCompleted.text = completed.toString()
        txtCost.text = cost.toString() + " VNĐ"
    }
}
