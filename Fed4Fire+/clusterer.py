from pyclustering.cluster.kmedoids import kmedoids
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

class Clusterer:
    def __init__(self, Ls,Ns, Links, formula=0):
        self.Nodes = Ns
        self.Leaders = Ls

        self.N = len(self.Nodes)
        self.L = len(self.Leaders)

        self.D = {}
        for i in range(self.N):
            self.D[self.Nodes[i]]=i


        self.A = [[Links[i][j] for j in self.Nodes] for i in self.Nodes]

        if formula == -1:
            k = int(math.sqrt(self.N)/1.55)
        elif formula == -2:
            k = int(math.sqrt(self.N)*2)
        elif formula > 0:
            k = formula
        else:
            k = int(math.sqrt(self.N))
        # Set random initial medoids. considering the already selected leaders
        if self.L<k:
            sample = []
            for i in range(len(self.A)):
                if i not in [self.D[i] for i in self.Leaders]:
                    sample.append(i)
            self.initial_medoids = [self.D[i] for i in self.Leaders] + random.sample(sample,k=k-self.L)
        elif self.L==k:
            self.initial_medoids = [self.D[i] for i in self.Leaders]
        else:
            self.initial_medoids = random.sample([self.D[i] for i in self.Leaders],k=k)

    def cluster(self, tries=1):
        mini = math.inf
        data = None
        for _ in range(tries):
            # create K-Medoids algorithm for processing distance matrix instead of points
            kmedoids_instance = kmedoids(self.A, self.initial_medoids, data_type='distance_matrix')
            # run cluster analysis and obtain results
            kmedoids_instance.process()
            medoids = kmedoids_instance.get_medoids()
            clusters = kmedoids_instance.get_clusters()
            q = quality(self.A,clusters,medoids)

            if q < mini:
                mini = q

                new_leaders = []

                for i in self.D:
                    if self.D[i] in medoids:
                        new_leaders.append(i)
                
                changes = 0

                for i in new_leaders:
                    if i not in self.Leaders:
                        changes+=1

                inv = {v:k for k,v in self.D.items()}

                clusts = []

                for c in clusters:
                    clust = []
                    for el in c:
                        clust.append(inv[el])
                    clusts.append(clust)

                data = {
                    "quality": q,
                    "new_leaders": new_leaders,
                    "changes": changes,
                    "clusters": clusts
                    }
        return data