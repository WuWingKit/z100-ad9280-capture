# Current MCU entry point

This copy is the SPI **slave** build. Start with [the project README](../README.md).
The main adaptation is `05_application/firmware_app/source/src/app_sync_frontend.c`.
Documents under `docs/` retain historical vendor and MCU-master reference material; they do not override this release’s four-wire pin map or mailbox protocol. Never flash the bare application BIN as a whole-Flash image. Use the matching 512KiB Boot V3 image from the release.
