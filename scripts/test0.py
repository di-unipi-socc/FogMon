#!/usr/bin/env python3
from pyclustering.cluster.kmedoids import kmedoids
from pyclustering.cluster import cluster_visualizer
from pyclustering.utils import read_sample
from pyclustering.samples.definitions import FCPS_SAMPLES
import random
import math

def dist(a,b):
    return math.sqrt(pow((a[0]-b[0]),2)+pow((a[1]-b[1]),2))

def calculate_distance_matrix(sample):
    return [[dist(a,b) for a in sample] for b in sample]

# Load list of points for cluster analysis.
sample = [[0,0],[1,1],[101,101],[100,100],[101,100],[50,50],[51,51]]
# calculate distance matrix for sample
#matrix = calculate_distance_matrix(sample)
matrix = [
        [0,2,100,100,100,5,5],
        [2,0,100,100,100,5,5],
        [100,100,0,1,1,1,1],
        [100,100,1,0,1,1,1],
        [100,100,1,1,0,1,1],
        [5,5,1,1,1,0,2],
        [5,5,1,1,1,2,0]
    ]

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
    #TODO: add linear combination of changed medoids to the cost
    v = 0
    avgs = [avg_dist(matrix,clusters[i],medoids[i]) for i in range(len(medoids))]
    print(avgs)
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

instance = None
m = 0
for i in range(4):
    
    # Set random initial medoids.
    initial_medoids = random.sample(range(len(matrix[0])),k=2)

    # create K-Medoids algorithm for processing distance matrix instead of points
    kmedoids_instance = kmedoids(matrix, initial_medoids, data_type='distance_matrix')
    # run cluster analysis and obtain results
    kmedoids_instance.process()
    medoids = kmedoids_instance.get_medoids()
    clusters = kmedoids_instance.get_clusters()
    q = quality(matrix,clusters,medoids)
    print(q)
    if m==0:
        m=q
        instance = kmedoids_instance
    else:
        if q < m:
            m = q
            instance = kmedoids_instance
print(m)
kmedoids_instance = instance
medoids = kmedoids_instance.get_medoids()
# Run cluster analysis and obtain results.
clusters = kmedoids_instance.get_clusters()

# Show allocated clusters.
print(clusters)
print(medoids)
# Display clusters.
#visualizer = cluster_visualizer()
#visualizer.append_clusters(clusters, sample)
#visualizer.show()

from matplotlib import pyplot as plt
from matplotlib.collections import LineCollection
from sklearn.manifold import MDS

mds = MDS(n_components=2, max_iter=3000000, eps=1e-9, dissimilarity="precomputed", n_jobs=1)
pos = mds.fit(matrix).embedding_
print(pos)

#fig = plt.figure(1)
#s = 100
#plt.scatter(pos[:, 0], pos[:, 1], color='turquoise', s=s, lw=0, label='MDS')
#plt.show()

visualizer = cluster_visualizer()
visualizer.append_clusters(clusters, pos)
visualizer.show()



