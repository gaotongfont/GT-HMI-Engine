CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/extra/draw/*.c))


DEPPATH += --dep-path $(HMI_DIR)/src/extra/draw
VPATH += :$(HMI_DIR)/src/extra/draw

CFLAGS += "-I$(HMI_DIR)/src/extra/draw"
