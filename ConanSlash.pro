# -------------------------------------------------
# Project created by QtCreator 2010-06-13T04:26:52
# -------------------------------------------------
TARGET = ConanSlash
QT += network sql declarative
TEMPLATE = app
win32 : RC_FILE = resource/icon.rc
CONFIG += warn_on audio qaxcontainer

# If you want to enable joystick support, please uncomment the following line:
# CONFIG += joystick
# However, joystick is not supported under Mac OS X temporarily


SOURCES += \
	src/main.cpp \
	src/client/aux-skills.cpp \
	src/client/client.cpp \
	src/client/clientplayer.cpp \
	src/client/clientstruct.cpp \
	src/core/card.cpp \
	src/core/engine.cpp \
	src/core/general.cpp \
	src/core/lua-wrapper.cpp \
	src/core/player.cpp \
	src/core/settings.cpp \
	src/core/skill.cpp \
	src/core/statistics.cpp \
	src/core/banpair.cpp \
	src/dialog/cardeditor.cpp \
	src/dialog/cardoverview.cpp \
	src/dialog/choosegeneraldialog.cpp \
	src/dialog/configdialog.cpp \
	src/dialog/connectiondialog.cpp \
	src/dialog/distanceviewdialog.cpp \
	src/dialog/generaloverview.cpp \
	src/dialog/mainwindow.cpp \
	src/dialog/packagingeditor.cpp \
	src/dialog/playercarddialog.cpp \
	src/dialog/roleassigndialog.cpp \
	src/dialog/scenario-overview.cpp \
	src/dialog/halldialog.cpp \
	src/package/package.cpp \
	src/package/exppattern.cpp \
	src/package/standard.cpp \
	src/package/standard-cards.cpp \
	src/package/maneuvering.cpp \
	src/package/standard-generals.cpp \
	src/package/wind.cpp \
	src/package/thicket.cpp \
	src/package/fire.cpp \
	src/package/mountain.cpp \
	src/package/negative.cpp \
	src/package/thunder.cpp \
	src/package/god.cpp \
	src/package/joy.cpp \
	src/scenario/boss-mode-scenario.cpp \
	src/scenario/couple-scenario.cpp \
	src/scenario/scenario.cpp \
	src/scenario/scenerule.cpp \
	src/scenario/zombie-mode-scenario.cpp \
	src/server/ai.cpp \
	src/server/contestdb.cpp \
	src/server/gamerule.cpp \
        src/server/generalselector.cpp \
	src/server/room.cpp \
	src/server/roomthread.cpp \
	src/server/roomthread1v1.cpp \
	src/server/roomthread3v3.cpp \
	src/server/server.cpp \
	src/server/serverplayer.cpp \
	src/ui/button.cpp \
	src/ui/cardcontainer.cpp \
	src/ui/carditem.cpp \
	src/ui/clientlogbox.cpp \
	src/ui/dashboard.cpp \
	src/ui/indicatoritem.cpp \
	src/ui/photo.cpp \
	src/ui/pixmap.cpp \
	src/ui/rolecombobox.cpp \
	src/ui/roomscene.cpp \
	src/ui/startscene.cpp \
	src/ui/window.cpp \
	src/util/detector.cpp \
	src/util/nativesocket.cpp \
	src/util/recorder.cpp \
	src/lua/print.c \
	src/lua/lzio.c \
	src/lua/lvm.c \
	src/lua/lundump.c \
	src/lua/ltm.c \
	src/lua/ltablib.c \
	src/lua/ltable.c \
	src/lua/lstrlib.c \
	src/lua/lstring.c \
	src/lua/lstate.c \
	src/lua/lparser.c \
	src/lua/loslib.c \
	src/lua/lopcodes.c \
	src/lua/lobject.c \
	src/lua/loadlib.c \
	src/lua/lmem.c \
	src/lua/lmathlib.c \
	src/lua/llex.c \
	src/lua/liolib.c \
	src/lua/linit.c \
	src/lua/lgc.c \
	src/lua/lfunc.c \
	src/lua/ldump.c \
	src/lua/ldo.c \
	src/lua/ldebug.c \
	src/lua/ldblib.c \
	src/lua/lcode.c \
	src/lua/lbaselib.c \
	src/lua/lauxlib.c \
	src/lua/lapi.c \
	swig/sanguosha_wrap.cxx

