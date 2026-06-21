# Blind Typing Trainer

Десктопный тренажёр слепой печати, который находит ваши слабые сочетания клавиш
и генерирует упражнения именно под них.

[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/)
[![Qt](https://img.shields.io/badge/Qt-6%20(QML%2FQuick)-41cd52.svg)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.21%2B-red.svg)](https://cmake.org/)

---

## Что это

Тренажёр измеряет время и ошибки по каждой n-грамме (1–3 символа), определяет, какие
сочетания даются вам тяжелее всего, и собирает из реальных слов текст, насыщенный именно
ими. Статистика копится между запусками — чем дольше пользуетесь, тем точнее подбор.

**Режимы:**

- **Smart** — текст генерируется из частотного словаря под ваши проблемные n-граммы.
- **Свободный** — тренировка на собственном тексте.

**Метрики в реальном времени:** WPM, CPM, точность и *consistency* — ритмичность печати
(разброс интервалов между нажатиями), а не только скорость.

---

## Архитектура

Проект разделён на два слоя, общающихся через единственный контракт
[`src/contracts.hpp`](src/contracts.hpp): фронтенд не знает о внутренностях ядра, ядро не
знает о Qt.

- **`backend/`** — чистый C++20 без Qt. `TypingTrainerCore` (машина состояний сессии и
  метрики), `NgramStatistics` (статистика n-грамм, веса, JSON-персистентность),
  `SmartTextGenerator` (генерация текста), `dictionaries` (словари en/ru).
- **`frontend/`** — Qt Quick/QML. `QmlTypingTrainerAdapter` мостит QML и ядро.

Обмен асинхронный: UI кладёт `InputEvent` в потокобезопасную очередь, фоновый
`std::jthread` обрабатывает их и возвращает `BackendEvent` (полный `SessionState` или
дельту `StateUpdate`). UI никогда не блокируется на вычислениях.

**Алгоритм Smart-режима.** Каждое нажатие учитывается во всех n-граммах, оканчивающихся на
текущем символе (накапливаются flight-time, попытки, ошибки; контекст строится по эталонному
тексту, поэтому опечатки его не загрязняют). Вес проблемности грамма — **W = T_avg + λ ·
error_rate**. Генератор берёт худшие граммы и собирает блок из словарных слов с этими
граммами, разбавляя обычными словами в заданной пропорции (`filler_ratio`).

---

## Сборка и запуск

**Требования:** C++20 (GCC 11+, Clang 13+, MSVC 19.30+), CMake 3.21+, Qt 6
(`Core Qml Quick Widgets QuickControls2 QuickEffects`), nlohmann/json. На Windows —
зависимости через [vcpkg](https://github.com/microsoft/vcpkg) (`VCPKG_ROOT` указывает на каталог vcpkg).

**Windows (vcpkg):**

```powershell
vcpkg install qtbase:x64-windows qtdeclarative:x64-windows nlohmann-json:x64-windows
cmake --preset vcpkg-release && cmake --build --preset release && cmake --install build/vcpkg-release --prefix dist
```

**Linux:**

```sh
sudo apt install build-essential cmake qt6-base-dev qt6-declarative-dev nlohmann-json3-dev qml6-module-qtquick qml6-module-qtquick-controls qml6-module-qtquick-layouts
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build && ./build/BlindTypingTrainer
```

Пресеты сборки — в [`CMakePresets.json`](CMakePresets.json).

---

## Статус и планы

- Экран статистики: история результатов и самые проблемные n-граммы.
- Пауза/возобновление из интерфейса (в ядре уже есть).
- Расширение словарей и новые языки.
- Автотесты ядра и CI.

---

## Команда

| Слой | Разработчик | Зона ответственности |
|------|-------------|----------------------|
| **Backend** | Тихон Сотников | Ядро сессии, машина состояний, метрики, алгоритм n-грамм, Smart-генератор, JSON-персистентность, потоковая модель. |
| **Frontend** | Андрей Червов | UI на Qt Quick/QML, адаптер `QObject`, обработка ввода, отрисовка текста и курсора, темы, метрики. |

Граница между зонами — контракт [`src/contracts.hpp`](src/contracts.hpp); изменения
согласуются только через него.

---

## Лицензия

См. [LICENSE](LICENSE).
