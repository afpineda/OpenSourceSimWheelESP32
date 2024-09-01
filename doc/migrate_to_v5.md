# Migrating your custom firmware to version 5 from a previous version

There are minor changes to version 5. Any custom, **battery-based**, firmware needs some rewriting.
Other custom firmwares are not affected.

For more details, please, read again the following documentation:

- [Battery monitor subsystem](./hardware/subsystems/BatteryMonitor/BatteryMonitor_en.md)

## New namespace `batteryMonitor`

Just replace `power::startBatteryMonitor()` with `batteryMonitor::begin()` (same parameters).

Please, run the [source code setup procedure](./firmware/sourcesSetup_en.md) again.
