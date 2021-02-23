#ifndef MSX_HAL_H
#define MSX_HAL_H

/*****************************
 * Explaination of MSX_HAL_H
 * [...]
 * [...]
******************************/

/* Register Masking */
#define __MSX_HAL_MASK_SET(__REGISTER__, __MASK__) ((__REGISTER__) |= (__MASK__))
#define __MSX_HAL_MASK_CLEAR(__REGISTER__, __MASK__) ((__REGISTER__) &= ~(__MASK__))
#define __MSX_HAL_MASK_CHECK(__REGISTER__, __MASK__) ((__REGISTER__) & (__MASK__))

/* Register Modifiers */
#define __MSX_HAL_REG_CLEAR(__REGISTER__) (__REGISTER__ = 0);

#endif