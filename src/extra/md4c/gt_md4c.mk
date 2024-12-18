CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/extra/md4c/*.c))

DEPPATH += --dep-path $(HMI_DIR)/src/extra/md4c
VPATH += :$(HMI_DIR)/src/extra/md4c

CFLAGS += "-I$(HMI_DIR)/src/extra/md4c"
