CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/font/*.c))

DEPPATH += --dep-path $(HMI_DIR)/src/font
VPATH += :$(HMI_DIR)/src/font

CFLAGS += "-I$(HMI_DIR)/src/font"
