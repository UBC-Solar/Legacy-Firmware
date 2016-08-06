#!/usr/bin/python

import sys

if len(sys.argv) < 2:
    print("Usage: " + sys.argv[0] + " <can bus log file>")
    exit(1)

f = open(sys.argv[1], "rb")

try:
	while True:
		year = ord(f.read(1)[0])
		month = ord(f.read(1)[0])
		day = ord(f.read(1)[0])
		hour = ord(f.read(1)[0])
		minute = ord(f.read(1)[0])
		second = ord(f.read(1)[0])

		id = 0
		for i in range(0,4):
			id += ord(f.read(1)[0]) << (i * 8)

		length = ord(f.read(1)[0])

		data = f.read(length)

		print "[%02d/%02d/%02d %02d:%02d:%02d][%d] %s" % (year, month, day, hour, minute, second, id, " ".join("{:02x}".format(ord(c)) for c in data))
except IndexError: #end of file
	pass