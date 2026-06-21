#include "smart_text_generator.hpp"

#include "ngram_statistics.hpp"

#include <algorithm>
#include <cstddef>
#include <random>
#include <string>
#include <vector>

namespace typing_trainer
{

SmartTextGenerator::SmartTextGenerator() : rng_(std::random_device{}()) {}

std::u32string SmartTextGenerator::generate(const std::vector<WeightedNgram>&  weighted_ngrams,
                                            const std::vector<std::u32string>& dictionary)
{
	if (dictionary.empty()) return {};

	std::size_t const target_count = std::min(weighted_ngrams.size(), K_TARGET_NGRAMS);

	std::vector<std::u32string> scored_words;
	std::vector<double>         scored_weights;
	scored_words.reserve(dictionary.size());
	scored_weights.reserve(dictionary.size());

	for (auto const& word : dictionary)
	{
		double score = 0.0;
		for (std::size_t i = 0; i < target_count; ++i)
		{
			WeightedNgram const& wn = weighted_ngrams[i];
			if (word.find(wn.gram) != std::u32string::npos) score += wn.weight;
		}
		if (score > 0.0)
		{
			scored_words.push_back(word);
			scored_weights.push_back(score);
		}
	}

	std::discrete_distribution<std::size_t>    weighted_pick(scored_weights.begin(),
	                                                         scored_weights.end());
	std::uniform_int_distribution<std::size_t> filler_pick(0, dictionary.size() - 1);
	std::uniform_real_distribution<double>     coin(0.0, 1.0);

	bool const has_scored = !scored_words.empty();

	std::u32string result;
	while (result.size() < K_TARGET_LENGTH)
	{
		bool const take_filler = !has_scored || (coin(rng_) < K_FILLER_RATIO);

		std::u32string const& word
		    = take_filler ? dictionary[filler_pick(rng_)] : scored_words[weighted_pick(rng_)];

		if (!result.empty()) result.push_back(U' ');
		result += word;
	}

	return result;
}

} // namespace typing_trainer
