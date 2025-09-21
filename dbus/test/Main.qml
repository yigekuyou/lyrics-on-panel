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

        Connections {
            target: mpris
            function onAsTextChanged() {
                // Log when asTextChanged signal is triggered
                //console.log("asTextChanged signal was triggered!");
                // Log the new value of asText
                //console.log("New asText value:", mpris.asText);
            }
        }

        Button {
            id: findButton
            text: "Find and Get asText"
            Layout.fillWidth: true
            onClicked: {
                // Log the function call and the identity being used
                //console.log("Calling findAndGetAsText with identity:", identityInput.text);
                mpris.findAndGetAsText(identityInput.text);
            }
        }

        Label {
            text: "Now playing:"
            font.bold: true
        }

        // Add a new Label to show status messages
        Label {
            id: statusLabel
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: {
                // Add debug logging to see which condition is met
                if (mpris.asText.length > 0) {
                    //console.log("Status: Lyrics found.");
                    return "";
                } else {
                    //console.log("Status: No lyrics for the current song.");
                    return "No lyrics for the current song.";
                }
            }
        }

        Label {
            id: asTextOutput
            text: mpris.asText
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

    }

}
