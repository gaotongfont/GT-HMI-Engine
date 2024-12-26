CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/extra/serial/*.c))

DEPPATH += --dep-path $(HMI_DIR)/src/extra/serial
VPATH += :$(HMI_DIR)/src/extra/serial

CFLAGS += "-I$(HMI_DIR)/src/extra/serial"

