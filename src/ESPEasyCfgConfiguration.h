#ifndef ESPEasyCfgConfiguration_H_
#define ESPEasyCfgConfiguration_H_

#define ESPEasyCfg_SERIAL_DEBUG 1

#ifdef ESPEasyCfg_SERIAL_DEBUG
	#define DebugPrintln(a) (Serial.println(a))
	#define DebugPrint(a) (Serial.print(a))
#else
	#define DebugPrintln(a)
	#define DebugPrint(a)
#endif

#endif