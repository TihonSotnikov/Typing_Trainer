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
            blockCursor.visible = false
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
        
        onCurrentItemChanged: {
            if (currentItem) {
                currentItem.forceActiveFocus();
            }
        }
    }


    FocusScope {
        id: mainScreen
        visible: true

        Column {
            anchors.centerIn: parent
            spacing: 30
            width: parent.width * 0.8 // Ограничиваем ширину для демонстрации переноса строк

            // Блок текста с цветным курсором
            Item {
                id: textContainer
                width: parent.width
                height: 200
                clip: true // Ограничиваем видимую область высотой контейнера

                property bool isEditing: false

                // Утилита для получения ширины символов шрифта
                FontMetrics {
                    id: fontMetrics
                    font: textDisplay.font
                }

                // Область просмотра для длинного текста
                Flickable {
                    id: textFlickable
                    anchors.fill: parent
                    visible: !textContainer.isEditing
                    
                    contentWidth: width
                    contentHeight: textDisplay.contentHeight // Высота контента равна реальной высоте текста
                    interactive: false // Отключаем ручной скролл, чтобы фокус ввода не сбивался

                    // Автоматически рассчитываем координату Y, удерживая курсор по центру экрана
                    property real targetContentY: {
                        if (!blockCursor.visible) return 0;
                        
                        let curY = textDisplay.cursorRectangle.y;
                        let curH = textDisplay.cursorRectangle.height || fontMetrics.height;
                        
                        // Вычисляем позицию, при которой курсор окажется строго по центру высоты Flickable
                        let target = curY - (height / 2) + (curH / 2);
                        
                        // Ограничиваем прокрутку, чтобы не выходить за рамки текста
                        let maxScroll = Math.max(0, contentHeight - height);
                        return Math.max(0, Math.min(target, maxScroll));
                    }

                    contentY: targetContentY

                    // Плавный переход при изменении положения прокрутки
                    Behavior on contentY {
                        NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
                    }

                    TextEdit {
                        id: textDisplay
                        width: parent.width
                        height: contentHeight // Привязываем высоту к внутреннему контенту, чтобы Flickable знал реальный размер

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

                    // Цветной курсор (теперь находится внутри Flickable, чтобы двигаться вместе с текстом)
                    Rectangle {
                        id: blockCursor
                        color: "#4CAF50"
                        opacity: 0.4
                        radius: 4
                        visible: false

                        // Привязываем позицию к системному курсору внутри TextEdit
                        x: textDisplay.cursorRectangle.x
                        y: textDisplay.cursorRectangle.y

                        // Высота равна высоте строки, а ширина - средней ширине символа
                        width: fontMetrics.averageCharacterWidth
                        height: textDisplay.cursorRectangle.height || fontMetrics.height

                        // Плавная анимация перемещения курсора
                        Behavior on x {
                            NumberAnimation { duration: 80; easing.type: Easing.OutQuad }
                        }
                        Behavior on y {
                            NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
                        }
                    }
                }

                Loader {
                    id: editorLoader
                    anchors.fill: parent
                    active: textContainer.isEditing    // Создается в памяти только при редактировании
                    visible: textContainer.isEditing   // Отображается только при редактировании

                    sourceComponent: ScrollView {
                        // Псевдоним для доступа к тексту извне через editorLoader.item.text
                        property alias text: editingTextArea.text
                        anchors.fill: parent

                        TextArea {
                            id: editingTextArea
                            text: trainer.textToType 
                            textFormat: TextEdit.PlainText

                            font.pointSize: 20
                            font.family: "Courier New"
                            wrapMode: TextArea.WordWrap
                            
                            readOnly: false
                            selectByMouse: true
                            cursorVisible: true
                            activeFocusOnPress: true

                            Component.onCompleted: forceActiveFocus()
                        }
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
                    id: editButton
                    enabled: trainer.sessionStatus === "inactive" || trainer.sessionStatus === "completed"
                    text: "Редактировать"
                    onClicked: {
                        if (!textContainer.isEditing) {
                            textContainer.isEditing = true
                            editButton.text = "Готово"
                        } else {
                            trainer.uploadCustomText(editorLoader.item.text)
                            textContainer.isEditing = false
                            editButton.text = "Редактировать"
                        }
                    }
                }

                XButton {
                    id: startButton
                    enabled: !textContainer.isEditing
                    text: trainer.sessionStatus === "active" || trainer.sessionStatus === "paused"
                        ? "Завершить"
                        : "Старт"
                    onClicked: {
                        if (trainer.sessionStatus === "active") {
                            trainer.stopSession()
                        } else {
                            statusLabel.text = "Печатаем..."
                            trainer.startFreeSession()
                            blockCursor.visible = true
                            trainerInputField.forceActiveFocus()
                        }
                    }
                }
            }

            Label {
                text: `Status: ${trainer.sessionStatus}`
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

        Item {
            id: trainerInputField
            focus: true
            
            onActiveFocusChanged: {
                let isCurrentScreen = (stackView.currentItem === mainScreen);
                let isSessionRunning = (trainer.sessionStatus === "active" || trainer.sessionStatus === "paused");

                if (!activeFocus && isSessionRunning && isCurrentScreen && !textContainer.isEditing) {
                    trainerInputField.forceActiveFocus();
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


    FocusScope {
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

            Frame {
                padding: 20
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.implicitWidth

                Row {
                    spacing: 5
                    anchors.horizontalCenter: parent.horizontalCenter

                    XSegmentedControl {
                        id: metricControl
                        height: 45
                        items: ["WPM", "CPM"]
                        currentIndex: 0
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

        Item {
            id: settingsInputField
            anchors.fill: parent
            focus: true
            
            onActiveFocusChanged: {
                let isCurrentScreen = (stackView.currentItem === settingsScreen);

                if (!activeFocus && isCurrentScreen) {
                    settingsInputField.forceActiveFocus();
                }
            }

            Keys.onPressed: (event) => {
                if (event.key === Qt.Key_Escape) {
                    stackView.pop();
                }
                event.accepted = true;
            }
        }
    }

}