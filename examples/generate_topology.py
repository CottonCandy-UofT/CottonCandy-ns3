import networkx as nx
import math
import numpy as np
import matplotlib.pyplot as plt
from argparse import ArgumentParser

# Accept two position tuples (x,y)
def compute_cartesian_distance(a, b):
  return math.sqrt((a[0] - b[0])**2 + (a[1] - b[1])**2)

parser = ArgumentParser()
parser.add_argument('-p', '--path', default='topology.txt',required=False)

args = parser.parse_args()
file_path = args.path


G=nx.Graph()

nodes_location = {}
edge_list = []
color_list = []

total_num_req = 0

num_req_received = {}
num_reply_delivered = {}

gateway_node = 0

with open(file_path) as fp:
    for line in fp:
        data = line.strip().split(' ')
        
        if len(data) < 4:
            continue

        node = int(data[0],16)
        pos = (float(data[1]),float(data[2]))
        edge = (node, int(data[3],16))

        nodes_location[node] = pos

        num_req = int(data[4])
        num_reply = int(data[5])

        if not node & 0x8000:
          edge_list.append(edge)
          color_list.append('green')

          num_req_received[node] = num_req
          num_reply_delivered[node] = num_reply
        else:
          color_list.append('red')
          gateway_node = node
          total_num_req = int(data[4])

num_nodes = len(num_req_received)
G.add_nodes_from(nodes_location.keys())
G.add_edges_from(edge_list)

fig = plt.figure(figsize=(8,6))
ax = fig.add_subplot(1, 1, 1)

nx.draw_networkx(G, nodes_location, node_color=color_list,with_labels=False)
#ax.grid('on')

#plt.savefig("my_ns3_topology.png")

#print(G.degree())



reference_line = [total_num_req] * num_nodes


fig1, ax1 = plt.subplots(1,1, sharex=True)
ax1.bar(num_req_received.keys(), num_req_received.values())
ax1.bar(num_reply_delivered.keys(), num_reply_delivered.values())
ax1.legend(["Number of GatewayReq received", "Number of Replies delivered (at the gateway)"])
ax1.set_xlabel("Node Address (in decimal)")
ax1.set_title("Data Gathering Performance")
ax1.set_ylim([0,total_num_req*1.2])
ax1.set_xlim([0,num_nodes])


#Plot the reference line
ax1.plot(num_req_received.keys(),reference_line, linestyle='--')

#Get the hops info -> this gives a dictionary with hops from low to high
hops_info = nx.single_source_shortest_path_length(G, gateway_node)

mean_reply_dr_by_hop = {}
std_reply_dr_by_hop = {}
mean_distance_by_hop = {}
std_distance_by_hop = {}

current_hop = 1
my_reply_list = []
my_dist_list = []



for i in hops_info:
  hop = hops_info[i]
  #print(i,hop)
  # Don't worry about the gateway
  if hop == 0:
    continue

  if hop != current_hop:
    mean_reply_dr_by_hop[current_hop] = np.mean(my_reply_list)
    mean_distance_by_hop[current_hop] = np.mean(my_dist_list)

    std_reply_dr_by_hop[current_hop] = np.std(my_reply_list)
    std_distance_by_hop[current_hop] = np.std(my_dist_list)

    current_hop = hop
    my_reply_list = []
    my_dist_list = []

  my_reply_list.append(num_reply_delivered[i]/total_num_req * 100)
  #print(num_reply_delivered[i]/total_num_req)

  dist = compute_cartesian_distance(nodes_location[i], nodes_location[gateway_node])
  my_dist_list.append(dist)

mean_reply_dr_by_hop[current_hop] = np.mean(my_reply_list)
mean_distance_by_hop[current_hop] = np.mean(my_dist_list)
std_reply_dr_by_hop[current_hop] = np.std(my_reply_list)
std_distance_by_hop[current_hop] = np.std(my_dist_list)


#print(mean_reply_dr_by_hop)
#print(mean_distance_by_hop)
my_x_axis = []
for hop in mean_reply_dr_by_hop.keys():
  my_x_axis.append(str(hop) + " \n(~" + str(round(mean_distance_by_hop[hop]/1000, 1)) + "Km)")
#print(my_x_axis)

fig2, ax2 = plt.subplots(1,1, sharex=True)

my_line_style={"linestyle":"-", "linewidth":2, "markeredgewidth":2, "elinewidth":2, "capsize":3}
ax2.errorbar(my_x_axis, mean_reply_dr_by_hop.values(), std_reply_dr_by_hop.values(), **my_line_style,color='darkgreen')
ax2.set_xlabel("Number of hops (average distance) away from the gateway")
ax2.set_ylabel("Average delivery rate for sensor data(%)")
ax2.set_title("Packet Delivery Rate with respect to Number of Hops")
ax2.grid(color='lightgrey', linestyle='-')
ax2.set_facecolor('w')
ax2.set_ylim([30,100])
plt.show()