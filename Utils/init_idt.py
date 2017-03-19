for i in range(32):
	x = "(u32) isr" + str(i)+","
	y = "\tinit_idt_desc(" + str(i) + ","
	print(y, "0x08,", x, "INTGATE);")

