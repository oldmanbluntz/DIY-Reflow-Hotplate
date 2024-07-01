import numpy as np

# Create data points for each phase of the curve

# Ramp up to 120°C over 2 minutes (480 data points)
ramp_up_to_120 = np.linspace(0, 120, 480)

# Hold at 120°C for 2 minutes (480 data points)
hold_at_120 = np.full(480, 120)

# Ramp up to 200°C over 2 minutes (480 data points)
ramp_up_to_200 = np.linspace(150, 200, 480)

# Hold at 200°C for 5 minutes (600 data points)
hold_at_200 = np.full(600, 200)

# Combine all data points
curve_data = np.concatenate((ramp_up_to_120, hold_at_120, ramp_up_to_200, hold_at_200))

# Write the curve data to a text file
file_path = 'curve.txt'
with open(file_path, 'w') as file:
    for value in curve_data:
        file.write(f"{value:.2f}\n")

print(f"curve.txt file has been created at {file_path}")
