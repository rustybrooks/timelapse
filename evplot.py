#!/usr/bin/python

import sys
import matplotlib.pyplot as pylab
import sqlite3
import math

conn = sqlite3.connect(sys.argv[1])

w = 30

count = 0
counts = []
ev = []
ev2 = []
ev3 = []
with conn:
    c = conn.cursor()

    sql = "select time, measured_ev, aperture, exposure_time, iso from photos"
    for row in c.execute(sql):
        counts.append(count)
        ev.append(row[1])

        val = math.log((row[2]**2) / (row[3]*row[4]/100.0))/math.log(2)
        ev2.append(val)

        if (count > w):
            ev3.append(sum(ev[-w:-1]) / len(ev[-w:-1]))
        else:
            ev3.append(row[1])

        count += 1

pylab.plot(counts, ev)
pylab.plot(counts, ev2)
pylab.plot(counts, ev3)
pylab.show()
