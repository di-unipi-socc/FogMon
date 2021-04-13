#!/usr/bin/env python3
from spec import Spec
from template_fabfile import TestBeds, Ubuntu
import json

from topology import Topology
import random

topology = Topology()
topology.create_tree(6,[2,3,2,3,2,1],([(10,20),(1,5),(1,10),(1,30),(1,10)],[(70000,200000),(50000,100000),(10000,100000),(10000,100000),(10,100000)]))
    
cloud_high = random.sample(topology.return_level(2),  3) # Central cloud         3
cloud_low = random.sample(topology.return_level(3),   5) # Decentralised cloud   5
isp = random.sample(topology.return_level(4),         12) # ISP                   12
home = random.sample(topology.return_level(5),        20) # Home                 20
                                                   # =32                        =40
selected = cloud_high + cloud_low + isp + home

topology.purge(selected)
M = topology.matrix(selected)

matrix = []
for i in selected:
   latencies = [M[0][i][j] for j in selected]
   uploads = [M[1][i][j] for j in selected]
   testbed = TestBeds.WALL2 if i not in home else TestBeds.CITY
   matrix.append((latencies,uploads,testbed))
# the first and second matrix must be symmetric, the first represent the latency, the other represent the upload of every node against another
# this example create 4 nodes, 2 from WALL2 and 2 from citylab
# matrix = [
#    ([0,4,10,10],  [0,  0,1000  ,1000],  TestBeds.WALL1),
#    ([4,0,10,10],  [0,  0,0  ,      0],  TestBeds.WALL1),
#    ([10,10,0,4],  [1000,0,0,    1000],  TestBeds.CITY),
#    ([10,10,4,0],  [1000,0,1000,    0],  TestBeds.CITY),
# ]

# matrix = [ # 28+5, diagonals are ignored
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([4]*28+[10]*5,  [1000]*28+[500]*5,  TestBeds.WALL2),
#    ([10]*28+[4]*5,  [500]*28+[1000]*5,  TestBeds.CITY),
#    ([10]*28+[4]*5,  [500]*28+[1000]*5,  TestBeds.CITY),
#    ([10]*28+[4]*5,  [500]*28+[1000]*5,  TestBeds.CITY),
#    ([10]*28+[4]*5,  [500]*28+[1000]*5,  TestBeds.CITY),
#    ([10]*28+[4]*5,  [500]*28+[1000]*5,  TestBeds.CITY),
# ]

spec = Spec()

# spec.create_nodes(2, TestBeds.WALL1)
# spec.create_nodes(1, TestBeds.WALL2)
# spec.create_nodes(2, TestBeds.CITY)

# this take the matrix and create the nodes
for row in matrix:
   spec.create_nodes(1, row[2])

# instantiate the links
spec.create_links()

# set the link informations
for i in range(len(matrix)):
   for j in range(len(matrix)):
      if i == j:
         continue
      spec.setLinkLatCap(i,j,matrix[i][0][j],matrix[i][1][j])

# save the xml spec
with open("spec.xml","w") as wr:
   wr.write(spec.print_spec())

# save the spec.json
with open("spec.json","w") as wr:
   json.dump(spec.spec, wr)