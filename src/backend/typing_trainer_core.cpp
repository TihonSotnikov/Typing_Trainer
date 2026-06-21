#include "typing_trainer_core.hpp"
#include <stop_token>

#include "../contracts.hpp"
#include "dictionaries.hpp"
#include <chrono>
#include <cstddef>
#include <functional>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace typing_trainer
{

namespace
{

/// \brief Привести символ к нижнему регистру для языков приложения (EN и RU).
char32_t to_lower_app(char32_t ch)
{
	if (ch >= U'A' && ch <= U'Z') return ch + 0x20; // латиница
	if (ch >= U'А' && ch <= U'Я') return ch + 0x20; // кириллица А-Я -> а-я
	if (ch == U'Ё') return U'ё';                    // отдельная буква Ё
	return ch;
}

} // namespace

TypingTrainerCore::TypingTrainerCore()
{
	worker_thread_ = std::jthread([this](const std::stop_token& stoken) { process_loop(stoken); });
}

TypingTrainerCore::~TypingTrainerCore() { input_queue_.close(); }

void TypingTrainerCore::push_input(InputEvent event) { input_queue_.push(std::move(event)); }

std::optional<BackendEvent> TypingTrainerCore::poll_output() { return output_queue_.try_pop(); }

void TypingTrainerCore::set_output_ready_callback(std::function<void()> callback)
{
	std::scoped_lock const lock(callback_mutex_);
	output_ready_callback_ = std::move(callback);
}

void TypingTrainerCore::process_loop(const std::stop_token& stop_token)
{
	ngram_stats_.load();

	while (!stop_token.stop_requested())
	{
		auto event_opt = input_queue_.wait_and_pop(stop_token);
		if (event_opt == std::nullopt) break;
		handle_event(*event_opt);
	}
}

void TypingTrainerCore::handle_event(const InputEvent& event)
{
	std::visit(
	    [this](auto&& arg) {
		    using T = std::decay_t<decltype(arg)>;
		    if constexpr (std::is_same_v<T, StartSessionCommand>) start_session(arg);
		    else if constexpr (std::is_same_v<T, StopSessionCommand>) stop_session();
		    else if constexpr (std::is_same_v<T, PauseSessionCommand>) pause_session();
		    else if constexpr (std::is_same_v<T, ResumeSessionCommand>) resume_session();
		    else if constexpr (std::is_same_v<T, KeyPressData>) process_key_press(arg);
	    },
	    event);
}

void TypingTrainerCore::start_session(const StartSessionCommand& command)
{
	std::scoped_lock const lock(session_mutex_);

	if (command.config.mode == TrainingMode::Free) { text_to_type_ = command.config.custom_text; }
	else
	{
		auto const  weighted = ngram_stats_.weighted_ngrams();
		auto const& dict     = dictionary(command.config.language);
		text_to_type_ = smart_generator_.generate(weighted, dict, command.config.filler_ratio,
		                                          command.config.target_length);
	}

	if (text_to_type_.empty()) return;

	chars_.clear();
	chars_.reserve(text_to_type_.size());
	for (char32_t const ch : text_to_type_)
		chars_.push_back(CharState{.character = ch, .status = CharStatus::Pending});

	cursor_               = 0;
	ignore_case_          = command.config.ignore_case;
	total_presses_        = 0;
	errors_count_         = 0;
	accumulated_duration_ = std::chrono::steady_clock::duration::zero();
	last_resume_time_     = std::chrono::steady_clock::now();
	status_               = SessionStatus::Active;
	metrics_ = SessionMetrics{.wpm = 0.0, .cpm = 0.0, .accuracy = 100.0, .consistency = 0.0};

	ngram_stats_.reset_context();

	output_queue_.push(SessionState{
	    .chars = chars_, .cursor_position = cursor_, .metrics = metrics_, .status = status_});
	notify_ui();
}

void TypingTrainerCore::stop_session()
{
	std::scoped_lock const lock(session_mutex_);
	status_ = SessionStatus::Inactive;
	chars_.clear();
	text_to_type_.clear();
	cursor_               = 0;
	accumulated_duration_ = std::chrono::steady_clock::duration::zero();

	ngram_stats_.save();

	SessionState empty_state{
	    .chars           = chars_,
	    .cursor_position = cursor_,
	    .metrics = SessionMetrics{.wpm = 0.0, .cpm = 0.0, .accuracy = 100.0, .consistency = 0.0},
	    .status  = status_};

	output_queue_.push(empty_state);
	notify_ui();
}

void TypingTrainerCore::pause_session()
{
	std::scoped_lock const lock(session_mutex_);
	pause_session_internal();
}

void TypingTrainerCore::pause_session_internal()
{
	if (status_ != SessionStatus::Active) return;

	auto const now = std::chrono::steady_clock::now();
	accumulated_duration_ += (now - last_resume_time_);
	status_ = SessionStatus::Paused;

	output_queue_.push(SessionState{
	    .chars = chars_, .cursor_position = cursor_, .metrics = metrics_, .status = status_});
	notify_ui();
}

void TypingTrainerCore::resume_session()
{
	std::scoped_lock const lock(session_mutex_);
	resume_session_internal();
}

void TypingTrainerCore::resume_session_internal()
{
	if (status_ != SessionStatus::Paused) return;

	last_resume_time_ = std::chrono::steady_clock::now();
	status_           = SessionStatus::Active;
	ngram_stats_.reset_context(); // интервал через паузу как flight-time невалиден

	output_queue_.push(SessionState{
	    .chars = chars_, .cursor_position = cursor_, .metrics = metrics_, .status = status_});
	notify_ui();
}

void TypingTrainerCore::process_key_press(const KeyPressData& key_data)
{
	std::scoped_lock const lock(session_mutex_);

	if (status_ != SessionStatus::Active) return;

	if (std::holds_alternative<ControlKey>(key_data.key))
	{
		auto const ctrl = std::get<ControlKey>(key_data.key);
		if (ctrl == ControlKey::Escape)
		{
			pause_session_internal();
			return;
		}

		if (ctrl == ControlKey::Backspace)
		{
			ngram_stats_.reset_context(); // перепечатывание даст мусорный контекст

			if (cursor_ > 0)
			{
				--cursor_;
				chars_.at(cursor_).status = CharStatus::Pending;

				output_queue_.push(StateUpdate{.changed_index   = cursor_,
				                               .changed_char    = chars_.at(cursor_),
				                               .cursor_position = cursor_,
				                               .metrics         = metrics_,
				                               .is_completed    = false,
				                               .status          = status_});
				notify_ui();
			}
			return;
		}
	}

	if (std::holds_alternative<char32_t>(key_data.key))
	{
		if (cursor_ >= text_to_type_.size()) return;

		char32_t const pressed  = std::get<char32_t>(key_data.key);
		char32_t const expected = text_to_type_.at(cursor_);

		bool const is_correct = ignore_case_ ? (to_lower_app(pressed) == to_lower_app(expected))
		                                     : (pressed == expected);

		chars_.at(cursor_).status = is_correct ? CharStatus::Correct : CharStatus::Wrong;

		total_presses_++;
		if (!is_correct) errors_count_++;

		ngram_stats_.feed(expected, key_data.timestamp, is_correct);

		recalculate_metrics(key_data.timestamp);

		bool const is_completed = (cursor_ == text_to_type_.size() - 1);

		if (is_completed)
		{
			status_ = SessionStatus::Completed;
			ngram_stats_.save();
		}

		StateUpdate update{.changed_index   = cursor_,
		                   .changed_char    = chars_.at(cursor_),
		                   .cursor_position = is_completed ? cursor_ : cursor_ + 1,
		                   .metrics         = metrics_,
		                   .is_completed    = is_completed,
		                   .status          = status_};

		if (!is_completed) cursor_++;

		output_queue_.push(update);
		notify_ui();
	}
}

void TypingTrainerCore::recalculate_metrics(std::chrono::steady_clock::time_point current_time)
{
	auto const   total_duration = accumulated_duration_ + (current_time - last_resume_time_);
	double const seconds        = std::chrono::duration<double>(total_duration).count();

	if (seconds > 0.05)
	{
		size_t correct_count = 0;
		for (size_t i = 0; i <= cursor_; ++i)
			if (chars_.at(i).status == CharStatus::Correct) correct_count++;

		metrics_.cpm = (static_cast<double>(correct_count) / seconds) * 60.0;

		metrics_.wpm = metrics_.cpm / 5.0;
	}

	if (total_presses_ > 0)
	{
		metrics_.accuracy = (static_cast<double>(total_presses_ - errors_count_)
		                     / static_cast<double>(total_presses_))
		                    * 100.0;
	}
	else
	{
		metrics_.accuracy = 100.0;
	}

	metrics_.consistency = 0.0;
}

void TypingTrainerCore::notify_ui()
{
	std::scoped_lock const lock(callback_mutex_);
	if (output_ready_callback_) output_ready_callback_();
}

} // namespace typing_trainer
