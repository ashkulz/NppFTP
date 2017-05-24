NPPFTP_SRC = \
    src/DragDropSupport.cpp \
    src/Encryption.cpp \
    src/FileObject.cpp \
    src/FTPCache.cpp \
    src/FTPClientWrapper.cpp \
    src/FTPClientWrapperSSH.cpp \
    src/FTPClientWrapperSSL.cpp \
    src/FTPProfile.cpp \
    src/FTPQueue.cpp \
    src/FTPSession.cpp \
    src/FTPSettings.cpp \
    src/Monitor.cpp \
    src/NppFTP.cpp \
    src/Output.cpp \
    src/PathUtils.cpp \
    src/PluginInterface.cpp \
    src/ProgressMonitor.cpp \
    src/QueueOperation.cpp \
    src/RefObject.cpp \
    src/SSLCertificates.cpp \
    src/StringUtils.cpp \
    src/WinPlatform.cpp

NPPWIN_SRC = \
    src/Windows/AboutDialog.cpp \
    src/Windows/ChildDialog.cpp \
    src/Windows/Dialog.cpp \
    src/Windows/DockableWindow.cpp \
    src/Windows/DragDropWindow.cpp \
    src/Windows/FTPWindow.cpp \
    src/Windows/InputDialog.cpp \
    src/Windows/KBIntDialog.cpp \
    src/Windows/MessageDialog.cpp \
    src/Windows/OutputWindow.cpp \
    src/Windows/ProfilesDialog.cpp \
    src/Windows/QueueWindow.cpp \
    src/Windows/SettingsDialog.cpp \
    src/Windows/Toolbar.cpp \
    src/Windows/Treeview.cpp \
    src/Windows/Window.cpp

TINYXML_SRC = \
    tinyxml/src/tinystr.cpp \
    tinyxml/src/tinyxml.cpp \
    tinyxml/src/tinyxmlerror.cpp \
    tinyxml/src/tinyxmlparser.cpp

UTCP_SRC = \
    UTCP/src/ftp_c.cpp \
    UTCP/src/uh_ctrl.cpp \
    UTCP/src/UTDataSource.cpp \
    UTCP/src/utfile.cpp \
    UTCP/src/utstrlst.cpp \
    UTCP/src/ut_clnt.cpp \
    UTCP/src/UT_Queue.cpp \
    UTCP/src/UT_StrOp.cpp

OBJECTS_REL = $(TINYXML_SRC:tinyxml/src/=obj/release/) $(UTCP_SRC:UTCP/src/=obj/release/) \
              $(NPPFTP_SRC:src/=obj/release/) $(NPPWIN_SRC:src/Windows/=obj/release/) \
              obj/release/NppFTP.res

OBJECTS_DBG = $(TINYXML_SRC:tinyxml/src/=obj/debug/) $(UTCP_SRC:UTCP/src/=obj/debug/) \
              $(NPPFTP_SRC:src/=obj/debug/) $(NPPWIN_SRC:src/Windows/=obj/debug/) \
              obj/debug/NppFTP.res
