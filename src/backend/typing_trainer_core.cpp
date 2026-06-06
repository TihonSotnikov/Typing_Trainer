#include "typing_trainer_core.hpp"
#include <stop_token>

#include "../contracts.hpp"
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
		// Пока что заглушка
		text_to_type_ = U"smart training mode is currently under construction";
	}

	if (text_to_type_.empty()) return;

	chars_.clear();
	chars_.reserve(text_to_type_.size());
	for (char32_t const ch : text_to_type_)
		chars_.push_back(CharState{.character = ch, .status = CharStatus::Pending});

	cursor_        = 0;
	total_presses_ = 0;
	errors_count_  = 0;
	start_time_    = std::chrono::steady_clock::now();
	metrics_       = SessionMetrics{.wpm = 0.0, .cpm = 0.0, .accuracy = 100.0, .consistency = 0.0};
	is_active_     = true;

	output_queue_.push(
	    SessionState{.chars = chars_, .cursor_position = cursor_, .metrics = metrics_});
	notify_ui();
}

void TypingTrainerCore::stop_session()
{
	std::scoped_lock const lock(session_mutex_);
	is_active_ = false;
	chars_.clear();
	text_to_type_.clear();
	cursor_ = 0;
	
	SessionState empty_state{
		.chars = chars_,
		.cursor_position = cursor_,
		.metrics = SessionMetrics{.wpm = 0.0, .cpm = 0.0, .accuracy = 100.0, .consistency = 0.0}
	};
	
	output_queue_.push(empty_state);
	notify_ui();
}

void TypingTrainerCore::process_key_press(const KeyPressData& key_data)
{
	std::scoped_lock const lock(session_mutex_);

	if (!is_active_) return;

	if (std::holds_alternative<ControlKey>(key_data.key))
	{
		auto const ctrl = std::get<ControlKey>(key_data.key);
		if (ctrl == ControlKey::Escape)
		{
			is_active_ = false;
			return;
		}

		if (ctrl == ControlKey::Backspace)
		{
			if (cursor_ > 0)
			{
				--cursor_;
				chars_.at(cursor_).status = CharStatus::Pending;

				output_queue_.push(StateUpdate{.changed_index   = cursor_,
				                               .changed_char    = chars_.at(cursor_),
				                               .cursor_position = cursor_,
				                               .metrics         = metrics_,
				                               .is_completed    = false});
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

		bool const is_correct = (pressed == expected);

		chars_.at(cursor_).status = is_correct ? CharStatus::Correct : CharStatus::Wrong;

		total_presses_++;
		if (!is_correct) errors_count_++;

		recalculate_metrics(key_data.timestamp);

		bool const is_completed = (cursor_ == text_to_type_.size() - 1);

		StateUpdate update{.changed_index   = cursor_,
		                   .changed_char    = chars_.at(cursor_),
		                   .cursor_position = is_completed ? cursor_ : cursor_ + 1,
		                   .metrics         = metrics_,
		                   .is_completed    = is_completed};

		if (is_completed) is_active_ = false;
		else cursor_++;

		output_queue_.push(update);
		notify_ui();
	}
}

void TypingTrainerCore::recalculate_metrics(std::chrono::steady_clock::time_point current_time)
{
	std::chrono::duration<double> const elapsed = current_time - start_time_;
	double const                        seconds = elapsed.count();

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

	// Пока что заглушка
	metrics_.consistency = 0.0;
}

void TypingTrainerCore::notify_ui()
{
	std::scoped_lock const lock(callback_mutex_);
	if (output_ready_callback_) output_ready_callback_();
}

} // namespace typing_trainer
