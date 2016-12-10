#!/usr/bin/env python3

# Tells me what page I should be up to by the end of the day.

from datetime import date

startDate = date(2016, 11, 27)
startPage = 90
pageChunk = 20 
# Down from 25. This is not conservative, you *must* do the 20 pages to keep up.
# This is page 844 - page 90, divided by 39 days (27th of Nov to 5th of Jan).

daysSinceStart = (date.today() - startDate).days
target = startPage + daysSinceStart * pageChunk
print(target)
