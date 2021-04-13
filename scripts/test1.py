#!/usr/bin/env python3
from pyclustering.cluster.kmedoids import kmedoids
from pyclustering.cluster import cluster_visualizer
from pyclustering.utils import read_sample
from pyclustering.samples.definitions import FCPS_SAMPLES
import random
import math

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
    print(avgs)
    for i in range(len(medoids)):
        m = 0
        for j in range(len(medoids)):
            if i==j:
                continue
            m2 = (avgs[i]+avgs[j])/matrix[medoids[i]][medoids[j]]
            print("dist[",medoids[i],"][",medoids[j],"] = ",matrix[medoids[i]][medoids[j]])
            if m < m2:
                m = m2
        v += m
    return v/len(medoids)

def dist(a,b):
    return math.sqrt(pow((a[0]-b[0]),2)+pow((a[1]-b[1]),2))

def calculate_distance_matrix(sample):
    return [[dist(a,b) for a in sample] for b in sample]

print("starting db test")

import sqlite3
conn = sqlite3.connect('leader_node.db')
c = conn.cursor()

Nodes = c.execute('SELECT * FROM MNodes').fetchall()
Leaders = c.execute('SELECT * FROM MMNodes').fetchall()
print(Leaders)

L = len(Leaders)
LeadersIds = []
for i in Leaders:
    LeadersIds.append(i[0])

for i in Nodes:
    print(i)

N = len(Nodes)

print(N)

D = {}

for i in range(N):
    D[Nodes[i][0]]=i

print(D)

A = [[None for _ in range(N)] for _ in range(N)]

avg = 0
n = 0
for a in c.execute('SELECT * FROM MLinks'):
    A[D[a[0]]][D[a[1]]] = a[2]
    if a[2] != None:
        avg += a[2]
        n+=1

avg /= n
for i in range(len(A)):
    for j in range(len(A[i])):
        if A[i][j]==None:
            A[i][j] = avg
        if A[i][j]==0:
            A[i][j] = 1
        A[i][j] = A[i][j]



for i in A:
    print(i)
c.close()

k = int(math.sqrt(N))
print("k = ",k)

# Set random initial medoids.
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


print("initial medoids = ",initial_medoids)

# create K-Medoids algorithm for processing distance matrix instead of points
kmedoids_instance = kmedoids(A, initial_medoids, data_type='distance_matrix')
# run cluster analysis and obtain results
kmedoids_instance.process()
medoids = kmedoids_instance.get_medoids()
clusters = kmedoids_instance.get_clusters()
q = quality(A,clusters,medoids)

print("quality = ",q)
print("clusters = ",clusters)
print("medoids = ",medoids)

from matplotlib import pyplot as plt
from matplotlib.collections import LineCollection
from sklearn.manifold import MDS

mds = MDS(n_components=2, max_iter=3000000, eps=1e-9, dissimilarity="precomputed", n_jobs=1)
pos = mds.fit(A).embedding_
print(pos)

visualizer = cluster_visualizer()
visualizer.append_clusters(clusters, pos)
visualizer.show()