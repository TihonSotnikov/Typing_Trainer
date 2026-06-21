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
    Q_PROPERTY(int cursorPosition READ cursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(double wpm READ wpm NOTIFY metricsChanged)
    Q_PROPERTY(double accuracy READ accuracy NOTIFY metricsChanged)
    Q_PROPERTY(QString sessionStatus READ sessionStatus NOTIFY sessionStatusChanged)

public:
    explicit QmlTypingTrainerAdapter(QObject* parent = nullptr);

    // Методы, доступные для вызова из QML
    Q_INVOKABLE void startSession(const QString& text);
    Q_INVOKABLE void startFreeSession();
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
    int cursorPosition() const { return m_cursorPosition; }
    double wpm() const { return m_wpm; }
    double accuracy() const { return m_accuracy; }
    QString sessionStatus() const;

signals:
    void textToTypeChanged();
    void formattedTextChanged();
    void cursorPositionChanged();
    void metricsChanged();
    void sessionStatusChanged();
    void sessionCompleted();

private:
    // Обработчик события готовности данных (выполняется в UI-потоке)
    Q_INVOKABLE void onOutputReady();
    void updateFormattedText();
    QString sessionStatusToString(SessionStatus status) const;

    std::unique_ptr<ITypingTrainerCore> m_core;
    
    // Кэш состояния для QML
    QString m_textToType = "He watched as the young man tried to impress everyone in the room with his intelligence. There was no doubt that he was smart.";
    QString m_formattedText;
    int m_cursorPosition = 0;
    double m_wpm = 0.0;
    double m_accuracy = 100.0;
    SessionStatus m_sessionStatus = SessionStatus::Inactive;
    
    std::vector<CharState> m_charStates;
};

} // namespace typing_trainer