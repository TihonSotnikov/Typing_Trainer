#include "main_window.h"
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // 1. Настройка главного виджета и Layout'а (компоновщика)
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // 2. Создание элементов
    helloLabel = new QLabel("Hello, World!", this);
    helloLabel->setAlignment(Qt::AlignCenter);

    closeButton = new QPushButton("Закрыть приложение", this);
    
    // Подключаем кнопку к функции закрытия окна
    connect(closeButton, &QPushButton::clicked, this, &QMainWindow::close);

    // 3. Добавляем элементы в Layout
    layout->addStretch(); // Пружина сверху
    layout->addWidget(helloLabel);
    layout->addSpacing(20); // Отступ между текстом и кнопкой
    layout->addWidget(closeButton, 0, Qt::AlignHCenter); // Центрируем кнопку
    layout->addStretch(); // Пружина снизу

    setCentralWidget(centralWidget);
    resize(400, 300);
    setWindowTitle("Custom UI Example");

    // ==========================================
    // 4. ИЗМЕНЕНИЕ ДИЗАЙНА (QSS - Qt Style Sheets)
    // ==========================================
    this->setStyleSheet(R"(
        /* Стиль для главного окна (темный фон) */
        QMainWindow {
            background-color: #2b2b2b;
        }

        /* Стиль для текста */
        QLabel {
            color: #00ffcc; /* Неоновый бирюзовый цвет */
            font-size: 36px;
            font-weight: bold;
            font-family: 'Segoe UI', sans-serif;
        }

        /* Стиль для кнопки в обычном состоянии */
        QPushButton {
            background-color: #3c3f41;
            color: #ffffff;
            border: 2px solid #00ffcc;
            border-radius: 15px; /* Закругленные углы */
            padding: 10px 30px;
            font-size: 16px;
            font-weight: bold;
        }

        /* Эффект при наведении курсора на кнопку */
        QPushButton:hover {
            background-color: #00ffcc;
            color: #2b2b2b;
        }

        /* Эффект при нажатии на кнопку */
        QPushButton:pressed {
            background-color: #00ccaa;
            border: 2px solid #00ccaa;
        }
    )");
}