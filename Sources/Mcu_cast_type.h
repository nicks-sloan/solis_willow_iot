/*
 * Mcu_cast_type.h
 *
 *  Created on: Jul 18, 2023
 *      Author: DengS1
 */

#ifndef MCU_CAST_TYPE_H_
#define MCU_CAST_TYPE_H_

  #ifndef bool 
typedef unsigned char           bool;
  #endif

#ifndef false
  #define  false  0x00u                /* Boolean value FALSE. FALSE is defined always as a zero value. */
#endif
#ifndef true
  #define  true   0x01u                /* Boolean value TRUE. TRUE is defined always as a non zero value. */
#endif

#endif /* MCU_CAST_TYPE_H_ */
