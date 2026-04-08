## gt_latex makefile
# Provide include path and link the prebuilt static library gt_latex.a

# Add header include path for gt_latex
CFLAGS += "-I$(HMI_DIR)/lib/gt_latex"

# Link the prebuilt static archive when the final binary is linked
# (app/Makefile sets LDFLAGS initially; here we append the archive path)
LDFLAGS += $(HMI_DIR)/lib/gt_latex/gt_latex.a

