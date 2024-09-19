CSRCS += $(notdir $(wildcard $(HMI_DIR)/examples/*.c))

DEPPATH += --dep-path $(HMI_DIR)/examples
VPATH += :$(HMI_DIR)/examples

CFLAGS += "-I$(HMI_DIR)/examples"

DEPPATH += --dep-path $(HMI_DIR)/examples/widgets
VPATH += :$(HMI_DIR)/examples/widgets

CFLAGS += "-I$(HMI_DIR)/examples/widgets"
