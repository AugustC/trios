// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import "components"

View {
    property int win_width: windowconfiguration.window_width
    property int win_height: windowconfiguration.window_height

    id: root

    function loadWindow(w_width, w_height, w_window) {
        win_width_text.text = w_width;
        win_height_text.text = w_height;

        var i, j, k = 0;
        for (i = 0; i < w_height; i++) {
            for (j = 0; j < w_width; j++) {
                windowconfiguration.set(i, j, w_window[k]);
            }
        }
    }

    function getWindowData() {
        var i, j;
        var m = []
        for (i = 0; i < win_height; i++) {
            for (j = 0; j < win_width; j++) {
                m.push(windowconfiguration.get(i, j));
            }
        }
        return m;
    }

    Text {
        id: text1
        text: qsTr("Window Configuration")
        anchors.top: parent.top
        anchors.topMargin: 0
        horizontalAlignment: Text.AlignHCenter
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 0
        font.pixelSize: 20
    }

    Row {
        id: row1
        x: 170
        y: 44
        width: 300
        height: 59
        spacing: 9
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: text1.bottom
        anchors.topMargin: 21

        Text {
            text: "Width"
        }

        TextInput {
            id: win_width_text
            width: 50
            height: 20
            color: "#00000000"
            text: "3"
            border.color: "#000000"
            onTextChanged: {
                var i = parseInt(text);
                if (i != NaN && i >= 0 && windowconfiguration != null) {
                    windowconfiguration.window_width = i;
                }
            }

        }

        Text {
            text: "Height"
        }

        TextInput {
            id: win_height_text
            width: 50
            height: 20
            color: "#00000000"
            text: "3"
            border.color: "#000000"
            onTextChanged: {
                var i = parseInt(text);
                if (i != NaN && i >= 0 && windowconfiguration != null) {
                    windowconfiguration.window_height = i;
                }
            }
        }
    }

    WindowConfiguration {
        id: windowconfiguration
        x: 156
        y: 123
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: text4
        x: 170
        y: 19
        text: qsTr("Window Size")
        anchors.left: row1.left
        anchors.leftMargin: 0
        anchors.top: row1.top
        anchors.topMargin: -25
        font.pixelSize: 12
    }
}
