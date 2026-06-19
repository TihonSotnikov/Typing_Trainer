#pragma once

#include "../concurrent_queue.hpp"
#include "../contracts.hpp"
#include "ngram_statistics.hpp"
#include <chrono>
#include <cstddef>
#include <functional>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

namespace typing_trainer
{

/// \brief Реализация ядра тренажера слепой печати.
/// Управляет жизненным циклом сессии тренировки, обрабатывает ввод
/// в фоновом потоке и рассчитывает метрики скорости и точности.
class TypingTrainerCore final : public ITypingTrainerCore
{
public:
	TypingTrainerCore();
	~TypingTrainerCore() override;

	TypingTrainerCore(const TypingTrainerCore&)            = delete;
	TypingTrainerCore(TypingTrainerCore&&)                 = delete;
	TypingTrainerCore& operator=(const TypingTrainerCore&) = delete;
	TypingTrainerCore& operator=(TypingTrainerCore&&)      = delete;

	/// \brief Отправка события ввода во внутреннюю очередь бэкенда.
	void push_input(InputEvent event) override;

	/// \brief Неблокирующее извлечение события для графического интерфейса.
	std::optional<BackendEvent> poll_output() override;

	/// \brief Установка функции обратного вызова для уведомления UI о наличии данных.
	void set_output_ready_callback(std::function<void()> callback) override;

private:
	// ----- ФУНКЦИИ -----

	/// \brief Основной рабочий цикл фонового потока.
	void process_loop(const std::stop_token& stop_token);

	/// \brief Распределение входящего события по обработчикам.
	void handle_event(const InputEvent& event);

	/// \brief Запуск новой сессии тренировки.
	void start_session(const StartSessionCommand& command);

	/// \brief Прерывание текущей сессии.
	void stop_session();

	/// \brief Потокобезопасная приостановка текущей сессии.
	void pause_session();

	/// \brief Внутренний метод приостановки без захвата мьютекса.
	void pause_session_internal();

	/// \brief Потокобезопасное возобновление текущей сессии.
	void resume_session();

	/// \brief Внутренний метод возобновления без захвата мьютекса.
	void resume_session_internal();

	/// \brief Обработка нажатия клавиши.
	void process_key_press(const KeyPressData& key_data);

	/// \brief Потокобезопасный вызов колбэка интерфейса.
	void notify_ui();

	/// \brief Перерасчет метрик на основе текущего времени.
	void recalculate_metrics(std::chrono::steady_clock::time_point current_time);


	// ----- ДАННЫЕ -----

	// Очереди асинхронного обмена сообщениями
	ConcurrentQueue<InputEvent>   input_queue_;
	ConcurrentQueue<BackendEvent> output_queue_;

	// Синхронизация доступа к callback-функции
	std::mutex            callback_mutex_;
	std::function<void()> output_ready_callback_;

	// Фоновый поток обработки ввода
	std::jthread worker_thread_;

	// Данные текущей сессии
	std::mutex             session_mutex_;
	SessionStatus          status_ = SessionStatus::Inactive;
	std::u32string         text_to_type_;
	std::vector<CharState> chars_;
	size_t                 cursor_ = 0;

	// Статистика сессии и метрики
	std::chrono::steady_clock::time_point last_resume_time_;
	std::chrono::steady_clock::duration   accumulated_duration_{0};
	size_t                                total_presses_ = 0;
	size_t                                errors_count_  = 0;
	SessionMetrics                        metrics_;

	// Сбор статистики n-грамм для smart-режима
	NgramStatistics ngram_stats_;
};

} // namespace typing_trainer
