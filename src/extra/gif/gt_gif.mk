CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/extra/gif/*.c))


DEPPATH += --dep-path $(HMI_DIR)/src/extra/gif
VPATH += :$(HMI_DIR)/src/extra/gif

CFLAGS += "-I$(HMI_DIR)/src/extra/gif"

