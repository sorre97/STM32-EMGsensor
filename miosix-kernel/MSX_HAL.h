#ifndef MSX_HAL_H
#define MSX_HAL_H

/*********************************************************************
 * The idea of this class is to abstract hardware implementation 
 * E.g. ADC_handler_typedef ADC
 * registers settable e.g. ADC.EOICS = ENABLE etc...
 * or e.g. ADC.config(ADCconfigStruct)...
 * (so far only abstraction on register mask and bit set is provided)
**********************************************************************/

/* Register Masking */
#define __MSX_HAL_MASK_SET(__REGISTER__, __MASK__) ((__REGISTER__) |= (__MASK__))
#define __MSX_HAL_MASK_CLEAR(__REGISTER__, __MASK__) ((__REGISTER__) &= ~(__MASK__))
#define __MSX_HAL_MASK_CHECK(__REGISTER__, __MASK__) ((__REGISTER__) & (__MASK__))

/* Register Modifiers */
#define __MSX_HAL_REG_CLEAR(__REGISTER__) ((__REGISTER__) = 0);
#define __MSX_HAL_REG_SET(__REGISTER__, __VALUE__) ((__REGISTER__) = (__VALUE__));

#endif