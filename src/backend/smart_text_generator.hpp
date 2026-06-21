#pragma once

#include "ngram_statistics.hpp"

#include <cstddef>
#include <random>
#include <string>
#include <vector>

namespace typing_trainer
{

/// \brief Генератор практического текста для Smart-режима.
///
/// Из весов проблемных n-грамм и словаря собирает строку, насыщенную
/// проблемными граммами, но с долей обычных слов для читаемости.
class SmartTextGenerator
{
public:
	SmartTextGenerator();
	~SmartTextGenerator() = default;

	SmartTextGenerator(const SmartTextGenerator&)            = delete;
	SmartTextGenerator(SmartTextGenerator&&)                 = delete;
	SmartTextGenerator& operator=(const SmartTextGenerator&) = delete;
	SmartTextGenerator& operator=(SmartTextGenerator&&)      = delete;

	/// \brief Сгенерировать батч практики.
	/// \param weighted_ngrams Граммы с весами по убыванию.
	/// \param dictionary      Частотный словарь языка.
	/// \return Текст практики; пустая строка, если словарь пуст.
	[[nodiscard]] std::u32string generate(const std::vector<WeightedNgram>&  weighted_ngrams,
	                                      const std::vector<std::u32string>& dictionary);

private:
	std::mt19937 rng_;

	static constexpr std::size_t K_TARGET_NGRAMS
	    = 30; ///< Сколько худших грамм участвуют в генерациию.
	static constexpr double      K_FILLER_RATIO  = 0.3; ///< Доля обычных читаемых слов.
	static constexpr std::size_t K_TARGET_LENGTH = 250; ///< Длина батч в символах.
};

} // namespace typing_trainer
