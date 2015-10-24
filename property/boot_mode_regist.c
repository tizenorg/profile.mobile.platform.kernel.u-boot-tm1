#include "boot_mode.h"
#include <config.h>
//define in board folder
CBOOT_FUNC s_boot_func_array[CHECK_BOOTMODE_FUN_NUM] = {
#ifndef CONFIG_MACH_CORI
#if !defined(CONFIG_KANAS_W) && !defined(CONFIG_KANAS_TD)
    get_mode_from_bat_low,
#endif
#endif
#ifdef CONFIG_SPRD_SYSDUMP
    write_sysdump_before_boot_extend,
#endif
    // 5 get mode from keypad
    get_mode_from_keypad,
    // 4 get mode from charger
    get_mode_from_charger,
#ifndef CONFIG_TIZEN
    // 1 get mode from file
    get_mode_from_file_extend,
    // 2 get mode from watch dog
    get_mode_from_watchdog,
    // 3 get mode from alarm register
    get_mode_from_alarm_register,
    // 0 get mode from calibration detect
    get_mode_from_pctool,
    // 6 get mode from gpio
    get_mode_from_gpio_extend,
    //shutdown device
    //get_mode_from_shutdown
#endif
};

void cmd_mode_regist(CBOOT_MODE_ENTRY *array)
{
    MODE_REGIST(CMD_NORMAL_MODE, normal_mode);
    MODE_REGIST(CMD_CHARGE_MODE, charge_mode);
	MODE_REGIST(CMD_THOR_MODE, thor_mode);
#ifndef CONFIG_TIZEN
    MODE_REGIST(CMD_RECOVERY_MODE, recovery_mode);
    MODE_REGIST(CMD_FACTORYTEST_MODE,factorytest_mode);
    MODE_REGIST(CMD_FASTBOOT_MODE, fastboot_mode);
    MODE_REGIST(CMD_WATCHDOG_REBOOT, watchdog_mode);
    MODE_REGIST(CMD_UNKNOW_REBOOT_MODE, unknow_reboot_mode);
    MODE_REGIST(CMD_PANIC_REBOOT, panic_reboot_mode);
#ifdef CONFIG_AUTODLOADER
    MODE_REGIST(CMD_AUTODLOADER_REBOOT, autodloader_mode);
#endif
    MODE_REGIST(CMD_SPECIAL_MODE, special_mode);
    MODE_REGIST(CMD_ENGTEST_MODE,factorytest_mode);//engtest_mode
    MODE_REGIST(CMD_CALIBRATION_MODE, calibration_mode);
    MODE_REGIST(CMD_AUTOTEST_MODE,autotest_mode);
    MODE_REGIST(CMD_EXT_RSTN_REBOOT_MODE, normal_mode);
    MODE_REGIST(CMD_IQ_REBOOT_MODE, iq_mode);
    MODE_REGIST(CMD_ALARM_MODE, alarm_mode);
#endif
}
