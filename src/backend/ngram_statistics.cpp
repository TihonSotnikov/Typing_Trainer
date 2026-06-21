#include "ngram_statistics.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

namespace
{

constexpr int         K_SCHEMA_VERSION  = 2; // v2: + поле attempts
constexpr const char* K_STATS_FILE_PATH = "ngram_stats.json";

} // namespace

namespace typing_trainer
{

void to_json(nlohmann::json& j, const NgramStat& stat)
{
	j = nlohmann::json{{"occurrences", stat.occurrences},
	                   {"total_time", stat.total_time},
	                   {"errors", stat.errors},
	                   {"attempts", stat.attempts}};
}

void from_json(const nlohmann::json& j, NgramStat& stat)
{
	j.at("occurrences").get_to(stat.occurrences);
	j.at("total_time").get_to(stat.total_time);
	j.at("errors").get_to(stat.errors);
	j.at("attempts").get_to(stat.attempts);
}

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
		++stat.attempts;
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

void NgramStatistics::save() const
{
	nlohmann::json ngrams = nlohmann::json::array();
	for (auto const& [gram, stat] : stats_)
	{
		nlohmann::json entry = stat;
		entry["gram"]        = std::vector<std::uint32_t>(gram.begin(), gram.end());
		ngrams.push_back(std::move(entry));
	}

	nlohmann::json root;
	root["version"] = K_SCHEMA_VERSION;
	root["ngrams"]  = std::move(ngrams);

	std::ofstream file(K_STATS_FILE_PATH);
	if (file) file << root.dump(2);
}

void NgramStatistics::load()
{
	std::ifstream file(K_STATS_FILE_PATH);
	if (!file) return;

	try
	{
		nlohmann::json const root = nlohmann::json::parse(file);
		if (root.value("version", 0) != K_SCHEMA_VERSION) return;

		stats_.clear();
		for (auto const& entry : root.at("ngrams"))
		{
			auto const           code_points = entry.at("gram").get<std::vector<std::uint32_t>>();
			std::u32string const gram(code_points.begin(), code_points.end());
			stats_[gram] = entry.get<NgramStat>();
		}
	}
	catch (std::exception const&)
	{
		stats_.clear();
	}
}

std::vector<WeightedNgram> NgramStatistics::weighted_ngrams() const
{
	std::vector<WeightedNgram> result;
	result.reserve(stats_.size());

	for (auto const& [gram, stat] : stats_)
	{
		if (stat.attempts < K_MIN_ATTEMPTS) continue;

		double const t_avg = (stat.occurrences > 0)
		                         ? stat.total_time / static_cast<double>(stat.occurrences)
		                         : 0.0;
		double const error_rate
		    = static_cast<double>(stat.errors) / static_cast<double>(stat.attempts);

		double const weight = t_avg + (K_ERROR_WEIGHT * error_rate);
		result.push_back(WeightedNgram{.gram = gram, .weight = weight});
	}

	std::ranges::sort(
	    result, [](WeightedNgram const& a, WeightedNgram const& b) { return a.weight > b.weight; });

	return result;
}

} // namespace typing_trainer
