#if (ENABLE_BANK_SWITCH == 0)
#error Bank switching support disabled. Build SGDK with ENABLE_BANK_SWITCH = 1
#endif

#if (MODULE_MEGAWIFI == 0)
#error MegaWiFi module disabled. Build SGDK with MODULE_MEGAWIFI = 1
#endif
