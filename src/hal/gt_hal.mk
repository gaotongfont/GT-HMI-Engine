CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/hal/*.c))

DEPPATH += --dep-path $(HMI_DIR)/src/hal
VPATH += :$(HMI_DIR)/src/hal

CFLAGS += "-I$(HMI_DIR)/src/hal"
