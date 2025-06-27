CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/extra/libtexprintf/*.c))

DEPPATH += --dep-path $(HMI_DIR)/src/extra/libtexprintf
VPATH += :$(HMI_DIR)/src/extra/libtexprintf

CFLAGS += "-I$(HMI_DIR)/src/extra/libtexprintf"

