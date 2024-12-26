CSRCS += $(notdir $(wildcard $(HMI_DIR)/test/*.c))

DEPPATH += --dep-path $(HMI_DIR)/test
VPATH += :$(HMI_DIR)/test

CFLAGS += "-I$(HMI_DIR)/test"