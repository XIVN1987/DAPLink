#ifndef __CMSIS_COMPILER_H__
#define __CMSIS_COMPILER_H__


#define __CC_ARM

#define __STATIC_FORCEINLINE    __attribute__((always_inline)) static inline

#define __WEAK                  __attribute__((weak))



typedef struct
{
  __I  uint32_t CPUID;                  /*!< Offset: 0x000 (R/ )  CPUID Base Register */
  __IO uint32_t ICSR;                   /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register */
  __IO uint32_t VTOR;                   /*!< Offset: 0x008 (R/W)  Vector Table Offset Register */
  __IO uint32_t AIRCR;                  /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register */
} SCB_Type;


#define SCB ((SCB_Type *) 0xE000ED00UL)


#define SCB_AIRCR_VECTKEY_Pos              16U                                            /*!< SCB AIRCR: VECTKEY Position */
#define SCB_AIRCR_VECTKEY_Msk              (0xFFFFUL << SCB_AIRCR_VECTKEY_Pos)            /*!< SCB AIRCR: VECTKEY Mask */

#define SCB_AIRCR_PRIGROUP_Pos              8U                                            /*!< SCB AIRCR: PRIGROUP Position */
#define SCB_AIRCR_PRIGROUP_Msk             (7UL << SCB_AIRCR_PRIGROUP_Pos)                /*!< SCB AIRCR: PRIGROUP Mask */

#define SCB_AIRCR_SYSRESETREQ_Pos           2U                                            /*!< SCB AIRCR: SYSRESETREQ Position */
#define SCB_AIRCR_SYSRESETREQ_Msk          (1UL << SCB_AIRCR_SYSRESETREQ_Pos)             /*!< SCB AIRCR: SYSRESETREQ Mask */


#endif
