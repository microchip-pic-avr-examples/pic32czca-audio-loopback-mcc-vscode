# The following variables contains the files used by the different stages of the build process.
set(firmware_default_default_XC32_FILE_TYPE_assemble)
set_source_files_properties(${firmware_default_default_XC32_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${firmware_default_default_XC32_FILE_TYPE_assemble})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(firmware_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
set_source_files_properties(${firmware_default_default_XC32_FILE_TYPE_assembleWithPreprocess} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${firmware_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(firmware_default_default_XC32_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/app_audio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/bsp/bsp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/driver/i2c/src/drv_i2c.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/exceptions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/initialization.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/interrupts.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/libc_syscalls.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/clock/plib_clock.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/dma/plib_dma.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/evsys/plib_evsys.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/nvic/plib_nvic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/port/plib_port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/sercom/i2c_master/plib_sercom3_i2c_master.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/sercom/usart/plib_sercom1_usart.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/tcc/plib_tcc0.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/startup_xc32.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/stdio/xc32_monitor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/system/cache/sys_cache.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/system/console/src/sys_console.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/system/console/src/sys_console_uart.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/system/debug/src/sys_debug.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/system/int/src/sys_int.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/system/time/src/sys_time.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/tasks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/main.c")
set_source_files_properties(${firmware_default_default_XC32_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(firmware_default_default_XC32_FILE_TYPE_compile_cpp)
set_source_files_properties(${firmware_default_default_XC32_FILE_TYPE_compile_cpp} PROPERTIES LANGUAGE CXX)
set(firmware_default_default_XC32_FILE_TYPE_link)

# The linker script used for the build.
set(firmware_default_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/PIC32CZ8110CA80208.ld")
set(firmware_default_image_name "default.elf")
set(firmware_default_image_base_name "default")

# The output directory of the final image.
set(firmware_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/firmware")

# The full path to the final image.
set(firmware_default_full_path_to_image ${firmware_default_output_dir}/${firmware_default_image_name})
