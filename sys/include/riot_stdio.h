/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *               2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_stdio Standard I/O
 * @ingroup     sys
 * @brief       Generic interface for mapping the standard input/output streams
 * @{
 *
 * @file
 * @brief       Generic standard I/O interface definition
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef STDIO_H
#define STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initialize the given STDIO module
 */
void stdio_init(void);

/**
 * @brief   Read @p len bytes from standard input into @p buffer
 *
 * @param[out]  buffer  buffer to read into
 * @param[in]   len     nr of bytes to read
 *
 * @return      nr of bytes read
 * @return      <0 on error
 */
int stdio_read(char* buffer, int len);

/**
 * @brief write @p len bytes from @p buffer into uart
 *
 * @param[in]   buffer  buffer to read from
 * @param[in]   len     nr of bytes to write
 *
 * @return      nr of bytes written
 * @return      <0 on error
 */
int stdio_write(const char* buffer, int len);

#ifdef __cplusplus
}
#endif

#endif /* STDIO_H */
/** @} */
