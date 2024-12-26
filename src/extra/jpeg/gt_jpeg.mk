CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/extra/jpeg/*.c))

DEPPATH += --dep-path $(HMI_DIR)/src/extra/jpeg
VPATH += :$(HMI_DIR)/src/extra/jpeg

CFLAGS += "-I$(HMI_DIR)/src/extra/jpeg"

