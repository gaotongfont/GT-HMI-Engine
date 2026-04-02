CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/core/effects/*.c))


DEPPATH += --dep-path $(HMI_DIR)/src/core/effects
VPATH += :$(HMI_DIR)/src/core/effects

CFLAGS += "-I$(HMI_DIR)/src/core/effects"
