CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/widgets/serial/*.c))


DEPPATH += --dep-path $(HMI_DIR)/src/widgets/serial
VPATH += :$(HMI_DIR)/src/widgets/serial

CFLAGS += "-I$(HMI_DIR)/src/widgets/serial"
