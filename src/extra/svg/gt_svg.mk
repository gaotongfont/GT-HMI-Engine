CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/extra/svg/*.c))

DEPPATH += --dep-path $(HMI_DIR)/src/extra/svg
VPATH += :$(HMI_DIR)/src/extra/svg

CFLAGS += "-I$(HMI_DIR)/src/extra/svg"
