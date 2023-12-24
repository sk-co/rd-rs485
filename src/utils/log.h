//
// Created by SERG on 16.02.2021.
//

#ifndef LORAWAN_DEV_SRC_UTILS_LOG_H_
#define LORAWAN_DEV_SRC_UTILS_LOG_H_

#include <cstdio>
#include <cstdarg>
#include <string_view>
#include <array>
#include "uart-def.h"
#include "shift-static-vectors.hpp"

namespace utils::log {

void Init(Uart_t *uart, uint32_t baudrate);
void Out(const char *str, ...);
void Flush();
// Передача отладочных сообщений завершилась.
bool IsTransmited();

}

#define L_NO      0
#define L_ERROR   1
#define L_TRACE   2
#define L_DEBUG   3
#ifndef LOG_LEVEL
  #define LOG_LEVEL L_NO
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#if (LOG_LEVEL == L_NO)
  #define LOG_INIT(uart, baudrate)
  #define LOG_FLUSH()
  #define LOG_IS_TRANSMITE() true
#else
  #define LOG_INIT(uart, baudrate) utils::log::Init(uart,baudrate)
  #define LOG_FLUSH() utils::log::Flush()
  #define LOG_IS_TRANSMITED() utils::log::IsTransmited()
#endif

#if (LOG_LEVEL == L_NO)
  #define LOG_ERR(...)
  #define LOG_TRC(...)
  #define LOG_DBG(...)
#elif (LOG_LEVEL == L_ERROR)
  #define LOG_ERR(...)    utils::log::Out(__VA_ARGS__)
  #define LOG_TRC(...)
  #define LOG_DBG(...)
#elif (LOG_LEVEL == L_TRACE)
  #define LOG_ERR(...)    utils::log::Out(__VA_ARGS__)
  #define LOG_TRC(...)    utils::log::Out(__VA_ARGS__)
  #define LOG_DBG(...)
#elif (LOG_LEVEL == L_DEBUG)
  #define LOG_ERR(...)    utils::log::Out(__VA_ARGS__)
  #define LOG_TRC(...)    utils::log::Out(__VA_ARGS__)
  #define LOG_DBG(...)    utils::log::Out(__VA_ARGS__)
#endif

#if defined(__cplusplus)
}
#endif

#endif //LORAWAN_DEV_SRC_UTILS_LOG_H_
