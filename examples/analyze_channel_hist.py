import networkx as nx
import math
import numpy as np
import matplotlib.pyplot as plt
from argparse import ArgumentParser

num_seq_number = 100

num_channel_switch = [0] * num_seq_number

with open('channelHistory.txt') as fp:
    
    for line in fp:
        data = line.strip().split(' ')

        if len(data) < 2:
            break
        
        address = int(data[0])

        for v in data[1:]:
            index = int(v) - 1
            print(index)
            num_channel_switch[index] += 1

plt.figure()
plt.plot(range(1,101), num_channel_switch)
plt.show()

        