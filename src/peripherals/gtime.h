#ifndef LORAWAN_DEV_TOOL_APP_CORE_GTIME_H_
#define LORAWAN_DEV_TOOL_APP_CORE_GTIME_H_

#include <cstdint>

namespace app::time {

// Возвращает время в формате UTC (от 01.01.1970) + дополнительно миллисекунды.
uint32_t GetUTCTime(uint16_t *millisec);
uint64_t GetUTCTime();
// Устанавливаем время в формате UTC.
int SetUTCTime(uint32_t sec, uint16_t millisec);
int SetUTCTime(uint64_t utc_ms);
// Проверяем, что время корректное.
bool IsCorrectTime();

}

#endif //LORAWAN_DEV_TOOL_APP_CORE_GTIME_H_
