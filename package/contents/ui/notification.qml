import QtQuick 2.0
import QtQuick.Controls 2.5 as QQC2
import org.kde.kirigami 2.4 as Kirigami

Kirigami.FormLayout {
    id: generalPage

    Text {
        id: notificationText
        text: "This widget is only for Plasma 6 and is not available for Plasma 5."
        color: "red"


    Text {
        id: notificationH2
        text: "If you encounter any bugs, feel free to let me know. There are two ways to reset this widget:\n"
    }

    Text {
        id: notificationText3
        text: "         1. In terminal, enter: plasmashell --replace"
    }

    Text {
        id: notificationText4
        text: "         2. Remove this widget from your panel and add it back."
    }
}
