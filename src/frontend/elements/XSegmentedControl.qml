import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Control {
    id: root

    // --- Открытый интерфейс ---
    property var items: []
    property int currentIndex: 0
    readonly property string currentText: items[currentIndex] || ""

    property color backgroundColor: root.Material.theme === Material.Dark 
        ? Qt.rgba(255, 255, 255, 0.08) 
        : Qt.rgba(0, 0, 0, 0.06)

    property color activeColor: root.Material.theme === Material.Dark 
        ? Qt.rgba(255, 255, 255, 0.12) 
        : "#ffffff"

    property color activeTextColor: root.Material.theme === Material.Dark
        ? Qt.lighter(root.Material.accent, 2.0)
        : root.Material.accent

    property color inactiveTextColor: root.Material.theme === Material.Dark 
        ? "#aaaaaa" 
        : "#666666"
    property int animationDuration: 180

    // Ссылка на активный элемент в репитере
    readonly property Item activeItem: repeater.count > 0 && currentIndex >= 0 && currentIndex < repeater.count 
        ? repeater.itemAt(currentIndex) 
        : null

    implicitWidth: layoutRow.width + padding * 2
    implicitHeight: 40
    padding: 4

    Rectangle {
        id: bgRect
        anchors.fill: parent
        radius: height / 2
        color: root.backgroundColor

        // Движущийся ползунок-выделитель
        Rectangle {
            id: selector
            width: activeItem ? activeItem.width : 0
            height: parent.height - root.padding * 2
            radius: height / 2
            color: root.activeColor
            y: root.padding
            x: root.padding + (activeItem ? activeItem.x : 0)

            // Анимируем одновременно и перемещение, и изменение ширины ползунка
            Behavior on x {
                NumberAnimation {
                    duration: root.animationDuration
                    easing.type: Easing.OutCubic
                }
            }
            Behavior on width {
                NumberAnimation {
                    duration: root.animationDuration
                    easing.type: Easing.OutCubic
                }
            }

            // Мягкая рамка-тень (светлая для темной темы, темная для светлой)
            layer.enabled: true
            border.color: root.Material.theme === Material.Dark 
                ? Qt.rgba(255, 255, 255, 0.05) 
                : Qt.rgba(0, 0, 0, 0.08)
            border.width: 1
        }

        // Ряд сегментов разной ширины
        Row {
            id: layoutRow
            anchors.verticalCenter: parent.verticalCenter
            x: root.padding
            spacing: 0

            Repeater {
                id: repeater
                model: root.items

                delegate: MouseArea {
                    id: itemMouseArea
                    
                    width: textElement.contentWidth + 32
                    height: bgRect.height - root.padding * 2
                    hoverEnabled: true

                    Text {
                        id: textElement
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 13
                        font.bold: root.currentIndex === index
                        
                        // Цвет текста меняется при наведении и переключении
                        color: {
                            if (root.currentIndex === index) return root.activeTextColor;
                            if (itemMouseArea.containsMouse) {
                                // При наведении делаем серый текст светлее в темной теме, и темнее в светлой
                                return root.Material.theme === Material.Dark 
                                    ? Qt.lighter(root.inactiveTextColor, 1.25) 
                                    : Qt.darker(root.inactiveTextColor, 1.25);
                            }
                            return root.inactiveTextColor;
                        }

                        Behavior on color { ColorAnimation { duration: 120 } }
                        
                        scale: root.currentIndex === index ? 1.04 : 1.0
                        Behavior on scale { NumberAnimation { duration: 120 } }
                    }

                    onClicked: {
                        root.currentIndex = index;
                    }
                }
            }
        }
    }
}