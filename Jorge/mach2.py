# -*- coding: utf-8 -*-
"""
Created on Tue Jul  7 08:08:39 2020

@author: beto_
"""


import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import tensorflow as tf


print(f"TensorFlow version = {tf.__version__}\n")

# Set a fixed random seed value, for reproducibility, this will allow us to get
# the same random numbers each time the notebook is run
SEED = 1337
np.random.seed(SEED)
tf.random.set_seed(SEED)

# the list of gestures that data is available for
GESTURES = [
    "punch",
    "flex",
]

SAMPLES_PER_GESTURE = 120

NUM_GESTURES = len(GESTURES)
num_recordings = int(35)

# create a one-hot encoded matrix that is used in the output
ONE_HOT_ENCODED_GESTURES = np.eye(NUM_GESTURES)

inputs = []
outputs = []

# read each csv file and push an input and output

# print(f"Processing index {0} for gesture 'punch'.")
  
  
dp = pd.read_csv("C:\\Users\\beto_\\OneDrive - Universidad Iberoamericana A.C\\TUM\\2do semestre\\Geoteck 3\\Ppp\\" + "punch" + ".csv",header=None)

  # print(df.shape)
  # calculate the number of gesture recordings in the file
# num_recordings = int(dp.shape[0])

# print(f"\tThere are {num_recordings} recordings of the {gesture} gesture.")
  
df = pd.read_csv("C:\\Users\\beto_\\OneDrive - Universidad Iberoamericana A.C\\TUM\\2do semestre\\Geoteck 3\\Ppp\\" + "flex" + ".csv",header=None)
 
dp = dp.append(df).reset_index(drop=True)

for gesture_index in range(num_recordings):
    output = ONE_HOT_ENCODED_GESTURES[0]
    outputs.append(output)
for gesture_index in range(num_recordings):
    output = ONE_HOT_ENCODED_GESTURES[1]
    outputs.append(output)
    
# # convert the list to numpy array
inputs = dp
outputs = np.array(outputs)  


# Randomize the order of the inputs, so they can be evenly distributed for training, testing, and validation
# https://stackoverflow.com/a/37710486/2020087
num_inputs = len(inputs)


# Swap the consecutive indexes (0, 1, 2, etc) with the randomized indexes


# Split the recordings (group of samples) into three sets: training, testing and validation
TRAIN_SPLIT = int(0.6 * num_inputs)
TEST_SPLIT = int(0.2 * num_inputs + TRAIN_SPLIT)

inputs_train, inputs_test, inputs_validate = np.split(inputs, [TRAIN_SPLIT, TEST_SPLIT])
outputs_train, outputs_test, outputs_validate = np.split(outputs, [TRAIN_SPLIT, TEST_SPLIT])


print("Data set randomization and splitting complete.")
inputs_train=np.array(inputs_train)
inputs_test=np.array(inputs_test)
outputs_train=np.array(outputs_train)
outputs_test=np.array(outputs_test)
inputs_validate=np.array(inputs_validate)
outputs_validate=np.array(outputs_validate)


model = tf.keras.Sequential()
model.add(tf.keras.layers.Dense(50, activation='relu')) # relu is used for performance
model.add(tf.keras.layers.Dense(15, activation='relu'))
model.add(tf.keras.layers.Dense(NUM_GESTURES, activation='softmax')) # softmax is used, because we only expect one gesture to occur per input
model.compile(optimizer='rmsprop', loss='mse', metrics=['mae'])

history = model.fit(inputs_train, outputs_train, epochs=15, batch_size=1, validation_data=(inputs_validate, outputs_validate))

  
  



# increase the size of the graphs. The default size is (6,4).
plt.rcParams["figure.figsize"] = (20,10)

# graph the loss, the model above is configure to use "mean squared error" as the loss function
loss = history.history['loss']
val_loss = history.history['val_loss']
epochs = range(1, len(loss) + 1)
plt.plot(epochs, loss, 'g.', label='Training loss')
plt.plot(epochs, val_loss, 'b', label='Validation loss')
plt.title('Training and validation loss')
plt.xlabel('Epochs')
plt.ylabel('Loss')
plt.legend()
plt.show()

print(plt.rcParams["figure.figsize"])
  
# graph the loss again skipping a bit of the start
SKIP = 4
plt.plot(epochs[SKIP:], loss[SKIP:], 'g.', label='Training loss')
plt.plot(epochs[SKIP:], val_loss[SKIP:], 'b.', label='Validation loss')
plt.title('Training and validation loss')
plt.xlabel('Epochs')
plt.ylabel('Loss')
plt.legend()
plt.show()
  
# use the model to predict the test inputs
predictions = model.predict(inputs_test)

# print the predictions and the expected ouputs
print("predictions =\n", np.round(predictions, decimals=3))
print("actual =\n", outputs_test)

# Plot the predictions along with to the test data
plt.clf()
plt.title('Training data predicted vs actual values')
plt.plot(inputs_test, outputs_test, 'b.', label='Actual')
plt.plot(inputs_test, predictions, 'r.', label='Predicted')
plt.show()
  
  
# graph of mean absolute error
mae = history.history['mae']
val_mae = history.history['val_mae']
plt.plot(epochs[SKIP:], mae[SKIP:], 'g.', label='Training MAE')
plt.plot(epochs[SKIP:], val_mae[SKIP:], 'b.', label='Validation MAE')
plt.title('Training and validation mean absolute error')
plt.xlabel('Epochs')
plt.ylabel('MAE')
plt.legend()
plt.show()  
  
  
  
  
  
  
