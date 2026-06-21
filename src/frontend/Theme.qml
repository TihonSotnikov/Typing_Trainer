pragma Singleton
import QtQuick
import QtCore

Item {
    id: root

    // Доступные темы: "light", "dark", "blue"
    property string currentTheme: "light"

    // Вспомогательное свойство для быстрой проверки
    readonly property var darkThemes: ["dark", "black"]
    readonly property bool isDark: darkThemes.includes(currentTheme)

    // Описание цветов палитры в зависимости от выбранной темы
    readonly property color background: {
        switch (currentTheme) {
            case "dark": return "#1e1e1e"
            case "black": return "#000000"
            case "blue": return "#0a192f"
            default:     return "#f5f5f5" // "light"
        }
    }

    readonly property color surface: {
        switch (currentTheme) {
            case "dark": return "#2d2d2d"
            case "black": return "#1a1a1a"
            case "blue": return "#172a45"
            default:     return "#ffffff"
        }
    }

    readonly property color text: {
        switch (currentTheme) {
            case "dark": return "#ffffff"
            case "black": return "#f0f0f0"
            case "blue": return "#f8f9fa"
            default:     return "#2b2b2b"
        }
    }

    readonly property color accent: {
        switch (currentTheme) {
            case "dark": return '#006199'
            case "black": return '#6645a3'
            case "blue": return '#2e8d9e'
            default:     return '#3ea1e2'
        }
    }
    
    Settings {
        category: "Interface"
        property alias currentTheme: root.currentTheme
    }
}