#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt
x = np.array([16, 32, 64, 128, 256, 512, 1024])

# THROUGHPUT
y = np.array([110.83, 385.44, 649.82, 1605.21, 1518.92, 1550.82, 1564.22])
e = np.array([7.08, 192.41, 151.28, 733.40, 624.91, 609.30, 685.67])

plt.xlabel('Data Rate (Kbps)')
plt.ylabel('Data Rate (Kbps)')
# plt.axis([16, 1024, 0, 1650])
plt.errorbar(x, y, e, linestyle='None', marker='^')
plt.show()

# DELAY
y = np.array([.0157, .089, .066, 0.098, 0.074, 0.089, 0.072])
e = np.array([0.040, 0.043, 0.020, 0.049, 0.035, 0.038, 0.026])


plt.xlabel('Data Rate (Kbps)')
plt.ylabel('Data Rate (Kbps)')
# plt.axis([16, 1024, 0, 1650])
plt.errorbar(x, y, e, linestyle='None', marker='^')
plt.show()

# JITTER
y = np.array([0.015, 0.014, 0.004, 0.018, 0.008, 0.014, 0.007])
e = np.array([0.013, 0.018, 0.009, 0.021, 0.015, 0.027, 0.011])

plt.xlabel('Data Rate (Kbps)')
plt.ylabel('Data Rate (Kbps)')
# plt.axis([16, 1024, 0, 1650])
plt.errorbar(x, y, e, linestyle='None', marker='^')
plt.show()


