import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import TypingTrainerModule

Button {
    font.family: "Tahoma"
    font.pixelSize: 18
    font.bold: true
    font.letterSpacing: 1.2
    highlighted: hovered || pressed
}