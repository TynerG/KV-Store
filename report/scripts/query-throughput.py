import matplotlib.pyplot as plt
import csv
import os

data_sizes = []
throughputs = []
time_taken = []

filename = "bin_search_get_throughput"

# read data from CSV
script_dir = os.path.dirname(os.path.abspath(__file__))
folder_path = os.path.join(script_dir, "../data")
print("path", folder_path)
with open(folder_path + f"/{filename}.csv", "r") as file:
    csv_reader = csv.reader(file)
    next(csv_reader)  # Skip the header
    for row in csv_reader:
        data_sizes.append(float(row[0]))  # Data Size
        throughputs.append(float(row[1]))  # Throughput
        time_taken.append(float(row[2]))  # Time Taken

# create the figure and axis objects
fig, ax1 = plt.subplots()

# plot throughput on the left y-axis
ax1.set_xlabel("Data Size (GB)")
ax1.set_ylabel("Throughput (KB/s)", color="b")
ax1.plot(
    data_sizes,
    throughputs,
    color="b",
    marker="o",
    linestyle="-",
    label="Throughput (KB/s)",
)
ax1.tick_params(axis="y", labelcolor="b")

# plot time taken
ax2 = ax1.twinx()
ax2.set_ylabel("Time Taken (s)", color="r")
ax2.plot(
    data_sizes,
    time_taken,
    color="r",
    marker="x",
    linestyle="--",
    label="Time Taken (seconds)",
)
ax2.tick_params(axis="y", labelcolor="r")

# add title and grid
plt.title("KV Store Throughput and Time Taken vs Data Size")
fig.tight_layout()

# save the plot
plt.savefig(folder_path + f"/{filename}.png")
