#!/usr/bin/env python3

# Tells me what page I should be up to by the end of the day.

from datetime import date
from sys import argv
from math import ceil

startDate = date(2016, 11, 27)
startPage = 90
endDate = date(2017, 1, 5)
endPage = 844

# I stopped for a few days and fell too far behind, so this
# is a revised schedule which omits the last 2 chapters.
if len(argv) > 1:
    if argv[1] == "revised" or argv[1] == "rev":
        startDate = date(2016, 12, 18)
        startPage = 405
        endPage = 742

# We calculate the pageChunk now, instead of specifying it in advance.
# It should still be 20 regardless of which of the dates/pages you choose.
pageChunk = ceil((endPage - startPage) / (endDate - startDate).days)
daysSinceStart = (date.today() - startDate).days
target = startPage + daysSinceStart * pageChunk
print(target)
