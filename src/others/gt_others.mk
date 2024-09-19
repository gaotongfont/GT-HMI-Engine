CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/others/*.c))


DEPPATH += --dep-path $(HMI_DIR)/src/others
VPATH += :$(HMI_DIR)/src/others

CFLAGS += "-I$(HMI_DIR)/src/others"
