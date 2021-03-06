CFLAGS  = -V --Werror -mmcs51 --model-large --xram-size 0x1800 --xram-loc 0x0000 --code-size 0xf000 --stack-auto
CC      = sdcc
FLASHER = ./CH55x_python_flasher/chflasher.py
TARGET  = hujico
OBJS	= hujico.rel dual_hid.rel jvs.rel JVSIO_c.rel adc.rel ch559.rel led.rel rs485.rel timer3.rel usb_device.rel

all: $(TARGET).bin

program: all
	$(FLASHER) -w -f $(TARGET).bin

run: program
	$(FLASHER) -s

clean:
	rm -f *.asm *.lst *.rel *.rst *.sym $(TARGET).bin $(TARGET).ihx $(TARGET).lk $(TARGET).map $(TARGET).mem

%.rel: %.c chlib/*.h *.h
	$(CC) -c $(CFLAGS) $<

%.rel: chlib/%.c chlib/*.h
	$(CC) -c $(CFLAGS) $<

JVSIO_c.rel: jvsio/JVSIO_c.c jvsio/JVSIO_c.h chlib/*.h
	$(CC) -c $(CFLAGS) -I chlib $<

$(TARGET).ihx: $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.bin: %.ihx
	sdobjcopy -I ihex -O binary $< $@
