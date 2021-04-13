from template_fabfile import TestBeds, Ubuntu

class Spec:

   def __init__(self, topology=None):
      self.start = """<?xml version='1.0'?>
      <rspec xmlns="http://www.geni.net/resources/rspec/3" type="request" generated_by="jFed RSpec Editor" generated="2020-11-30T18:35:22.186+01:00" xmlns:emulab="http://www.protogeni.net/resources/rspec/ext/emulab/1" xmlns:delay="http://www.protogeni.net/resources/rspec/ext/delay/1" xmlns:jfed-command="http://jfed.iminds.be/rspec/ext/jfed-command/1" xmlns:client="http://www.protogeni.net/resources/rspec/ext/client/1" xmlns:jfed-ssh-keys="http://jfed.iminds.be/rspec/ext/jfed-ssh-keys/1" xmlns:jfed="http://jfed.iminds.be/rspec/ext/jfed/1" xmlns:sharedvlan="http://www.protogeni.net/resources/rspec/ext/shared-vlan/1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.geni.net/resources/rspec/3 http://www.geni.net/resources/rspec/3/request.xsd ">"""
      self.end = "</rspec>"
      self.spec = {"nodes": {}, "links": {}}
      
      if topology is not None:
         self.create_from_topology(topology)

   _id_node_ = 0
   def create_id_node(self):
      ret = self._id_node_
      self._id_node_+=1
      return ret

   _id_link_ = 0
   def create_id_link(self):
      ret = self._id_link_
      self._id_link_+=1
      return ret

   _ip_ = 0
   def create_ip(self):
      ret = self._ip_
      self._ip_+=1
      return ret

   def create_id_if(self, n1):
      nodes = self.spec["nodes"]
      max_if = -1
      for if_,_,_ in nodes[n1]["if"]:
         n = int(if_[2:])
         if n>max_if:
            max_if = n
      return max_if+1

   def setLinkLatCap(self, node1: int, node2:int, latency: int=0, capacity: int=0, packet_loss: str=None):
      """
      To be called after create_links as no links are present before.
      set the link informations:
      latency, capacity, packet_loss
      """
      links= self.spec["links"]
      for l,v in links.items():
         n1 = v["interfaces"][0].split(":")[0]
         n2 = v["interfaces"][1].split(":")[0]
         if (n1 == "node"+str(node1) and n2 == "node"+str(node2)) or (n2 == "node"+str(node1) and n1 == "node"+str(node2)):
            if latency != 0:
               v["latency"] = latency
            if capacity != 0:
               v["capacity"] = capacity
            if packet_loss is not None:
               v["packet_loss"] = packet_loss

   def create_links(self):
      """
      To be called after the creation of the nodes to instantiate the links.
      """
      nodes= self.spec["nodes"]
      links= self.spec["links"]
      for n1,v1 in nodes.items():
         monotonic = False
         for n2,v2 in nodes.items():
            if not monotonic:
               if n1 == n2:
                  monotonic=True
               continue
            same_testbed = False
            if v1["testbed"] == v2["testbed"]:
               same_testbed = True
            id = self.create_id_link()
            if_1 = "if"+str(self.create_id_if(n1))
            if_2 = "if"+str(self.create_id_if(n2))
            ip = self.create_ip()
            nodes[n1]["if"].append((if_1,ip,same_testbed))
            nodes[n2]["if"].append((if_2,ip,same_testbed))
            link_type = "lan"
            if v1["testbed"] == TestBeds.CITY:
               link_type = "gre-tunnel"
            links["link"+str(id)] = {"testbed": v1["testbed"], "interfaces":[n1+":"+if_1,n2+":"+if_2], "link_type": link_type, "same_testbed": same_testbed, "ips": ["10.%d.%d.%d"%(ip//256, ip%256,int(n1[4:])+1),"10.%d.%d.%d"%(ip//256, ip%256,int(n2[4:])+1)]}


   def create_nodes(self, num: int, testbed: TestBeds):
      """
      create num nodes in the specified testbed.
      """
      nodes= self.spec["nodes"]
      for i in range(num):
         id = self.create_id_node()
         image = Ubuntu.WALL1
         if testbed == TestBeds.WALL2:
            image = Ubuntu.WALL1
         elif testbed == TestBeds.CITY:
            image = Ubuntu.CITY
         nodes["node"+str(id)] = {"testbed": testbed, "image": image,"if": []}

   def print_spec(self, division=False):
      """
      return the spec in an xml format
      """
      if division:
         text2 = self.start
      text = self.start
      nodes= self.spec["nodes"]
      links= self.spec["links"]
      x = 0
      y = 0
      for n,v in nodes.items():
         component = 'component_id="urn:publicid:IDN+lab.cityofthings.eu+node+node13"'
         nodeT= f'<node client_id="{n}" exclusive="true" component_manager_id="{v["testbed"].value}">\n<sliver_type name="raw-pc">\n<disk_image name="{v["image"].value}"/>\n</sliver_type>\n'
         nodeT+= f'<location xmlns="http://jfed.iminds.be/rspec/ext/jfed/1" x="{x}" y="{y}"/>\n'
         x+=10
         y+=10
         nodeT+= '</node>\n'
         if division and (v["testbed"] == TestBeds.CITY or v["testbed"] == TestBeds.WALL2):
            text2 += nodeT
            continue
         text+=nodeT
      
      text+=self.end
      if division:
         text2 += self.end
         return text,text2
      return text
   
   def create_from_topology(self, topology):      
      selected = topology.selected
      M = topology.matrix(selected)

      matrix = []
      m = 0
      for i in selected:
         latencies = [M[0][i][j] for j in selected]
         uploads = [M[1][i][j] for j in selected]
         if m < 10:
            testbed = TestBeds.WALL2 # TODO: decide testbed
         else:
            testbed = TestBeds.CITY
         matrix.append((latencies,uploads,testbed))
         m+=1
      # this take the matrix and create the nodes
      for row in matrix:
         self.create_nodes(1, row[2])

      # instantiate the links
      self.create_links()

      # set the link informations
      for i in range(len(matrix)):
         for j in range(len(matrix)):
               if i == j:
                  continue
               self.setLinkLatCap(i,j,matrix[i][0][j],matrix[i][1][j])
   
   def remove_nodes(self, removes):
      # remove nodes from json, example: ["10","0"]

      for remove in removes:
         del self.spec["nodes"][remove]
         rms = []
         for l,v in self.spec["links"].items():
            n1 = v["interfaces"][0].split(":")[0]
            n2 = v["interfaces"][1].split(":")[0]
            if n1 == remove or n2 == remove:
               rms.append(l)
         for rm in rms:
            del self.spec["links"][rm]
