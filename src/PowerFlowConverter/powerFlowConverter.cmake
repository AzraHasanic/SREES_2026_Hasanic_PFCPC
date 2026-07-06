set(PFC_NAME powerFlowConverter)
 
file(GLOB PFC_SOURCES  ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB PFC_INCS     ${CMAKE_CURRENT_LIST_DIR}/src/*.h)
set(PFC_PLIST          ${CMAKE_CURRENT_LIST_DIR}/src/Info.plist)
file(GLOB PFC_INC_TD      ${NATID_SDK_INC}/td/*.h)
file(GLOB PFC_INC_GUI     ${NATID_SDK_INC}/gui/*.h)
file(GLOB PFC_INC_SPARSE  ${NATID_SDK_INC}/sparse/*.h)
file(GLOB PFC_INC_DENSE   ${NATID_SDK_INC}/dense/*.h)
 
add_executable(${PFC_NAME}
    ${PFC_INCS}
    ${PFC_SOURCES}
    ${PFC_INC_TD}
    ${PFC_INC_GUI}
    ${PFC_INC_SPARSE}
    ${PFC_INC_DENSE}
)

if (WIN32)
    target_compile_definitions(${PFC_NAME} PRIVATE _USE_MATH_DEFINES)
endif()
 
source_group("inc"          FILES ${PFC_INCS})
source_group("src"          FILES ${PFC_SOURCES})
source_group("inc\\td"      FILES ${PFC_INC_TD})
source_group("inc\\gui"     FILES ${PFC_INC_GUI})
source_group("inc\\sparse"  FILES ${PFC_INC_SPARSE})
source_group("inc\\dense"   FILES ${PFC_INC_DENSE})
 
target_link_libraries(${PFC_NAME}
    debug     ${MU_LIB_DEBUG}
    debug     ${NATGUI_LIB_DEBUG}
    debug     ${MATRIX_LIB_DEBUG}
    optimized ${MU_LIB_RELEASE}
    optimized ${NATGUI_LIB_RELEASE}
    optimized ${MATRIX_LIB_RELEASE}
)
 
setTargetPropertiesForGUIApp(${PFC_NAME} ${PFC_PLIST})
setIDEPropertiesForGUIExecutable(${PFC_NAME} ${CMAKE_CURRENT_LIST_DIR})
setPlatformDLLPath(${PFC_NAME})
