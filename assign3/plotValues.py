#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt

# THROUGHPUT
x = np.array([16, 32, 64, 128, 256])
y = np.array([89.85, 110.83, 385.44, 649.82, 1605.21])
e = np.array([1.74, 7.08, 192.41, 151.28, 733.40])

plt.xlabel('Data Rate (Kbps)')
plt.ylabel('Data Rate (Kbps)')
# plt.axis([16, 1024, 0, 1650])
plt.errorbar(x, y, e, linestyle='None', marker='^')
plt.show()

# DELAY
x = np.array([16, 32, 64, 128, 256])
y = np.array([])
e = np.array([])

plt.xlabel('Data Rate (Kbps)')
plt.ylabel('Data Rate (Kbps)')
# plt.axis([16, 1024, 0, 1650])
plt.errorbar(x, y, e, linestyle='None', marker='^')
plt.show()

# JITTER
x = np.array([16, 32, 64, 128, 256])
y = np.array([])
e = np.array([])

plt.xlabel('Data Rate (Kbps)')
plt.ylabel('Data Rate (Kbps)')
# plt.axis([16, 1024, 0, 1650])
plt.errorbar(x, y, e, linestyle='None', marker='^')
plt.show()


