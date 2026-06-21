#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <memory>
#include "contracts.hpp"

namespace typing_trainer
{

class QmlTypingTrainerAdapter : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString textToType READ textToType NOTIFY textToTypeChanged)
    Q_PROPERTY(QString formattedText READ formattedText NOTIFY formattedTextChanged)
    Q_PROPERTY(int textLength READ textLength NOTIFY textLengthChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(double wpm READ wpm NOTIFY metricsChanged)
    Q_PROPERTY(double cpm READ cpm NOTIFY metricsChanged)
    Q_PROPERTY(double accuracy READ accuracy NOTIFY metricsChanged)
    Q_PROPERTY(QString sessionStatus READ sessionStatus NOTIFY sessionStatusChanged)

    Q_PROPERTY(bool smartMode READ smartMode WRITE setSmartMode NOTIFY smartModeChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(double fillerRatio READ fillerRatio WRITE setFillerRatio NOTIFY fillerRatioChanged)
    Q_PROPERTY(uint targetLength READ targetLength WRITE setTargetLength NOTIFY targetLengthChanged)

public:
    explicit QmlTypingTrainerAdapter(QObject* parent = nullptr);

    // Методы, доступные для вызова из QML
    Q_INVOKABLE void startSession(const QString& text);
    Q_INVOKABLE void startFreeSession();
    Q_INVOKABLE void startSmartSession();
    Q_INVOKABLE void stopSession();
    Q_INVOKABLE void pauseSession();
    Q_INVOKABLE void resumeSession();

    Q_INVOKABLE void sendKeyPress(const QString& keyText);
    Q_INVOKABLE void sendBackspace();
    Q_INVOKABLE void sendEscape();

    Q_INVOKABLE void uploadCustomText(const QString& text);

    // Геттеры для Q_PROPERTY
    QString textToType() const { return m_textToType; }
    QString formattedText() const { return m_formattedText; }
    int textLength() const { return m_textLength; }
    int cursorPosition() const { return m_cursorPosition; }
    double wpm() const { return m_wpm; }
    double cpm() const { return m_cpm; }
    double accuracy() const { return m_accuracy; }
    QString sessionStatus() const;

    QString language() const;
    bool smartMode() const { return m_smartMode; }
    double fillerRatio() const { return m_fillerRatio; }
    uint targetLength() const { return m_targetLength; }

    // Сеттеры для Q_PROPERTY
    void setSmartMode(bool smartMode) {
        if (m_smartMode != smartMode) {
            m_smartMode = smartMode;
            emit smartModeChanged();
        }
    }
    void setLanguage(const QString& lang);
    void setFillerRatio(double ratio);
    void setTargetLength(uint length);

signals:
    void textToTypeChanged();
    void formattedTextChanged();
    void textLengthChanged();
    void cursorPositionChanged();
    void metricsChanged();
    void sessionStatusChanged();
    void sessionCompleted();
    void smartModeChanged();
    void languageChanged();
    void fillerRatioChanged();
    void targetLengthChanged();

private:
    // Обработчик события готовности данных (выполняется в UI-потоке)
    Q_INVOKABLE void onOutputReady();
    void updateFormattedText();
    QString sessionStatusToString(SessionStatus status) const;

    std::unique_ptr<ITypingTrainerCore> m_core;

    // Настройки
    bool m_smartMode = false;
    Language m_language = Language::English;
    double m_fillerRatio = 0.3;
    uint m_targetLength = 250;
    
    // Кэш состояния для QML
    QString m_textToType = "He watched as the young man tried to impress everyone in the room with his intelligence. There was no doubt that he was smart.";
    QString m_formattedText;
    int m_textLength = 0;
    int m_cursorPosition = 0;
    double m_wpm = 0.0;
    double m_cpm = 0.0;
    double m_accuracy = 100.0;
    SessionStatus m_sessionStatus = SessionStatus::Inactive;
    
    std::vector<CharState> m_charStates;
};

} // namespace typing_trainer