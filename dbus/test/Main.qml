import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.github.yigekuyou.lyrics

Window {
    visible: true
    width: 450
    height: 550
    title: "MPRIS Multi-Test"

    // 实例 1：用于测试 identity 动态查找 (findAndGetAsText)
    Mpris {
        id: mprisByIdentity
    }

    // 实例 2：用于测试指定 serviceName 直连 (connectToServiceByName / serviceName 属性)
    Mpris {
        id: mprisByServiceName
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        // ==================== 测试方法一：Identity 动态查找 ====================
        Label {
            text: "测试方法一：通过 Identity 查找 (findAndGetAsText)"
            font.bold: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: identityInput
                Layout.fillWidth: true
                placeholderText: "输入播放器标识 (例如: vlc, spotify)"
                text: "vil"
                onAccepted: findByIdentityBtn.clicked()
            }

            Button {
                id: findByIdentityBtn
                text: "查找"
                onClicked: {
                    console.log("方法一: 调用 findAndGetAsText, identity =", identityInput.text);
                    mprisByIdentity.findAndGetAsText(identityInput.text);
                }
            }
        }

        Label {
            text: "方法一歌词输出:"
            font.italic: true
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            background: Rectangle {
                color: "#f0f0f0"
                border.color: "#ccc"
            }

            TextArea {
                readOnly: true
                text: mprisByIdentity.asText
                wrapMode: TextArea.Wrap
            }
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        // ==================== 测试方法二：Service Name 直连 ====================
        Label {
            text: "测试方法二：通过完整服务名直连 (serviceName)"
            font.bold: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: serviceNameInput
                Layout.fillWidth: true
                placeholderText: "完整 D-Bus 服务名 (例如: org.mpris.MediaPlayer2.vlc.instance1)"
                onAccepted: connectByServiceBtn.clicked()
            }

            Button {
                id: connectByServiceBtn
                text: "直连"
                onClicked: {
                    console.log("方法二: 调用 connectToServiceByName, serviceName =", serviceNameInput.text);
                    mprisByServiceName.connectToServiceByName(serviceNameInput.text);
                }
            }
        }

        Label {
            text: "方法二歌词输出:"
            font.italic: true
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            background: Rectangle {
                color: "#f0f0f0"
                border.color: "#ccc"
            }

            TextArea {
                readOnly: true
                text: mprisByServiceName.asText
                wrapMode: TextArea.Wrap
            }
        }
    }
}
