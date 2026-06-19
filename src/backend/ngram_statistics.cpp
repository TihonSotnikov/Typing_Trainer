#include "ngram_statistics.hpp"

#include <chrono>
#include <cstddef>
#include <optional>
#include <string>

namespace typing_trainer
{

void NgramStatistics::feed(char32_t expected, std::chrono::steady_clock::time_point timestamp,
                           bool correct)
{
	std::optional<double> flight;
	if (last_ts_)
	{
		double const sec = std::chrono::duration<double>(timestamp - *last_ts_).count();
		// Пауза «подумать» даёт огромный интервал и портит T_avg - по времени такие
		// наблюдения отбрасываем. Точку отсчёта ниже всё равно сдвигаем: интервал
		// до следующего символа - реальное время и от текущего отброса не зависит.
		if (sec >= 0.0 && sec <= K_MAX_FLIGHT_SECONDS) flight = sec;
	}

	accumulate(expected, flight, correct);

	context_.push_back(expected);
	if (context_.size() > K_MAX_CONTEXT) context_.erase(context_.begin());
	last_ts_ = timestamp;
}

void NgramStatistics::accumulate(char32_t expected, std::optional<double> flight, bool correct)
{
	// Граммы, оканчивающиеся на expected, с наращиванием контекста слева:
	//   [expected] -> [c-1, expected] -> [c-2, c-1, expected].
	// Время идёт в T_avg только для корректных нажатий с валидным flight;
	// ошибки учитываются всегда.
	std::u32string    gram(1, expected);
	std::size_t const ctx = context_.size();

	for (std::size_t k = 0; k <= ctx; ++k)
	{
		if (k > 0) gram.insert(gram.begin(), context_.at(ctx - k));

		NgramStat& stat = stats_[gram];
		if (correct && flight)
		{
			stat.total_time += *flight;
			++stat.occurrences;
		}
		else if (!correct) { ++stat.errors; }
	}
}

void NgramStatistics::reset_context()
{
	context_.clear();
	last_ts_.reset();
}

} // namespace typing_trainer
