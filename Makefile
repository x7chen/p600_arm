MAKE := make
CLEAN := make clean
FLASH := make flash

SDK_ROOT := .
.PHONY: all app bootloader clean
all: app bootloader
app:
	$(MAKE) -C ./app
bootloader:
	$(MAKE) -C ./bootloader
flash-app:
	$(FLASH) -C ./app
flash-sd:
	@echo Flashing: s130_nrf51_2.0.1_softdevice.hex
	nrfjprog --program $(SDK_ROOT)/components/softdevice/s130/hex/s130_nrf51_2.0.1_softdevice.hex -f nrf51 --chiperase 
	nrfjprog --reset -f nrf51
flash-bl:
	$(FLASH) -C ./bootloader
clean:
	$(CLEAN) -C ./app
	$(CLEAN) -C ./bootloader
help:
	@echo the target file in the app and bootloader folder
