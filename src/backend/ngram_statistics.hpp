#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <nlohmann/json_fwd.hpp> // NOLINT(misc-include-cleaner)
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace typing_trainer
{

/// \brief Накопленная статистика по одной n-грамме (T_avg = total_time / occurrences).
struct NgramStat
{
	std::uint64_t occurrences = 0;   ///< Корректные нажатия, давшие валидное время.
	double        total_time  = 0.0; ///< Сумма flight-time по ним, сек.
	std::uint64_t errors      = 0;   ///< Ошибки на этой n-грамме.
	std::uint64_t attempts    = 0;   ///< Все нажатия (любой flight, даже с выбросами).
};

/// \brief n-грамма с посчитанным весом проблемности (для smart-генератора.
struct WeightedNgram
{
	std::u32string gram;
	double         weight = 0.0;
};

/// \brief Сбор статистики проблемных n-грамм (1..3 символа) по потоку нажатий.
///
/// Ключ карты - целевая (ожидаемая) подстрока, поэтому контекст всегда чистый,
/// без опечаток пользователя. Слой только накапливает данные; расчёт весов и
/// сериализация - отдельная задача.
///
/// \note НЕ потокобезопасен: владелец - worker-поток ядра, доступ только из него.
class NgramStatistics
{
public:
	NgramStatistics()  = default;
	~NgramStatistics() = default;

	NgramStatistics(const NgramStatistics&)            = delete;
	NgramStatistics(NgramStatistics&&)                 = delete;
	NgramStatistics& operator=(const NgramStatistics&) = delete;
	NgramStatistics& operator=(NgramStatistics&&)      = delete;

	/// \brief Учесть одно нажатие на текущей позиции целевого текста.
	/// \param expected  Ожидаемый символ позиции (из него строится контекст n-грамм).
	/// \param timestamp Время нажатия; flight-time от прошлой позиции считается внутри.
	/// \param correct   true, если введён ожидаемый символ.
	void feed(char32_t expected, std::chrono::steady_clock::time_point timestamp, bool correct);

	/// \brief Сбросить контекст набора (окно символов + точку отсчёта времени),
	///        не затрагивая накопленную статистику.
	/// \note Вызывать там, где разрыв до следующего символа невалиден как flight-time:
	///       старт сессии, backspace, возобновление после паузы.
	void reset_context();

	/// \brief Сохранить накопленную статистику в JSON-файл.
	void save() const;

	/// \brief Загрузить статистику из JSON-файла.
	/// \note Отсутствие, повреждение или несовместимость по версии не считаются ошибкой:
	///       статистика просто остаётся пустой.
	void load();

	/// \brief Снимок n-грамм с весом W = T_avg + λ·error_rate, по убыванию веса.
	/// \note Граммы с attempts < K_MIN_ATTEMPTS отброшены как шум. НЕ потокобезопасна, только
	/// worker-поток.
	[[nodiscard]] std::vector<WeightedNgram> weighted_ngrams() const;

private:
	/// \brief Записать наблюдение во все n-граммы (1..3), оканчивающиеся на expected.
	void accumulate(char32_t expected, std::optional<double> flight_sec, bool correct);

	// Статистика n-грамм.
	static constexpr double      K_MAX_FLIGHT_SECONDS = 1.5; ///< Отсечение пауз/выбросов.
	static constexpr std::size_t K_MAX_CONTEXT        = 2;   ///< До 2 символов слева -> n до 3.

	// Вес n-грамм.
	static constexpr double        K_ERROR_WEIGHT = 1.0; ///< λ: вес error_rate в формуле W.
	static constexpr std::uint64_t K_MIN_ATTEMPTS
	    = 5; ///< Порог attempts: ниже - шум, грамма отброшена.

	std::unordered_map<std::u32string, NgramStat> stats_;
	std::u32string context_; ///< До K_MAX_CONTEXT последних expected.
	std::optional<std::chrono::steady_clock::time_point> last_ts_;
};

/// \note Для nlohmann.
void to_json(nlohmann::json& j, const NgramStat& stat);
/// \note Для nlohmann.
void from_json(const nlohmann::json& j, NgramStat& stat);

} // namespace typing_trainer
