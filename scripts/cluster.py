#!/usr/bin/env python3
from pyclustering.cluster.kmedoids import kmedoids
from pyclustering.cluster import cluster_visualizer
from pyclustering.utils import read_sample
from pyclustering.samples.definitions import FCPS_SAMPLES
import random
import math
import sqlite3

def avg_dist(matrix,cluster,medoid):
    m = 0
    for i in cluster:
        if i==medoid:
            continue
        m+= matrix[i][medoid]
    if len(cluster)==1:
        return 0
    return m/(len(cluster)-1)

def quality(matrix,clusters,medoids):
    v = 0
    avgs = [avg_dist(matrix,clusters[i],medoids[i]) for i in range(len(medoids))]
    for i in range(len(medoids)):
        m = 0
        for j in range(len(medoids)):
            if i==j:
                continue
            m2 = (avgs[i]+avgs[j])/matrix[medoids[i]][medoids[j]]
            if m < m2:
                m = m2
        v += m
    return v/len(medoids)

from shutil import copyfile

copyfile("leader_node.db", "leader_node_copy.db")

conn = sqlite3.connect('leader_node_copy.db')
c = conn.cursor()

Nodes = c.execute('SELECT * FROM MNodes').fetchall()
Leaders = c.execute('SELECT * FROM MMNodes').fetchall()

L = len(Leaders)
LeadersIds = []
for i in Leaders:
    LeadersIds.append(str(i[0]))

N = len(Nodes)

D = {}
for i in range(N):
    D[str(Nodes[i][0])]=i

A = [[None for _ in range(N)] for _ in range(N)]

avg = 0
n = 0
for a in c.execute('SELECT * FROM MLinks'):
    A[D[str(a[0])]][D[str(a[1])]] = a[2]
    if a[2] != None:
        avg += a[2]
        n+=1

c.close()

avg /= n
for i in range(len(A)):
    for j in range(len(A[i])):
        if A[i][j]==None:
            A[i][j] = avg
        if A[i][j]==0:
            A[i][j] = 0.5

c.close()

k = int(math.sqrt(N))

# Set random initial medoids. considering the already selected leaders
if L<k:
    sample = []
    for i in range(len(A)):
        if i not in [D[i] for i in LeadersIds]:
            sample.append(i)
    initial_medoids = [D[i] for i in LeadersIds] + random.sample(sample,k=k-L)

elif L==k:
    initial_medoids = [D[i] for i in LeadersIds]

else:
    initial_medoids = random.sample([D[i] for i in LeadersIds],k=k)

# create K-Medoids algorithm for processing distance matrix instead of points
kmedoids_instance = kmedoids(A, initial_medoids, data_type='distance_matrix')
# run cluster analysis and obtain results
kmedoids_instance.process()
medoids = kmedoids_instance.get_medoids()
clusters = kmedoids_instance.get_clusters()
q = quality(A,clusters,medoids)



new_leaders = []

for i in D:
    if D[i] in medoids:
        new_leaders.append(str(i))

changes = 0

for i in new_leaders:
    if i not in LeadersIds:
        changes+=1

data = {
    "quality": q,
    "new_leaders": new_leaders,
    "changes": changes
    }

import json

print(json.dumps(data))