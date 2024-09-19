CSRCS += $(notdir $(wildcard $(HMI_DIR)/driver/*.c))


DEPPATH += --dep-path $(HMI_DIR)/driver
VPATH += :$(HMI_DIR)/driver

CFLAGS += "-I$(HMI_DIR)/driver"
