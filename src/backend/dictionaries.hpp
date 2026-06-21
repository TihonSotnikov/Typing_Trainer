#pragma once

#include "../contracts.hpp"

#include <string>
#include <vector>

namespace typing_trainer
{

/// \brief Частотный словарь для языка (ассет для Smart-генератора).
/// \note Возвращает ссылку на статические данные, копирования нет.
///       Неизвестный язык -> английский словарь.
const std::vector<std::u32string>& dictionary(Language language);

} // namespace typing_trainer
