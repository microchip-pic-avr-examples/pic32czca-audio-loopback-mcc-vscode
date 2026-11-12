set(DEPENDENT_MP_BIN2HEXfirmware_default_pS36_6Mu "c:/Program Files/Microchip/xc32/v6.00/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFfirmware_default_pS36_6Mu ${CMAKE_CURRENT_LIST_DIR}/../../../../out/firmware/default.elf)
set(DEPENDENT_TARGET_DIRfirmware_default_pS36_6Mu ${CMAKE_CURRENT_LIST_DIR}/../../../../out/firmware)
set(DEPENDENT_BYPRODUCTSfirmware_default_pS36_6Mu ${DEPENDENT_TARGET_DIRfirmware_default_pS36_6Mu}/${sourceFileNamefirmware_default_pS36_6Mu}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRfirmware_default_pS36_6Mu}/${sourceFileNamefirmware_default_pS36_6Mu}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXfirmware_default_pS36_6Mu} --image ${DEPENDENT_DEPENDENT_TARGET_ELFfirmware_default_pS36_6Mu} --image-generated-c ${sourceFileNamefirmware_default_pS36_6Mu}.c --image-generated-h ${sourceFileNamefirmware_default_pS36_6Mu}.h --image-copy-mode ${modefirmware_default_pS36_6Mu} --image-offset ${addressfirmware_default_pS36_6Mu} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRfirmware_default_pS36_6Mu}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFfirmware_default_pS36_6Mu})
add_custom_target(
    dependent_produced_source_artifactfirmware_default_pS36_6Mu 
    DEPENDS ${DEPENDENT_TARGET_DIRfirmware_default_pS36_6Mu}/${sourceFileNamefirmware_default_pS36_6Mu}.c
    )
