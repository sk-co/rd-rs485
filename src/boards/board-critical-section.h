//
// Created by SERG on 20.02.2021.
//

#ifndef LORAWAN_DEV_SRC_BOARDS_BOARD_CRITICAL_SECTION_H_
#define LORAWAN_DEV_SRC_BOARDS_BOARD_CRITICAL_SECTION_H_

#include <stdint.h>

/*!
 * Begins critical section
 */
#define CRITICAL_SECTION_BEGIN() uint32_t mask; BoardCriticalSectionBegin( &mask )

/*!
 * Ends critical section
 */
#define CRITICAL_SECTION_END() BoardCriticalSectionEnd( &mask )

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * ============================================================================
 * Following functions must be implemented inside the specific platform
 * board.c file.
 * ============================================================================
 */
/*!
 * Disable interrupts, begins critical section
 *
 * \param [IN] mask Pointer to a variable where to store the CPU IRQ mask
 */
void BoardCriticalSectionBegin( uint32_t *mask );

/*!
 * Ends critical section
 *
 * \param [IN] mask Pointer to a variable where the CPU IRQ mask was stored
 */
void BoardCriticalSectionEnd( uint32_t *mask );

#ifdef __cplusplus
}
#endif

#endif //LORAWAN_DEV_SRC_BOARDS_BOARD_CRITICAL_SECTION_H_
