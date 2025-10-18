import numpy as np
import matplotlib.pyplot as plt

# Parameters
x0, y0 = 0, 0
radius = 12

# Grid
x = np.linspace(-radius, radius, 200)
y = np.linspace(-radius, radius, 200)
X, Y = np.meshgrid(x, y)
r = np.sqrt((X - x0)**2 + (Y - y0)**2)

# Lenia bell kernel
W = np.zeros_like(r)
mask = (r > 0) & (r <= radius)  # avoid division by zero
u = r[mask] / radius
W[mask] = np.exp((1-u**2)**3)

# Handle center separately (optional)
W[r == 0] = W[mask].max()  # assign max value at center

# Normalize kernel to [0,1]
W /= W.max()

# Plot heatmap
plt.imshow(W, extent=[-radius, radius, -radius, radius],
           origin='lower', cmap='viridis')
plt.colorbar(label='Kernel Strength')
plt.title("Normalized Lenia Bell Kernel")
plt.show()
