


########### next target ###############

set(kwin_fitz_config_PART_SRCS config.cpp )


kde4_add_ui_files(kwin_fitz_config_PART_SRCS configdialog.ui)

kde4_add_plugin(kwin_fitz_config ${kwin_fitz_config_PART_SRCS})



target_link_libraries(kwin_fitz_config  kdeui kio ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kwin_fitz_config  DESTINATION ${PLUGIN_INSTALL_DIR} )

