#include "typing_trainer_adapter.hpp"
#include "typing_trainer_core.hpp"
#include <QMetaObject>

namespace typing_trainer
{

QmlTypingTrainerAdapter::QmlTypingTrainerAdapter(QObject* parent)
    : QObject(parent)
    , m_core(std::make_unique<TypingTrainerCore>())
{
    // Настраиваем колбэк от ядра
    m_core->set_output_ready_callback([this]() {
        // ВАЖНО: Колбэк вызывается из фонового потока std::jthread.
        // Мы используем QMetaObject::invokeMethod с параметром Qt::QueuedConnection,
        // чтобы безопасно перенаправить выполнение метода onOutputReady в UI-поток Qt.
        QMetaObject::invokeMethod(this, "onOutputReady", Qt::QueuedConnection);
    });

    m_textLength = m_textToType.length();
}

void QmlTypingTrainerAdapter::startSession(const QString& text)
{
    if (m_textToType != text) {
        m_textToType = text;
        m_textLength = text.length();
        emit textToTypeChanged();
        emit textLengthChanged();
    }

    StartSessionCommand cmd;
    cmd.config.mode = TrainingMode::Free;
    cmd.config.custom_text = text.toStdU32String();
    m_core->push_input(cmd);
}


void QmlTypingTrainerAdapter::startFreeSession()
{
    StartSessionCommand cmd;
    cmd.config.mode = TrainingMode::Free;
    cmd.config.custom_text = m_textToType.toStdU32String();
    m_core->push_input(cmd);
}

void QmlTypingTrainerAdapter::startSmartSession()
{
    StartSessionCommand cmd;
    cmd.config.mode = TrainingMode::Smart;
    cmd.config.language = m_language;
    cmd.config.filler_ratio = m_fillerRatio;
    cmd.config.target_length = m_targetLength;
    m_core->push_input(cmd);
}

void QmlTypingTrainerAdapter::stopSession()
{
    m_core->push_input(StopSessionCommand{});
}

void QmlTypingTrainerAdapter::pauseSession()
{
    m_core->push_input(PauseSessionCommand{});
}

void QmlTypingTrainerAdapter::resumeSession()
{
    m_core->push_input(ResumeSessionCommand{});
}

void QmlTypingTrainerAdapter::sendKeyPress(const QString& keyText)
{
    if (keyText.isEmpty()) return;
    
    KeyPressData data;
    data.key = static_cast<char32_t>(keyText.at(0).unicode());
    data.timestamp = std::chrono::steady_clock::now();
    m_core->push_input(data);
}

void QmlTypingTrainerAdapter::sendBackspace()
{
    KeyPressData data;
    data.key = ControlKey::Backspace;
    data.timestamp = std::chrono::steady_clock::now();
    m_core->push_input(data);
}

void QmlTypingTrainerAdapter::sendEscape()
{
    KeyPressData data;
    data.key = ControlKey::Escape;
    data.timestamp = std::chrono::steady_clock::now();
    m_core->push_input(data);
}

void QmlTypingTrainerAdapter::uploadCustomText(const QString& text)
{
    m_textToType = text;
    m_formattedText = "<span style='color: #9E9E9E'>" + text + "</span>";
    m_textLength = text.length();
    emit textToTypeChanged();
    emit formattedTextChanged();
    emit textLengthChanged();
}


QString QmlTypingTrainerAdapter::sessionStatus() const
{
    return sessionStatusToString(m_sessionStatus);
}

QString QmlTypingTrainerAdapter::sessionStatusToString(SessionStatus status) const
{
    switch (status)
    {
        case SessionStatus::Inactive:  return QStringLiteral("inactive");
        case SessionStatus::Active:    return QStringLiteral("active");
        case SessionStatus::Paused:    return QStringLiteral("paused");
        case SessionStatus::Completed: return QStringLiteral("completed");
    }
    return QStringLiteral("inactive");
}

QString QmlTypingTrainerAdapter::language() const
{
    switch (m_language)
    {
        case Language::English: return QStringLiteral("English");
        case Language::Russian: return QStringLiteral("Russian");
    }
    return QStringLiteral("English");
}

void QmlTypingTrainerAdapter::setLanguage(const QString& lang)
{
    if (lang == "English") {
        m_language = Language::English;
    } else if (lang == "Russian") {
        m_language = Language::Russian;
    }
    emit languageChanged();
}

void QmlTypingTrainerAdapter::setFillerRatio(double ratio)
{
    m_fillerRatio = ratio;
    emit fillerRatioChanged();
}

void QmlTypingTrainerAdapter::setTargetLength(uint length)
{
    m_targetLength = length;
    emit targetLengthChanged();
}

void QmlTypingTrainerAdapter::updateFormattedText()
{
    QString html;
    // Резервируем память (примерно 40 символов на каждый тег)
    html.reserve(m_charStates.size() * 40); 

    for (const auto& state : m_charStates)
    {
        // Конвертируем UTF-32 в QString
        QString ch = QString::fromUcs4(&state.character, 1);
        
        // Экранируем спецсимволы HTML, чтобы не сломать парсер
        if (ch == "<") ch = "&lt;";
        else if (ch == ">") ch = "&gt;";
        else if (ch == "&") ch = "&amp;";

        switch (state.status)
        {
            case CharStatus::Pending:
                // Серый цвет для не набранного текста
                html += "<span style='color: #9E9E9E'>" + ch + "</span>"; 
                break;
            case CharStatus::Correct:
                // Зеленый для правильного
                html += "<span style='color: #4CAF50'>" + ch + "</span>"; 
                break;
            case CharStatus::Wrong:
                // ВАЖНО: Если пользователь ошибся на пробеле, красный текст не будет видно.
                // Поэтому для пробелов подсвечиваем фон.
                if (state.character == U' ') {
                    html += "<span style='background-color: #EF9A9A'> </span>";
                } else {
                    html += "<span style='color: #F44336; text-decoration: underline;'>" + ch + "</span>";
                }
                break;
        }
    }

    if (m_formattedText != html) {
        m_formattedText = html;
        emit formattedTextChanged();
    }
}

void QmlTypingTrainerAdapter::onOutputReady()
{
    while (auto event_opt = m_core->poll_output())
    {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            
            if constexpr (std::is_same_v<T, SessionState>)
            {
                m_charStates = arg.chars; // Сохраняем массив
                m_textLength = m_charStates.size();
                updateFormattedText();    // Генерируем HTML строку
                
                m_cursorPosition = static_cast<int>(arg.cursor_position);
                m_wpm = arg.metrics.wpm;
                m_accuracy = arg.metrics.accuracy;
                m_sessionStatus = arg.status;
                
                emit cursorPositionChanged();
                emit metricsChanged();
                emit sessionStatusChanged();
            }
            else if constexpr (std::is_same_v<T, StateUpdate>)
            {
                // Обновляем конкретный символ по дельте
                if (arg.changed_index < m_charStates.size()) {
                    m_charStates[arg.changed_index] = arg.changed_char;
                }
                updateFormattedText(); // Пересобираем HTML
                
                m_cursorPosition = static_cast<int>(arg.cursor_position);
                m_wpm = arg.metrics.wpm;
                m_cpm = arg.metrics.cpm;
                m_accuracy = arg.metrics.accuracy;
                m_sessionStatus = arg.status;
                emit cursorPositionChanged();
                emit metricsChanged();
                emit sessionStatusChanged();

                if (arg.is_completed) {
                    m_cursorPosition++;
                    emit cursorPositionChanged();
                    emit sessionCompleted();
                }
            }
        }, *event_opt);
    }
}

} // namespace typing_trainer