/**
 ******************************************************************************
 * @file    RingBuffer.c
 * @author  XinLi
 * @version v1.0
 * @date    15-January-2018
 * @brief   Ring buffer module source file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>Copyright &copy; 2018 XinLi</center></h2>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *该程序是免费软件：您可以根据自由软件基金会发布的GNU通用公共许可的条款（许可的版本3）
 *	或（根据您的选择）任何更高版本来重新分发和/或修改它。
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *分发该程序是希望它会有用，但是没有任何保证；
 *甚至没有对适销性或特定用途适用性的暗示保证。
 *有关更多详细信息，请参见GNU通用公共许可证。
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *您应该已经与该程序一起收到了GNU通用公共许可证的副本。
 *如果没有，请参见<https://www.gnu.org/licenses/>。
 ******************************************************************************
 */

/* Header includes -----------------------------------------------------------*/
#include "RingBuffer.h"
#include <stdlib.h>
#include <string.h>

/* Macro definitions ---------------------------------------------------------*/
#define min(a, b) (((a) < (b)) ? (a) : (b))

/* Type definitions ----------------------------------------------------------*/
/* Variable declarations -----------------------------------------------------*/
/* Variable definitions ------------------------------------------------------*/
/* Function declarations -----------------------------------------------------*/
static bool is_power_of_2(uint32_t x);

/* Function definitions ------------------------------------------------------*/

/**
  * @brief  Allocates a new FIFO and its internal buffer.
            分配一个新的FIFO及其内部缓冲区。
  * @param  [in] size: The size of the internal buffer to be allocated.
            要分配的内部缓冲区的大小。
  * @note   The size will be rounded-up to a power of 2.
            大小将四舍五入为2的幂。
  * @return RingBuffer pointer.
            RingBuffer指针。
  */
RingBuffer *RingBuffer_Init(RingBuffer *fifo, uint8_t *buffer, uint32_t size)
{
  fifo->buffer = buffer;
  if (fifo != NULL)
  {
    if (is_power_of_2(size) != true)
    {
      return NULL;
    }
    if (size > 0x80000000UL)
    {
      return NULL;
    }
    if (fifo->buffer == NULL)
    {
      return NULL;
    }
    fifo->size = size;
    fifo->in = fifo->out = 0;
  }

  return fifo;
}

/**
 * @brief  Puts some data into the FIFO.
 * @param  [in] fifo: The fifo to be used.
 * @param  [in] in:   The data to be added.
 * @param  [in] len:  The length of the data to be added.
 * @return The number of bytes copied.
 * @note   This function copies at most @len bytes from the @in into
 *         the FIFO depending on the free space, and returns the number
 *         of bytes copied.
 */
uint32_t RingBuffer_In(RingBuffer *fifo, void *in, uint32_t len, uint8_t wait)
{
	if(wait)
	{
  while (RingBuffer_Avail(fifo) < len)
    ;
	}

  len = min(len, RingBuffer_Avail(fifo));
  /* First put the data starting from fifo->in to buffer end. */
  uint32_t l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
  memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), in, l);

  /* Then put the rest (if any) at the beginning of the buffer. */
  memcpy(fifo->buffer, (uint8_t *)in + l, len - l);

  fifo->in += len;

  return len;
}

/**
 * @brief  Gets some data from the FIFO.
 * @param  [in] fifo: The fifo to be used.
 * @param  [in] out:  Where the data must be copied.
 * @param  [in] len:  The size of the destination buffer.
 * @return The number of copied bytes.
 * @note   This function copies at most @len bytes from the FIFO into
 *         the @out and returns the number of copied bytes.
 */
uint32_t RingBuffer_Out(RingBuffer *fifo, void *out, uint32_t len)
{
  len = min(len, RingBuffer_Len(fifo));

  /* First get the data from fifo->out until the end of the buffer. */
  uint32_t l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
  memcpy(out, fifo->buffer + (fifo->out & (fifo->size - 1)), l);

  /* Then get the rest (if any) from the beginning of the buffer. */
  memcpy((uint8_t *)out + l, fifo->buffer, len - l);

  fifo->out += len;

  return len;
}

/**
 * @brief  Determine whether some value is a power of two.
 * @param  [in] x: The number to be confirmed.
 * @retval true:   Yes.
 * @retval false:  No.
 * @note   Where zero is not considered a power of two.
 */
static bool is_power_of_2(uint32_t x)
{
  return (x != 0) && ((x & (x - 1)) == 0);
}
