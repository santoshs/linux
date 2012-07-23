
%.o : %.s
	$(AS) $(ASFLAGS) $< -o $(@F)

%.o : %.c
	$(CC) $(CCFLAGS) -c -o $(@F) $<

$(ULIB): $(OBJS)
	$(AR) $(AROPT) $(ULIB) $(OBJS)
	cp ./$(ULIB) $(VIA_DIR)/../libs/

clean:
	$(RM) $(OBJS)

allclean:
	$(RM) $(VIA_DIR)/../libs/$(ULIB)
	$(RM) $(ULIB) $(OBJS)
