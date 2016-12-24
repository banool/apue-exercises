#!/usr/bin/env python3

# Tells me what page I should be up to by the end of the day.
# Can take an offset. Will not check for silly offsets like -1000.

from datetime import date
from sys import argv, exit
from math import ceil

def printUsage():
	print("USAGE: {} <schedule or offset>".format(argv[0]))
	print("  <schedule>: \"rev\" or \"orig\". \"rev\" is default.")
	print("  <offset>:   +n or -n. n must be an integer")
	exit(1)

def getOffset(arg1):
	try:
		offset = int(arg1[1:])
		if arg1[0] == "-":
			offset = -offset
	except:
		printUsage()
	return offset

endDate = date(2017, 1, 5)
offset = 0

# I stopped for a few days and fell too far behind, so this
# is a revised schedule which omits the last 2 chapters.
if len(argv) == 1:
    argv.append("rev")
if len(argv) not in (1,2):
	printUsage()

if "+" in argv[1] or "-" in argv[1]:
	offset = getOffset(argv[1])
	argv[1] = "rev"

if argv[1] == "revised" or argv[1] == "rev":
    startDate = date(2016, 12, 18)
    startPage = 405
    endPage = 742
elif argv[1] == "original" or argv[1] == "orig":
	startDate = date(2016, 11, 27)
	startPage = 90
	endPage = 844
else:
    printUsage()

# We calculate the pageChunk now, instead of specifying it in advance.
# It should still be 20 regardless of which of the dates/pages you choose.
pageChunk = ceil((endPage - startPage) / (endDate - startDate).days)
daysSinceStart = (date.today() - startDate).days + offset
target = startPage + daysSinceStart * pageChunk
print(target)
