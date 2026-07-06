set(PFP_NAME powerFlowPlugin)

file(GLOB PFP_SOURCES   ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB PFP_INCS      ${CMAKE_CURRENT_LIST_DIR}/src/*.h)
file(GLOB PFP_INC_GUI   ${NATID_SDK_INC}/gui/*.h)
file(GLOB PFP_INC_TD    ${NATID_SDK_INC}/td/*.h)
file(GLOB PFP_INC_CNT   ${NATID_SDK_INC}/cnt/*.h)
file(GLOB PFP_INC_MU    ${NATID_SDK_INC}/mu/*.h)
file(GLOB PFP_INC_MEM   ${NATID_SDK_INC}/mem/*.h)
file(GLOB PFP_INC_FO    ${NATID_SDK_INC}/fo/*.h)
file(GLOB PFP_INC_SC    ${NATID_SDK_INC}/sc/*.h)
file(GLOB PFP_INC_ARCH  ${NATID_SDK_INC}/arch/*.h)
file(GLOB PFP_INC_SYST  ${NATID_SDK_INC}/syst/*.h)
file(GLOB PFP_INC_DENSE  ${NATID_SDK_INC}/dense/*.h)
file(GLOB PFP_INC_SPARSE ${NATID_SDK_INC}/sparse/*.h)

# Plugin is a SHARED library (DLL on Windows)
add_library(${PFP_NAME} SHARED
    ${PFP_SOURCES}
    ${PFP_INCS}
    ${PFP_INC_GUI}
    ${PFP_INC_TD}
    ${PFP_INC_CNT}
    ${PFP_INC_MU}
    ${PFP_INC_MEM}
    ${PFP_INC_FO}
    ${PFP_INC_SC}
    ${PFP_INC_ARCH}
    ${PFP_INC_SYST}
    ${PFP_INC_DENSE}
    ${PFP_INC_SPARSE}
)

source_group("src"           FILES ${PFP_SOURCES})
source_group("inc"           FILES ${PFP_INCS})
source_group("inc\\gui"      FILES ${PFP_INC_GUI})
source_group("inc\\td"       FILES ${PFP_INC_TD})
source_group("inc\\cnt"      FILES ${PFP_INC_CNT})
source_group("inc\\mu"       FILES ${PFP_INC_MU})
source_group("inc\\mem"      FILES ${PFP_INC_MEM})
source_group("inc\\fo"       FILES ${PFP_INC_FO})
source_group("inc\\sc"       FILES ${PFP_INC_SC})
source_group("inc\\arch"     FILES ${PFP_INC_ARCH})
source_group("inc\\syst"     FILES ${PFP_INC_SYST})
source_group("inc\\dense"    FILES ${PFP_INC_DENSE})
source_group("inc\\sparse"   FILES ${PFP_INC_SPARSE})

target_link_libraries(${PFP_NAME}
    debug     ${MU_LIB_DEBUG}     optimized ${MU_LIB_RELEASE}
    debug     ${MATRIX_LIB_DEBUG} optimized ${MATRIX_LIB_RELEASE}
    debug     ${NATGUI_LIB_DEBUG} optimized ${NATGUI_LIB_RELEASE}
)

target_compile_definitions(${PFP_NAME} PUBLIC PLUGIN_EXPORTS)

if (WIN32)
    target_compile_definitions(${PFP_NAME} PRIVATE _USE_MATH_DEFINES)
endif()

setIDEPropertiesForLib(${PFP_NAME})
