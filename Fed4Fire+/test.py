#!/usr/bin/env python3
from topology import Topology,Node
from clusterer import Clusterer
from spec import Spec

# js = {"data":{"Leaders":{"datetime":"Tue, 09 Mar 2021 21:55:32 GMT","sender":{"id":"bf16cdb3-0deb-4515-9438-3a3136e3d2b2","ip":"::1","port":"5555"},"session":49,"update":{"changes":5,"cost":1.7086141109466553,"id":225517865,"selected":[{"id":"2e9ad625-d926-4200-9aa8-7cbb933d807d","ip":"node6","port":"5555"},{"id":"4ff6accb-290d-4e3a-9363-d6d297f8cc11","ip":"node8","port":"5555"},{"id":"bf16cdb3-0deb-4515-9438-3a3136e3d2b2","ip":"::1","port":"5555"},{"id":"d9f31d7c-833d-41e0-a166-97f60bcbc1a2","ip":"node15","port":"5555"},{"id":"043a950f-a79e-4606-9a40-3be588c117e3","ip":"node20","port":"5555"},{"id":"fc9dd101-e453-43f3-bada-eeb0abd1ef1b","ip":"node5","port":"5555"}]}},"Reports":[],"d3":{"directed":True,"graph":{},"links":[{"l":0,"source":0,"target":2},{"l":0,"source":1,"target":2},{"l":0,"source":2,"target":2},{"l":0,"source":3,"target":3},{"l":0,"source":4,"target":4},{"l":0,"source":5,"target":2},{"l":0,"source":6,"target":4},{"l":0,"source":7,"target":2},{"l":0,"source":8,"target":2},{"l":0,"source":9,"target":2},{"l":0,"source":10,"target":4},{"l":0,"source":11,"target":11},{"l":0,"source":12,"target":11},{"l":0,"source":13,"target":4},{"l":0,"source":14,"target":2},{"l":0,"source":15,"target":4},{"l":0,"source":16,"target":2},{"l":0,"source":17,"target":2},{"l":0,"source":18,"target":2},{"l":0,"source":19,"target":2},{"l":0,"source":20,"target":2},{"l":0,"source":21,"target":2},{"l":0,"source":22,"target":2},{"l":0,"source":23,"target":2},{"l":0,"source":24,"target":2},{"l":0,"source":25,"target":28},{"l":0,"source":26,"target":28},{"l":0,"source":27,"target":28},{"l":0,"source":28,"target":28},{"l":0,"source":29,"target":28},{"l":0,"source":30,"target":28},{"l":0,"source":31,"target":28},{"l":0,"source":32,"target":2},{"l":0,"source":33,"target":3},{"l":0,"source":34,"target":3},{"l":0,"source":35,"target":3},{"l":0,"source":36,"target":3},{"l":0,"source":37,"target":3}],"multigraph":False,"nodes":[{"id":0,"name":"node2"},{"id":1,"name":"node28"},{"id":2,"name":"node0"},{"id":3,"name":"node15"},{"id":4,"name":"node8"},{"id":5,"name":"node10"},{"id":6,"name":"node33"},{"id":7,"name":"node6"},{"id":8,"name":"node18"},{"id":9,"name":"node38"},{"id":10,"name":"node13"},{"id":11,"name":"node20"},{"id":12,"name":"node16"},{"id":13,"name":"node17"},{"id":14,"name":"node3"},{"id":15,"name":"node24"},{"id":16,"name":"node12"},{"id":17,"name":"node37"},{"id":18,"name":"node7"},{"id":19,"name":"node35"},{"id":20,"name":"node30"},{"id":21,"name":"node23"},{"id":22,"name":"node19"},{"id":23,"name":"node1"},{"id":24,"name":"node25"},{"id":25,"name":"node29"},{"id":26,"name":"node22"},{"id":27,"name":"node26"},{"id":28,"name":"node5"},{"id":29,"name":"node31"},{"id":30,"name":"node34"},{"id":31,"name":"node11"},{"id":32,"name":"node9"},{"id":33,"name":"node36"},{"id":34,"name":"node32"},{"id":35,"name":"node14"},{"id":36,"name":"node27"},{"id":37,"name":"node4"}]}},"status":True}

# nodes = {f"node{i}":0 for i in range(40)}

# for node in js["data"]["d3"]["nodes"]:
#     nodes[node["name"]] = 1

# print([node for node,v in nodes.items() if v==0])

# exit()

num = 30
seed = 38

topology = Topology.load(f"topology-{num}-{seed}")
spec = Spec(topology=topology)
spec = spec.spec

topology.save(f"topology-{num}-{seed}")
import json
spec = Spec(topology=topology)

# save the xml spec
with open("spec.xml","w") as wr:
    wr.write(spec.print_spec())

# save the spec.json
with open("spec.json","w") as wr:
    json.dump(spec.spec, wr)

exit()

S = 0
num = 0
for k,link in spec["links"].items():
    S += link["capacity"]
    num+=1
S/=num
print(S)

T = [(274,275),(249,250),(260,260),(249,250)]

T_avg = []
for (t,r) in T:
    T_avg.append((t/S,t/S))
print(T_avg)


selected = topology.selected
M = topology.matrix(selected)
clusterer = Clusterer([selected[0]],selected,M[0])
                
data = clusterer.cluster(10000)
print(data)
topology.plot(data["new_leaders"], data["clusters"])