CSRCS += $(notdir $(wildcard $(HMI_DIR)/src/extra/*.c))

# extra
DEPPATH += --dep-path $(HMI_DIR)/src/extra
VPATH += :$(HMI_DIR)/src/extra

CFLAGS += "-I$(HMI_DIR)/src/extra"

# draw
DEPPATH += --dep-path $(HMI_DIR)/src/extra/draw
VPATH += :$(HMI_DIR)/src/extra/draw

CFLAGS += "-I$(HMI_DIR)/src/extra/draw"

# png
DEPPATH += --dep-path $(HMI_DIR)/src/extra/png
VPATH += :$(HMI_DIR)/src/extra/png

CFLAGS += "-I$(HMI_DIR)/src/extra/png"

# jpeg
DEPPATH += --dep-path $(HMI_DIR)/src/extra/jpeg
VPATH += :$(HMI_DIR)/src/extra/jpeg

CFLAGS += "-I$(HMI_DIR)/src/extra/jpeg"

# gif
DEPPATH += --dep-path $(HMI_DIR)/src/extra/gif
VPATH += :$(HMI_DIR)/src/extra/gif

CFLAGS += "-I$(HMI_DIR)/src/extra/gif"
