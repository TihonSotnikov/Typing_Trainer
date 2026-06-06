#include "main_window.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // 1. Настройка главного виджета и Layout'а (компоновщика)
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // 2. Создание элементов
    helloLabel = new QLabel("BTS Template", this);
    helloLabel->setContentsMargins(10, 10, 10, 10); // Отступы вокруг текста

    closeButton = new QPushButton("Закрыть приложение", this);
    
    // Подключаем кнопку к функции закрытия окна
    connect(closeButton, &QPushButton::clicked, this, &QMainWindow::close);

    // 3. Добавляем элементы в Layout
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft); // Выравнивание по центру сверху
    // layout->addStretch(); // Пружина сверху
    layout->addWidget(helloLabel, 0, Qt::AlignLeft);
    // layout->addSpacing(20); // Отступ между текстом и кнопкой
    layout->addStretch(); // Пружина между текстом и кнопкой
    layout->addLayout(buttonLayout);
    buttonLayout->addStretch(); // Пружина слева от кнопки
    buttonLayout->addWidget(closeButton, 0, Qt::AlignHCenter); // Центрируем кнопку
    buttonLayout->addStretch(); // Пружина справа от кнопки
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
