#######################################################################################################################
# Add the source files to SRCS_LIST
add_source_files(SRCS_LIST
	gqrx/gqrx.h
	gqrx/main.cpp
	gqrx/mainwindow.cpp
	gqrx/mainwindow.h
	gqrx/receiver.cpp
	gqrx/receiver.h
	gqrx/remote_control.cpp
	gqrx/remote_control.h
	gqrx/tcp_remote_control_server.cpp
	gqrx/tcp_remote_control_settings.cpp
	gqrx/tcp_remote_control_settings.h
	gqrx/file_resources.cpp
)

if(${ENABLE_SERIAL_REMOTE_CONTROL})
	add_source_files(SRCS_LIST
		gqrx/serial_remote_control_device.cpp
		gqrx/serial_remote_control_settings.cpp
	)
endif()

#######################################################################################################################
# Add the UI files to UI_SRCS_LIST
add_source_files(UI_SRCS_LIST
	gqrx/mainwindow.ui
	gqrx/tcp_remote_control_settings.ui
)

if(${ENABLE_SERIAL_REMOTE_CONTROL})
  add_source_files(UI_SRCS_LIST
  	gqrx/serial_remote_control_settings.ui
  )
endif()
