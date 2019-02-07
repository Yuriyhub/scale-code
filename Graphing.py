import matplotlib.pyplot as plt
import matplotlib.animation as animation

from Adafruit_IO import Client, Feed, Data, RequestError
import datetime


# Set to your Adafruit IO key.
# Remember, your key is a secret,
# so make sure not to publish it when you publish this code!
ADAFRUIT_IO_KEY = '13f2bbf99eb74adda6859b1216ac9835'

# Set to your Adafruit IO username.
# (go to https://accounts.adafruit.com to find your username)
ADAFRUIT_IO_USERNAME = 'YuriyD'

# Create an instance of the REST client.
aio = Client(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)
#try:
scale = aio.feeds('my-weight')
#except RequestError:
#    feed = Feed(name="scale")
#    scale = aio.create_feed(feed)

#data1 = aio.receive_next(scale.key)
#print(data1)



#data3 = aio.receive_previous(scale.key)
#print(data3)




# Parameters
x_len = 200         # Number of points to display
y_range = [0, 300]  # Range of possible Y values to display

# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = list(range(0, 200))
ys = [0] * x_len
ax.set_ylim(y_range)


# Initialize communication with TMP102
#tmp102.init()

data2 = aio.receive(scale.key).value
print(data2)
# Create a blank line. We will update the line in animate
line, = ax.plot(xs, ys)



# Add labels
plt.title('My Fat Body Over Time')
plt.xlabel('Samples')
plt.ylabel('Weight (kg)')

# This function is called periodically from FuncAnimation
def animate(i, ys):

    # Read temperature (Celsius) from TMP102
    #temp_c = round(tmp102.read_temp(), 2)

    # Add y to list
    data2 = aio.receive(scale.key).value
    print(data2)
    ys.append(data2)

    # Limit y list to set number of items
    ys = ys[-x_len:]

    # Update line with new Y values
    line.set_ydata(ys)

    return line,

# Set up plot to call animate() function periodically
ani = animation.FuncAnimation(fig,
    animate,
    fargs=(ys,),
    interval=1000,
    blit=True)
plt.show()
