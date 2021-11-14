LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc

MODULE_SRCS += \
	$(LOCAL_DIR)/CH57x_adc.c \
	$(LOCAL_DIR)/CH57x_clk.c \
	$(LOCAL_DIR)/CH57x_flash.c \
	$(LOCAL_DIR)/CH57x_gpio.c \
	$(LOCAL_DIR)/CH57x_int.c \
	$(LOCAL_DIR)/CH57x_lcd.c \
	$(LOCAL_DIR)/CH57x_pwm.c \
	$(LOCAL_DIR)/CH57x_pwr.c \
	$(LOCAL_DIR)/CH57x_spi0.c \
	$(LOCAL_DIR)/CH57x_spi1.c \
	$(LOCAL_DIR)/CH57x_sys.c \
	$(LOCAL_DIR)/CH57x_timer0.c \
	$(LOCAL_DIR)/CH57x_timer1.c \
	$(LOCAL_DIR)/CH57x_timer2.c \
	$(LOCAL_DIR)/CH57x_timer3.c \
	$(LOCAL_DIR)/CH57x_uart0.c \
	$(LOCAL_DIR)/CH57x_uart1.c \
	$(LOCAL_DIR)/CH57x_uart2.c \
	$(LOCAL_DIR)/CH57x_uart3.c \
	$(LOCAL_DIR)/CH57x_usbdev.c \
#	$(LOCAL_DIR)/CH57x_usbhostBase.c \
#	$(LOCAL_DIR)/CH57x_usbhostClass.c

MODULE_COMPILEFLAGS += -Wno-error

include make/module.mk