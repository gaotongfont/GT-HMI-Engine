CSRCS += gt_sjpg.c
CSRCS += tjpgd.c


DEPPATH += --dep-path $(HMI_DIR)/src/extra/jpeg
VPATH += :$(HMI_DIR)/src/extra/jpeg

CFLAGS += "-I$(HMI_DIR)/src/extra/jpeg"

