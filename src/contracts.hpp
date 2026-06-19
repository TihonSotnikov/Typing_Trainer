#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace typing_trainer
{

// ----- БАЗОВЫЕ КОНТРАКТЫ -----

enum class TrainingMode : std::uint8_t
{
	Free, ///< Тренировка на загруженном пользователем тексте.
	Smart ///< Умная генерация текста на основе ошибок и других метрик.
};

enum class CharStatus : std::uint8_t
{
	Pending,
	Correct,
	Wrong
};

/// \brief Статус жизненного цикла сессии.
enum class SessionStatus : std::uint8_t
{
	Inactive, ///< Не запущена.
	Active,   ///< Запущена и идет набор текста.
	Paused,   ///< Приостановлена. Ввод заблокирован.
	Completed ///< Успешно завершена.
};

struct CharState
{
	char32_t   character;
	CharStatus status;
};

struct SessionConfig
{
	TrainingMode   mode;
	std::u32string custom_text;
	bool           ignore_case = false;
};


// ----- FRONTEND -> BACKEND (ВВОД) -----

enum class ControlKey : std::uint8_t
{
	Backspace,
	Escape
};

struct KeyPressData
{
	std::variant<char32_t, ControlKey>    key;
	std::chrono::steady_clock::time_point timestamp; ///< Время нажатия для расчета скорости.
};

struct StartSessionCommand
{
	SessionConfig config;
};

struct StopSessionCommand
{};

/// \brief Команда приостановки сессии (от фронтенд-таймера или кнопки UI).
struct PauseSessionCommand
{};

/// \brief Команда возобновления сессии.
struct ResumeSessionCommand
{};

using InputEvent = std::variant<KeyPressData, StartSessionCommand, StopSessionCommand,
                                PauseSessionCommand, ResumeSessionCommand>;


// ----- BACKEND -> FRONTEND (ОБРАБОТКА И ВЫВОД) -----

struct SessionMetrics
{
	double wpm         = 0.0;   ///< Слов в минуту.
	double cpm         = 0.0;   ///< Знаков в минуту.
	double accuracy    = 100.0; ///< Точность в % (0..100).
	double consistency = 0.0;   ///< Ритмичность/постоянность темпа в % (0..100).
};

/// \brief Полный снимок состояния для (ре)инициализации UI.
struct SessionState
{
	std::vector<CharState> chars;
	size_t                 cursor_position = 0;
	SessionMetrics         metrics;
	SessionStatus          status = SessionStatus::Inactive; ///< Текущий статус сессии.
};

/// \brief Дельта-обновление для быстрой отрисовки в процессе печати.
struct StateUpdate
{
	size_t         changed_index{}; ///< Позиция символа, изменившего статус.
	CharState      changed_char{};
	size_t         cursor_position{};
	SessionMetrics metrics;
	bool           is_completed = false;                 ///< Флаг успешного набора всего текста.
	SessionStatus  status       = SessionStatus::Active; ///< Текущий статус сессии.
};

using BackendEvent = std::variant<SessionState, StateUpdate>;


// ----- ИНТЕРФЕЙС ВЗАИМОДЕЙСТВИЯ -----

/// Основной интерфейс взаимодействия frontend/backend, реализация в backend.
class ITypingTrainerCore
{
public:
	ITypingTrainerCore()          = default;
	virtual ~ITypingTrainerCore() = default;

	ITypingTrainerCore(const ITypingTrainerCore&) = delete;
	ITypingTrainerCore(ITypingTrainerCore&&)      = delete;

	ITypingTrainerCore& operator=(const ITypingTrainerCore&) = delete;
	ITypingTrainerCore& operator=(ITypingTrainerCore&&)      = delete;

	/// \brief Отправка события во внутреннюю очередь Backend.
	virtual void push_input(InputEvent event) = 0;

	/// \brief Неблокирующее извлечение события для Frontend.
	/// \return Событие BackendEvent или std::nullopt, если новых событий нет.
	virtual std::optional<BackendEvent> poll_output() = 0;

	/// \brief Установка callback'а для уведомления UI-потока о новых данных.
	/// \param callback Обратный вызов для уведомления UI.
	/// \note Для неблокирующего ожидания для непрерывной работы UI.
	virtual void set_output_ready_callback(std::function<void()> callback) = 0;
};

} // namespace typing_trainer
