add_definitions (-DQT3_SUPPORT -DQT_CLEAN_NAMESPACE)
#add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
add_subdirectory( config )

########### next target ###############

set(kwin3_fitz_PART_SRCS
   client.cpp
   button.cpp
   factory.cpp
   buttoncache.cpp
   fakemouse.cpp
   )


kde4_add_plugin(kwin3_fitz ${kwin3_fitz_PART_SRCS})

target_link_libraries(kwin3_fitz ${KDE4_KDEUI_LIBS} kdecorations ${QT_QT3SUPPORT_LIBRARY} )

install(TARGETS kwin3_fitz  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES fitz.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kwin/ )

