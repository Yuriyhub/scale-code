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

data2 = aio.receive(scale.key).value
print(data2)

#data3 = aio.receive_previous(scale.key)
#print(data3)

