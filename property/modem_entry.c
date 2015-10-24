#include "normal_mode.h"
#include <asm/arch/sprd_reg.h>
#ifdef CONFIG_ARCH_SCX35L //only for sharkL branch modem boot process
#include <asm/arch/cp_boot.h>
#endif

extern void boot_cp0(void);
extern void boot_cp1(void);
extern void boot_cp2(void);

#ifdef CONFIG_ARCH_SCX35L //only for sharkL branch modem boot process

void cp_adr_remap(u32 cp_kernel_exec_addr, u32 cp_zero_map_addr)
{
    u32 data[3] = {0xe59f0000, 0xe12fff10,   cp_kernel_exec_addr};
    memcpy( cp_zero_map_addr, data, sizeof(data));      /* copy cp0 source code */
}

void modem_entry()
{

#ifdef  CONFIG_PMIC_ARM7_BOOT   //arm7 boot
     pmic_arm7_boot();

#endif    
#ifdef  CONFIG_CP0_ARM0_BOOT   //cp0 arm0 boot
    debugf(" REG_PMU_APB_CP_SOFT_RST=%x,REG_AON_APB_APB_RST1=0x%x\n",  REG_PMU_APB_CP_SOFT_RST,REG_AON_APB_APB_RST1);
    cp_adr_remap(CP0_ARM0_EXEC_ADR, CP0_ZERO_MAP_ADR);
     cp0_arm0_boot();
#endif

#ifdef  CONFIG_CP1_BOOT    //cp1 boot
    cp_adr_remap(CP1_EXEC_ADR, CP1_ZERO_MAP_ADR);
    cp1_boot();
#endif //end of TDLTE_DSDA
}
#else //shark,9620,tshark branch
void modem_entry()
{
#if modem_cp0_enable
		boot_cp0();
#endif
	
#if modem_cp1_enable
		boot_cp1();
#endif
	
#if modem_cp2_enable
		boot_cp2();
#endif	
}

#endif /* CONFIG_ARCH_SCX35L */

