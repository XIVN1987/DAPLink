#ifndef __CMSIS_COMPILER_H__
#define __CMSIS_COMPILER_H__


#define __CC_ARM

#define __STATIC_INLINE         static inline

#define __STATIC_FORCEINLINE    __attribute__((always_inline)) static inline

#define __WEAK                  __attribute__((weak))


#endif
