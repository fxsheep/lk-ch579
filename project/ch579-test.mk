LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := ch579

MODULES += \
	dev/gpio

include project/virtual/test.mk

