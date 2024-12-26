CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/utils/*.c))


DEPPATH += --dep-path $(HMI_DIR)/src/utils
VPATH += :$(HMI_DIR)/src/utils

CFLAGS += "-I$(HMI_DIR)/src/utils"
