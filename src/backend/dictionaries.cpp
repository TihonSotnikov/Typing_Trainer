#include "dictionaries.hpp"

#include "../contracts.hpp"

#include <string>
#include <vector>

namespace typing_trainer
{

const std::vector<std::u32string>& dictionary(Language language)
{
	static const std::vector<std::u32string> dict_en = {
	    U"the",    U"be",    U"to",     U"of",      U"and",   U"in",     U"that",     U"have",
	    U"it",     U"for",   U"not",    U"on",      U"with",  U"you",    U"do",       U"at",
	    U"this",   U"but",   U"his",    U"by",      U"from",  U"they",   U"we",       U"say",
	    U"her",    U"she",   U"or",     U"will",    U"my",    U"one",    U"all",      U"would",
	    U"there",  U"their", U"what",   U"so",      U"up",    U"out",    U"if",       U"about",
	    U"who",    U"get",   U"which",  U"go",      U"me",    U"when",   U"make",     U"can",
	    U"like",   U"time",  U"just",   U"him",     U"know",  U"take",   U"into",     U"year",
	    U"your",   U"good",  U"some",   U"could",   U"them",  U"see",    U"other",    U"than",
	    U"then",   U"now",   U"look",   U"only",    U"come",  U"over",   U"think",    U"back",
	    U"after",  U"use",   U"two",    U"how",     U"work",  U"first",  U"well",     U"way",
	    U"even",   U"new",   U"want",   U"because", U"these", U"give",   U"day",      U"most",
	    U"world",  U"life",  U"hand",   U"part",    U"place", U"week",   U"case",     U"point",
	    U"number", U"group", U"fact",   U"water",   U"story", U"family", U"head",     U"night",
	    U"friend", U"home",  U"school", U"music",   U"light", U"word",   U"question", U"answer",
	};

	static const std::vector<std::u32string> dict_ru = {
	    U"и",       U"в",        U"не",      U"на",      U"я",      U"быть",   U"он",
	    U"что",     U"а",        U"по",      U"это",     U"она",    U"этот",   U"к",
	    U"но",      U"они",      U"мы",      U"как",     U"из",     U"у",      U"который",
	    U"то",      U"за",       U"свой",    U"весь",    U"год",    U"от",     U"так",
	    U"для",     U"ты",       U"же",      U"все",     U"тот",    U"мочь",   U"вы",
	    U"человек", U"такой",    U"его",     U"сказать", U"только", U"или",    U"ещё",
	    U"один",    U"уже",      U"до",      U"время",   U"если",   U"сам",    U"когда",
	    U"другой",  U"говорить", U"наш",     U"мой",     U"знать",  U"стать",  U"при",
	    U"чтобы",   U"дело",     U"жизнь",   U"кто",     U"первый", U"очень",  U"два",
	    U"день",    U"новый",    U"рука",    U"даже",    U"раз",    U"где",    U"там",
	    U"под",     U"можно",    U"большой", U"должен",  U"место",  U"иметь",  U"ничего",
	    U"после",   U"без",      U"видеть",  U"надо",    U"теперь", U"хотеть", U"между",
	    U"через",   U"тоже",     U"нет",     U"сейчас",  U"потом",  U"делать", U"лицо",
	    U"мир",     U"глаз",     U"любить",  U"право",   U"конец",  U"начать", U"считать",
	    U"голова",  U"вода",     U"слово",   U"вопрос",  U"ответ",  U"музыка", U"свет",
	    U"дом",     U"друг",     U"школа",   U"работа",  U"город",  U"страна", U"ночь",
	    U"мать",
	};

	switch (language)
	{
	case Language::Russian:
		return dict_ru;
	case Language::English:
		return dict_en;
	}
	return dict_en; // на всякий
}

} // namespace typing_trainer
