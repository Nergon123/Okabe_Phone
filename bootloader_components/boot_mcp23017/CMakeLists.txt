idf_component_register(
    SRCS "boot_mcp23017.c"
    INCLUDE_DIRS "include"
    REQUIRES boot_i2c
)