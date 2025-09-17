import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import io.github.lyric

Window {
    width: 400
    height: 300
    visible: true
    title: "MPRIS Test"

    // Instantiate the Mpris C++ class as a QML element
    Mpris {
        id: mpris
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 10

        TextField {
            id: identityInput

            placeholderText: "Enter media player identity (e.g., vlc, mpv)"
            Layout.fillWidth: true
            onAccepted: findButton.clicked()
        }

        Button {
            id: findButton

            text: "Find and Get asText"
            Layout.fillWidth: true
            onClicked: {
                mpris.findAndGetAsText(identityInput.text);
            }
        }

        Label {
            text: "Now playing:"
            font.bold: true
        }

        Label {
            id: asTextOutput

            text: mpris.asText
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

    }

}