HEADERS += src/client/aux-skills.h \
	src/client/client.h \
	src/client/clientplayer.h \
	src/client/clientstruct.h \
	src/core/audio.h \
	src/core/card.h \
	src/core/engine.h \
	src/core/general.h \
	src/core/lua-wrapper.h \
	src/core/player.h \
	src/core/settings.h \
	src/core/skill.h \
	src/core/statistics.h \
	src/core/banpair.h \
	src/dialog/cardeditor.h \
	src/dialog/cardoverview.h \
	src/dialog/choosegeneraldialog.h \
	src/dialog/configdialog.h \
	src/dialog/connectiondialog.h \
	src/dialog/distanceviewdialog.h \
	src/dialog/generaloverview.h \
	src/dialog/generalselector.h \
	src/dialog/halldialog.h \
	src/dialog/mainwindow.h \
	src/dialog/packagingeditor.h \
	src/dialog/playercarddialog.h \
	src/dialog/roleassigndialog.h \ 
	src/dialog/scenario-overview.h \
	src/package/exppattern.h \
	src/package/package.h \
	src/package/standard.h \
	src/package/standard-equips.h \
	src/package/standard-generals.h \
	src/package/maneuvering.h \
	src/package/wind.h \
	src/package/thicket.h \
	src/package/fire.h \
	src/package/mountain.h \
	src/package/negative.h \
	src/package/thunder.h \
	src/package/joy.h \
	src/package/god.h \
	src/scenario/boss-mode-scenario.h \
	src/scenario/couple-scenario.h \
	src/scenario/scenario.h \
	src/scenario/scenerule.h \
	src/scenario/zombie-mode-scenario.h \
	src/server/ai.h \
	src/server/contestdb.h \
	src/server/gamerule.h \
	src/server/room.h \
	src/server/roomthread.h \
	src/server/roomthread1v1.h \
	src/server/roomthread3v3.h \
	src/server/server.h \
	src/server/serverplayer.h \
	src/server/structs.h \
	src/ui/button.h \
	src/ui/cardcontainer.h \
	src/ui/carditem.h \
	src/ui/clientlogbox.h \
	src/ui/dashboard.h \
	src/ui/indicatoritem.h \
	src/ui/photo.h \
	src/ui/pixmap.h \
	src/ui/rolecombobox.h \
	src/ui/roomscene.h \
	src/ui/startscene.h \
	src/ui/window.h \
	src/util/detector.h \
	src/util/nativesocket.h \
	src/util/recorder.h \
	src/util/socket.h \
	src/lua/lzio.h \
	src/lua/lvm.h \
	src/lua/lundump.h \
	src/lua/lualib.h \
	src/lua/luaconf.h \
	src/lua/lua.hpp \
	src/lua/lua.h \
	src/lua/ltm.h \
	src/lua/ltable.h \
	src/lua/lstring.h \
	src/lua/lstate.h \
	src/lua/lparser.h \
	src/lua/lopcodes.h \
	src/lua/lobject.h \
	src/lua/lmem.h \
	src/lua/llimits.h \
	src/lua/llex.h \
	src/lua/lgc.h \
	src/lua/lfunc.h \
	src/lua/ldo.h \
	src/lua/ldebug.h \
	src/lua/lcode.h \
	src/lua/lauxlib.h \
	src/lua/lapi.h
	
FORMS += src/dialog/cardoverview.ui \
	src/dialog/configdialog.ui \
	src/dialog/connectiondialog.ui \
	src/dialog/generaloverview.ui \
	src/dialog/mainwindow.ui 
	
INCLUDEPATH += include
INCLUDEPATH += src/client
INCLUDEPATH += src/core
INCLUDEPATH += src/dialog
INCLUDEPATH += src/package
INCLUDEPATH += src/scenario
INCLUDEPATH += src/server
INCLUDEPATH += src/ui
INCLUDEPATH += src/util
INCLUDEPATH += src/lua

LIBS += -L. -lm

TRANSLATIONS += conanslash.ts

CONFIG(audio){
	DEFINES += AUDIO_SUPPORT
	INCLUDEPATH += include/fmod
	LIBS += -lfmodex
	SOURCES += src/core/audio.cpp
}

CONFIG(joystick){
	DEFINES += JOYSTICK_SUPPORT
	HEADERS += src/ui/joystick.h
	SOURCES += src/ui/joystick.cpp
	win32: LIBS += -lplibjs -lplibul -lwinmm
	unix: LIBS += -lplibjs -lplibul
}
