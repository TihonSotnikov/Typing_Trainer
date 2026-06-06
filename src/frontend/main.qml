import QtQuick
import QtQuick.Controls
import BlindTypingTrainerModule 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: "Typing Trainer"

    TypingTrainerCore {
        id: trainer
        onSessionCompleted: {
            statusLabel.text = "Session Completed!"
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 30
        width: parent.width * 0.8 // Ограничиваем ширину для демонстрации переноса строк

        // 1. Блок текста с цветным курсором
        Item {
            width: parent.width
            height: 200 // Резервируем высоту для многострочного текста

            // Утилита для получения ширины символов шрифта
            FontMetrics {
                id: fontMetrics
                font: textDisplay.font
            }

            TextEdit {
                id: textDisplay
                anchors.fill: parent
                text: trainer.textToType
                textFormat: TextEdit.RichText // <--- ДОБАВИТЬ ЭТУ СТРОКУ

                font.pointSize: 24
                font.family: "Courier New"
                wrapMode: TextEdit.WordWrap
                readOnly: true
                selectByMouse: false
                cursorPosition: trainer.cursorPosition
                cursorVisible: false 
            }

            // Наш кастомный цветной курсор
            Rectangle {
                id: blockCursor
                color: "#4CAF50" // Цвет курсора (Material Green)
                opacity: 0.4     // Полупрозрачный, чтобы видеть букву под ним
                radius: 4        // Слегка закругленные углы

                // Привязываем позицию к невидимому системному курсору внутри TextEdit
                x: textDisplay.cursorRectangle.x
                y: textDisplay.cursorRectangle.y
                
                // Высота равна высоте строки, а ширина - средней ширине символа
                width: fontMetrics.averageCharacterWidth
                height: textDisplay.cursorRectangle.height || fontMetrics.height

                // Добавляем плавную анимацию перемещения
                Behavior on x {
                    NumberAnimation { duration: 80; easing.type: Easing.OutQuad }
                }
                Behavior on y {
                    NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
                }
            }
        }

        // 2. Вывод метрик
        Row {
            spacing: 30
            anchors.horizontalCenter: parent.horizontalCenter
            
            Label { 
                text: "WPM: <font color='#2196F3'><b>" + trainer.wpm.toFixed(1) + "</b></font>" 
                font.pointSize: 14; textFormat: Text.RichText 
            }
            Label { 
                text: "Accuracy: <font color='#4CAF50'><b>" + trainer.accuracy.toFixed(1) + "%</b></font>" 
                font.pointSize: 14; textFormat: Text.RichText 
            }
            Label { 
                text: "Cursor: <b>" + trainer.cursorPosition + "</b>" 
                font.pointSize: 14; textFormat: Text.RichText 
            }
        }

        Label {
            id: statusLabel
            text: "Press 'Start' to begin"
            font.pointSize: 12
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Button {
            text: "Start"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                statusLabel.text = "Typing..."
                // Длинный текст для проверки переноса строк и работы курсора по Y
                trainer.startSession("This is a long example text for training your blind typing skills. The cursor will smoothly follow your progress and jump to the next line automatically.")
                inputField.forceActiveFocus()
            }
        }

        // 3. Невидимое поле для перехвата клавиатуры
        Item {
            id: inputField
            focus: true
            Keys.onPressed: (event) => {
                if (event.key === Qt.Key_Backspace) {
                    trainer.sendBackspace();
                    event.accepted = true;
                } else if (event.key === Qt.Key_Escape) {
                    trainer.stopSession();
                    statusLabel.text = "Session Stopped"
                    event.accepted = true;
                } else if (event.text.length > 0) {
                    // Превращаем \r в \n на стороне фронта
                    let charToSend = event.text === "\r" ? "\n" : event.text;
                    trainer.sendKeyPress(charToSend);
                    event.accepted = true;
                }
            }
        }
    }
}