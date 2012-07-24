#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------
ifeq ($(DUPDATER),UPDATER)
	TARGETBIN = updater
else
	TARGETBIN = $(TARGET)
endif


%.o : %.s
	$(AS) $(ASFLAGS) $< -o $(@F)

%.o : %.c
	$(CC) $(CCFLAGS) -c -o $(@F) $<

$(TARGET).axf : $(OBJS) $(LIBS) $(SCATER)
	$(LD)  $(LDFLAGS) -o $(TARGET).axf $(OBJS)  -T $(SCATER) -Map $(TARGET).map --start-group $(LIBS) --end-group

$(TARGET).bin : $(TARGET).axf
	$(CONV) -O srec $(TARGET).axf $(TARGET).srec
	$(CONV) --gap-fill=0xff -O binary $(TARGET).axf $(TARGET).bin
	cp $(TARGET).bin $(VIA_DIR)/../bin/$(TARGETBIN).bin

clean:
	$(RM) $(OBJS)

allclean:
	$(RM) $(VIA_DIR)/../bin/$(TARGETBIN).bin
	$(RM) $(TARGET).axf $(TARGET).bin $(TARGET).map $(TARGET).srec $(OBJS)
