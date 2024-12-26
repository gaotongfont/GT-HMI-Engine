CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/widgets/*.c))


DEPPATH += --dep-path $(HMI_DIR)/src/widgets
VPATH += :$(HMI_DIR)/src/widgets

CFLAGS += "-I$(HMI_DIR)/src/widgets"


# serial
DEPPATH += --dep-path $(HMI_DIR)/src/widgets/serial
VPATH += :$(HMI_DIR)/src/widgets/serial

CFLAGS += "-I$(HMI_DIR)/src/widgets/serial"
