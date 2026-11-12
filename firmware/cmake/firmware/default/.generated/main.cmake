include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(firmware_default_library_list )

# Handle files with suffix s, for group default-XC32
if(firmware_default_default_XC32_FILE_TYPE_assemble)
add_library(firmware_default_default_XC32_assemble OBJECT ${firmware_default_default_XC32_FILE_TYPE_assemble})
    firmware_default_default_XC32_assemble_rule(firmware_default_default_XC32_assemble)
    list(APPEND firmware_default_library_list "$<TARGET_OBJECTS:firmware_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(firmware_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(firmware_default_default_XC32_assembleWithPreprocess OBJECT ${firmware_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    firmware_default_default_XC32_assembleWithPreprocess_rule(firmware_default_default_XC32_assembleWithPreprocess)
    list(APPEND firmware_default_library_list "$<TARGET_OBJECTS:firmware_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(firmware_default_default_XC32_FILE_TYPE_compile)
add_library(firmware_default_default_XC32_compile OBJECT ${firmware_default_default_XC32_FILE_TYPE_compile})
    firmware_default_default_XC32_compile_rule(firmware_default_default_XC32_compile)
    list(APPEND firmware_default_library_list "$<TARGET_OBJECTS:firmware_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(firmware_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(firmware_default_default_XC32_compile_cpp OBJECT ${firmware_default_default_XC32_FILE_TYPE_compile_cpp})
    firmware_default_default_XC32_compile_cpp_rule(firmware_default_default_XC32_compile_cpp)
    list(APPEND firmware_default_library_list "$<TARGET_OBJECTS:firmware_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(firmware_default_default_XC32_FILE_TYPE_dependentObject)
add_library(firmware_default_default_XC32_dependentObject OBJECT ${firmware_default_default_XC32_FILE_TYPE_dependentObject})
    firmware_default_default_XC32_dependentObject_rule(firmware_default_default_XC32_dependentObject)
    list(APPEND firmware_default_library_list "$<TARGET_OBJECTS:firmware_default_default_XC32_dependentObject>")

endif()


# Main target for this project
add_executable(firmware_default_image_pS36_6Mu ${firmware_default_library_list})

set_target_properties(firmware_default_image_pS36_6Mu PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${firmware_default_output_dir}")
target_link_libraries(firmware_default_image_pS36_6Mu PRIVATE ${firmware_default_default_XC32_FILE_TYPE_link})
# Add the link options from the rule file.
firmware_default_link_rule( firmware_default_image_pS36_6Mu)

# Add bin2hex target for converting built file to a .hex file.
string(REGEX REPLACE [.]elf$ .hex firmware_default_image_name_hex ${firmware_default_image_name})
add_custom_target(firmware_default_Bin2Hex ALL
    COMMAND ${MP_BIN2HEX} \"${firmware_default_output_dir}/${firmware_default_image_name}\"
    BYPRODUCTS ${firmware_default_output_dir}/${firmware_default_image_name_hex}
    COMMENT "Convert built file to .hex")
add_dependencies(firmware_default_Bin2Hex firmware_default_image_pS36_6Mu)




