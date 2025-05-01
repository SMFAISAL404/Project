temperature_data = []

# Read the temperature data from the CSV file
with open("temperature.csv", "r") as file:
    lines = file.readlines()[1:]  # Skip the header row

    for line in lines:
        columns = line.strip().split(",")  # Split line by comma
        if len(columns) < 2:
            continue
        try:
            temperature = float(columns[1])  # The second column contains the temperature value
            temperature_data.append(temperature)
        except ValueError:
            continue  # Skip the line if the value is not a valid float

# Calculate total, minimum, and maximum temperature manually
if not temperature_data:
    print("No temperature data available to process.")
else:
    total_temp = 0
    min_temp = temperature_data[0]
    max_temp = temperature_data[0]

    for temp in temperature_data:
        total_temp += temp
        if temp < min_temp:
            min_temp = temp
        if temp > max_temp:
            max_temp = temp

    # Compute the average temperature
    average_temp = total_temp / len(temperature_data)

    # Output the results
    print("Average Temperature:", average_temp)
    print("Minimum Temperature:", min_temp)
    print("Maximum Temperature:", max_temp)
