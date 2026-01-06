CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/extra/qrcodegen/*.c))


DEPPATH += --dep-path $(HMI_DIR)/src/extra/qrcodegen
VPATH += :$(HMI_DIR)/src/extra/qrcodegen

CFLAGS += "-I$(HMI_DIR)/src/extra/qrcodegen"

