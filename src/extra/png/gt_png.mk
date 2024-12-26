CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/extra/png/*.c))


DEPPATH += --dep-path $(HMI_DIR)/src/extra/png
VPATH += :$(HMI_DIR)/src/extra/png

CFLAGS += "-I$(HMI_DIR)/src/extra/png"

