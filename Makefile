# Theos makefile to build this tool onto iOS

TARGET := iphone:clang:15.5:15.0

include $(THEOS)/makefiles/common.mk

TOOL_NAME = allemande

$(TOOL_NAME)_FILES = main.cpp
$(TOOL_NAME)_CFLAGS = -fobjc-arc
$(TOOL_NAME)_CCFLAGS = -std=c++20
$(TOOL_NAME)_CODESIGN_FLAGS = -Sentitlements.plist
$(TOOL_NAME)_INSTALL_PATH = /usr/local/bin

include $(THEOS_MAKE_PATH)/tool.mk
