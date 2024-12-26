CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/extra/*.c))

# extra
DEPPATH += --dep-path $(HMI_DIR)/src/extra
VPATH += :$(HMI_DIR)/src/extra

CFLAGS += "-I$(HMI_DIR)/src/extra"
