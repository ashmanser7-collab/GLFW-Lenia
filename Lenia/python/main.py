import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import convolve2d

# -----------------------
# Parameters
# -----------------------
size = 100      # Grid size
T = 200         # Number of iterations
dt = 0.1        # Time step
R = 8           # Kernel radius (larger for smoother patterns)
growth = 0.1    # Growth factor
sigma = 2.0     # Kernel smoothness

# -----------------------
# Initialize grid
# -----------------------
A = np.random.rand(size, size) * 0.2  # Mostly low values
A[size//2-5:size//2+5, size//2-5:size//2+5] = 0.8  # Seed in center

# -----------------------
# Kernel function (ring-shaped for motion)
# -----------------------
def kernel(r, R, sigma):
    """Ring-shaped kernel to encourage movement"""
    return np.exp(-((r-R/2)**2)/(2*sigma**2))

# Precompute kernel
x = np.arange(-R, R+1)
y = np.arange(-R, R+1)
X, Y = np.meshgrid(x, y)
r = np.sqrt(X**2 + Y**2)
K = kernel(r, R, sigma)
K /= K.sum()  # Normalize

# -----------------------
# Growth function
# -----------------------
def G(u):
    """Smooth growth map"""
    return 2 * u * (1 - u) - 0.05  # Subtract small constant for decay

# -----------------------
# Simulation loop
# -----------------------
plt.ion()
fig, ax = plt.subplots(figsize=(6,6))

for t in range(T):
    U = convolve2d(A, K, mode='same', boundary='wrap')
    A += dt * G(U)
    A = np.clip(A, 0, 1)

    # Display
    if t % 5 == 0:
        ax.clear()
        ax.imshow(A, cmap='inferno', interpolation='bilinear')
        ax.set_title(f"Iteration {t}")
        ax.axis('off')
        plt.pause(0.01)

plt.ioff()
plt.show()
