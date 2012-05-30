# Make file for compiling and loadable module HDMI


obj-$(CONFIG_AV7100)		+= av7100.o hdmi.o

ifdef CONFIG_HDMI_AV7100_DEBUG
EXTRA_CFLAGS += -DDEBUG
endif

clean-files := av7100.o hdmi.o built-in.o modules.order

