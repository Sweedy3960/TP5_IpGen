# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "C:/Users/aymer/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(TP5_IpGeN_config_1_config_1_toolchain_assemble_rule target)
    set(options
        "-g"
        "${ASSEMBLER_PRE}"
        "-mprocessor=32MX795F512L"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},-I${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/TP5_IpGeN.X"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32MX_DFP/1.5.259")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/TP5_IpGeN.X")
endfunction()
function(TP5_IpGeN_config_1_config_1_toolchain_assembleWithPreprocess_rule target)
    set(options
        "-x"
        "assembler-with-cpp"
        "-g"
        "${MP_EXTRA_AS_PRE}"
        "-mprocessor=32MX795F512L"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},-I${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/TP5_IpGeN.X")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_config_1=config_1")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/TP5_IpGeN.X")
endfunction()
function(TP5_IpGeN_config_1_config_1_toolchain_compile_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-x"
        "c"
        "-c"
        "-mprocessor=32MX795F512L"
        "-ffunction-sections"
        "-Werror"
        "-Wall"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32MX_DFP/1.5.259")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_config_1=config_1")
    target_include_directories(${target}
        PRIVATE "firmware/TP5_IpGeN.X/system_config"
        PRIVATE "firmware/TP5_IpGeN.X/system_config/pic32mx_eth_Sk2"
        PRIVATE "firmware/TP5_IpGeN.X/crypto/src"
        PRIVATE "firmware/TP5_IpGeN.X/crypto"
        PRIVATE "firmware/TP5_IpGeN.X/driver/tmr"
        PRIVATE "firmware/TP5_IpGeN.X/driver"
        PRIVATE "firmware/TP5_IpGeN.X/system/clk"
        PRIVATE "firmware/TP5_IpGeN.X/system/clk/src"
        PRIVATE "firmware/TP5_IpGeN.X/system/common"
        PRIVATE "firmware/TP5_IpGeN.X/system/devcon"
        PRIVATE "firmware/TP5_IpGeN.X/system/int"
        PRIVATE "firmware/TP5_IpGeN.X/system/ports"
        PRIVATE "firmware/TP5_IpGeN.X/system/random"
        PRIVATE "firmware/TP5_IpGeN.X/system/reset"
        PRIVATE "firmware/TP5_IpGeN.X/system/tmr"
        PRIVATE "firmware/TP5_IpGeN.X/system"
        PRIVATE "firmware/TP5_IpGeN.X/tcpip"
        PRIVATE "firmware/TP5_IpGeN.X/tcpip/src"
        PRIVATE "firmware/TP5_IpGeN.X/driver/ethmac"
        PRIVATE "firmware/TP5_IpGeN.X/driver/ethmac/src"
        PRIVATE "firmware/TP5_IpGeN.X/driver/ethmac/src/dynamic"
        PRIVATE "firmware/TP5_IpGeN.X/driver/ethphy"
        PRIVATE "firmware/TP5_IpGeN.X/driver/ethphy/src"
        PRIVATE "firmware/TP5_IpGeN.X/driver/ethphy/src/dynamic"
        PRIVATE "firmware/TP5_IpGeN.X/tcpip/src/common"
        PRIVATE "firmware/TP5_IpGeN.X/tcpip/src/system/drivers"
        PRIVATE "firmware/TP5_IpGeN.X/system_config/pic32mx_eth_sk2"
        PRIVATE "firmware/TP5_IpGeN.X/pic32mx_eth_sk2"
        PRIVATE "firmware/TP5_IpGeN.X/system_config/pic32mx_eth_sk2/framework"
        PRIVATE "firmware/TP5_IpGeN.X/system_config/pic32mx_eth_sk2/bsp"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/system_config/pic32mx_eth_sk2"
        PRIVATE "firmware/src/pic32mx_eth_sk2"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../framework"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/system_config/pic32mx_eth_sk2/framework"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../bsp/pic32mx_skes"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/TP5_IpGeN.X")
endfunction()
function(TP5_IpGeN_config_1_config_1_toolchain_compile_cpp_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-mprocessor=32MX795F512L"
        "-frtti"
        "-fexceptions"
        "-fno-check-new"
        "-fenforce-eh-specs"
        "-ffunction-sections"
        "-O1"
        "-fno-common"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32MX_DFP/1.5.259")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_config_1=config_1")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/TP5_IpGeN.X")
endfunction()
function(TP5_IpGeN_config_1_link_rule target)
    set(options
        "-g"
        "${MP_EXTRA_LD_PRE}"
        "-mprocessor=32MX795F512L"
        "-Wl,--defsym=__MPLAB_BUILD=1${MP_EXTRA_LD_POST},--defsym=_min_heap_size=44960,--gc-sections,--no-code-in-dinit,--no-dinit-in-serial-mem,-L${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/TP5_IpGeN.X,--memorysummary,memoryfile.xml"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32MX_DFP/1.5.259")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_config_1=config_1")
endfunction()
