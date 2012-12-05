import sys

print "#include <stdint.h>"

print "uint8_t z80_general_flags[256] = "
print "{"

# 7 6 5 4 3  2  1 0
# S Z X H X P/V N C
flag_c = 0x01
flag_n = 0x02
flag_p = 0x04
flag_v = 0x04
flag_h = 0x10
flag_z = 0x40
flag_s = 0x80

for i in range(256):
	flags = 0
	
	if i == 0:
		flags |= flag_z
	
	n = i
	c = 0
	while (n > 0):
		c += 1
		n &= n-1
	if (c & 1) == 0:
		flags |= flag_p

	if i & 0x80:
		flags |= flag_s
		
	#print (hex(flags) + ", ", end='')
	sys.stdout.write( hex(flags) )
	if i < 255:
		sys.stdout.write( ', ' )
		
	if (i & 0xF) == 0xF:
		sys.stdout.write( '\n' )

print "};"

