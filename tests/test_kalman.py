import matplotlib.pyplot as plt
import pandas as pd
import time
import os

df = pd.read_csv('test_kalman.csv')

plt, axes = plt.subplots(2, 2)
x_plot, dx_plot, y_plot, dy_plot = axes[0][0], axes[0][1], axes[1][0], axes[1][1]

x_plot.plot(df['x_'])
x_plot.plot(df['x'])
x_plot.plot(df['x_real'])
x_plot.legend(['$\widetilde{x}$', '$\hat{x}$', 'x'])
x_plot.set_xlabel('step')
x_plot.set_ylabel('position (x)')
x_plot.set_title('Position (x) tracking')

dx_plot.plot(df['dx_'])
dx_plot.plot(df['dx'])
dx_plot.plot(df['dx_real'])
dx_plot.legend(['$\widetilde{x\'}$', '$\hat{x}\'$', 'x\''])
dx_plot.set_xlabel('step')
dx_plot.set_ylabel('velocity (x)')
dx_plot.set_title('Velocity (x) tracking')

y_plot.plot(df['y_'])
y_plot.plot(df['y'])
y_plot.plot(df['y_real'])
y_plot.legend(['$\widetilde{y}$', '$\hat{y}$', 'y'])
y_plot.set_xlabel('step')
y_plot.set_ylabel('position (y)')
y_plot.set_title('Position (y) tracking')

dy_plot.plot(df['dy_'])
dy_plot.plot(df['dy'])
dy_plot.plot(df['dy_real'])
dy_plot.legend(['$\widetilde{y\'}$', '$\hat{y}$\'', 'y\''])
dy_plot.set_xlabel('step')
dy_plot.set_ylabel('velocity (y)')
dy_plot.set_title('Velocity (y) tracking')

plt.show()