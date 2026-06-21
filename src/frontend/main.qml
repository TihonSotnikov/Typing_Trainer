import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import BlindTypingTrainerModule 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    minimumWidth: 600
    minimumHeight: 400
    title: "Typing Trainer"

    // palette.window: Theme.background     // Фоновый цвет окон и подложек
    // palette.windowText: Theme.text       // Цвет текста для всех Label по умолчанию
    // palette.base: Theme.surface          // Фон полей ввода (TextField, TextArea)
    // palette.text: Theme.text             // Цвет текста внутри полей ввода
    // palette.buttonText: Theme.accent     // Цвет текста/иконок на кнопках

    Material.theme: Theme.isDark ? Material.Dark : Material.Light
    Material.accent: Theme.accent
    Material.primary: Theme.accent
    Material.background: Theme.surface

    TypingTrainerCore {
        id: trainer
        onSessionCompleted: {
            statusLabel.text = "Session Completed!"
        }
        onSessionStatusChanged: {
            if (trainer.sessionStatus === "paused") {
                statusLabel.text = "Session Paused"
                pauseFade.visible = true
            } else if (trainer.sessionStatus === "active") {
                statusLabel.text = "Typing..."
                pauseFade.visible = false
            }
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainScreen

        pushEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 250 // Длительность в миллисекундах
            }
        }
        pushExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 250
            }
        }
        popEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 250
            }
        }
        popExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 250
            }
        }
    }


    Item {
        id: mainScreen
        visible: true

        Column {
            anchors.centerIn: parent
            spacing: 30
            width: parent.width * 0.8 // Ограничиваем ширину для демонстрации переноса строк

            // Блок текста с цветным курсором
            Item {
                width: parent.width
                height: 200

                // Утилита для получения ширины символов шрифта
                FontMetrics {
                    id: fontMetrics
                    font: textDisplay.font
                }

                TextEdit {
                    id: textDisplay
                    anchors.fill: parent
                    text: trainer.formattedText
                    textFormat: TextEdit.RichText

                    font.pointSize: 24
                    font.family: "Courier New"
                    wrapMode: TextEdit.WordWrap
                    readOnly: true
                    selectByMouse: false
                    cursorPosition: trainer.cursorPosition
                    cursorVisible: false
                    activeFocusOnPress: false
                }

                // Цветной курсор
                Rectangle {
                    id: blockCursor
                    color: "#4CAF50"
                    opacity: 0.4
                    radius: 4

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

            // Вывод метрик
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
                text: "Нажмите 'Старт' чтобы начать"
                font.pointSize: 12
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Row {
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter

                XButton {
                    text: "Настройки"
                    onClicked: {
                        stackView.push(settingsScreen)
                    }
                }

                XButton {
                    text: "Старт"
                    onClicked: {
                        statusLabel.text = "Печатаем..."
                        // Длинный текст для проверки переноса строк и работы курсора по Y
                        trainer.startSession("This is a long example text for training your blind typing skills. The cursor will smoothly follow your progress and jump to the next line automatically.")
                        inputField.forceActiveFocus()
                    }
                }
            }

            Label {
                text: `Debug: ${trainer.sessionStatus}`
                font.pointSize: 10
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        Rectangle {
            id: pauseFade
            visible: false
            anchors.fill: parent

            color: "#ffffff"
            opacity: 0.6

            Label {
                id: pauseLabel
                anchors.centerIn: parent

                text: "Пауза"
                font.pointSize: 36
                font.bold: true
                color: "#1a1a1a"
            }
        }
    }


    Item {
        id: settingsScreen
        visible: false

        Column {
            anchors.centerIn: parent
            spacing: 20

            Label {
                text: "Настройки"
                font.pointSize: 24
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // Настройки BEGIN

            Frame {
                padding: 20

                Column {
                    Label {
                        text: "Темы"
                        font.pixelSize: 24
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Row {
                        spacing: 10
                        anchors.horizontalCenter: parent.horizontalCenter

                        XButton {
                            text: "Светлая"
                            onClicked: Theme.currentTheme = "light"
                        }
                        XButton {
                            text: "Тёмная"
                            onClicked: Theme.currentTheme = "dark"
                        }
                        XButton {
                            text: "Чёрная"
                            onClicked: Theme.currentTheme = "black"
                        }
                    }
                }
            }

            // Настройки END

            XButton {
                text: "Готово"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    stackView.pop()
                }
            }
        }
    }

    // 3. Невидимое поле для перехвата клавиатуры
    Item {
        id: inputField
        focus: true
        
        onActiveFocusChanged: {
            if (!activeFocus && (trainer.sessionStatus === "active" || trainer.sessionStatus === "paused")) {
                inputField.forceActiveFocus();
            }
        }

        Keys.onPressed: (event) => {
            if (trainer.sessionStatus !== "active" && trainer.sessionStatus !== "paused") {
                return;
            }

            if (event.key === Qt.Key_Backspace) {
                if (trainer.sessionStatus === "active") {
                    trainer.sendBackspace();
                }
                event.accepted = true;
            } else if (event.key === Qt.Key_Escape) {
                if (trainer.sessionStatus === "active") {
                    trainer.sendEscape();
                } else if (trainer.sessionStatus === "paused") {
                    trainer.resumeSession();
                }
                event.accepted = true;
            } else if (event.text.length > 0) {
                if (trainer.sessionStatus === "active") {
                    // Превращаем \r в \n на стороне фронта
                    let charToSend = event.text === "\r" ? "\n" : event.text;
                    trainer.sendKeyPress(charToSend);
                }
                event.accepted = true;
            }
        }
    }

}