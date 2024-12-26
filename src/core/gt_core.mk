CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/core/*.c))


DEPPATH += --dep-path $(HMI_DIR)/src/core
VPATH += :$(HMI_DIR)/src/core

CFLAGS += "-I$(HMI_DIR)/src/core"
