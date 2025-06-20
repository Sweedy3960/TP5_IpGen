include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(TP5_IpGeN_pic32mx_eth_sk2_library_list )

# Handle files with suffix s, for group pic32mx_eth_sk2-XC32
if(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_FILE_TYPE_assemble)
add_library(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_assemble OBJECT ${TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_FILE_TYPE_assemble})
    TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_assemble_rule(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_assemble)
    list(APPEND TP5_IpGeN_pic32mx_eth_sk2_library_list "$<TARGET_OBJECTS:TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_assemble>")
endif()

# Handle files with suffix S, for group pic32mx_eth_sk2-XC32
if(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_assembleWithPreprocess OBJECT ${TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_FILE_TYPE_assembleWithPreprocess})
    TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_assembleWithPreprocess_rule(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_assembleWithPreprocess)
    list(APPEND TP5_IpGeN_pic32mx_eth_sk2_library_list "$<TARGET_OBJECTS:TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_assembleWithPreprocess>")
endif()

# Handle files with suffix [cC], for group pic32mx_eth_sk2-XC32
if(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_FILE_TYPE_compile)
add_library(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_compile OBJECT ${TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_FILE_TYPE_compile})
    TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_compile_rule(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_compile)
    list(APPEND TP5_IpGeN_pic32mx_eth_sk2_library_list "$<TARGET_OBJECTS:TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_compile>")
endif()

# Handle files with suffix cpp, for group pic32mx_eth_sk2-XC32
if(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_FILE_TYPE_compile_cpp)
add_library(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_compile_cpp OBJECT ${TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_FILE_TYPE_compile_cpp})
    TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_compile_cpp_rule(TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_compile_cpp)
    list(APPEND TP5_IpGeN_pic32mx_eth_sk2_library_list "$<TARGET_OBJECTS:TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_compile_cpp>")
endif()

add_executable(${TP5_IpGeN_pic32mx_eth_sk2_image_name} ${TP5_IpGeN_pic32mx_eth_sk2_library_list})

target_link_libraries(${TP5_IpGeN_pic32mx_eth_sk2_image_name} PRIVATE ${TP5_IpGeN_pic32mx_eth_sk2_pic32mx_eth_sk2_XC32_FILE_TYPE_link})

# Add the link options from the rule file.
TP5_IpGeN_pic32mx_eth_sk2_link_rule(${TP5_IpGeN_pic32mx_eth_sk2_image_name})

# Add bin2hex target for converting built file to a .hex file.
add_custom_target(TP5_IpGeN_pic32mx_eth_sk2_Bin2Hex ALL
    ${MP_BIN2HEX} ${TP5_IpGeN_pic32mx_eth_sk2_image_name})
add_dependencies(TP5_IpGeN_pic32mx_eth_sk2_Bin2Hex ${TP5_IpGeN_pic32mx_eth_sk2_image_name})

# Post build target to copy built file to the output directory.
add_custom_command(TARGET ${TP5_IpGeN_pic32mx_eth_sk2_image_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${TP5_IpGeN_pic32mx_eth_sk2_output_dir}
                    COMMAND ${CMAKE_COMMAND} -E copy ${TP5_IpGeN_pic32mx_eth_sk2_image_name} ${TP5_IpGeN_pic32mx_eth_sk2_output_dir}/${TP5_IpGeN_pic32mx_eth_sk2_original_image_name}
                    BYPRODUCTS ${TP5_IpGeN_pic32mx_eth_sk2_output_dir}/${TP5_IpGeN_pic32mx_eth_sk2_original_image_name})
