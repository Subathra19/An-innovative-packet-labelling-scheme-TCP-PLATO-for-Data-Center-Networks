# An-innovative-packet-labelling-scheme-TCP-PLATO-for-Data-Center-Networks
In  data  center  networks,  many  applications  such  as  cluster-based  storage and MapReduce require many to one communication and high fan-in.  When multiple synchronized servers send data to the same client in parallel, TCPincast congestion occurs.  It leads to TCP throughput collapse, substantiallydegrading  the  performance  of  the  application. Hence, the existing loss detection mechanism of TCP NewReno has been modified by a packet labelling scheme called TCP PLATO. In PLATO, labelled packets are givenmore  priority  over  unlabelled  packets  at  the  switch.   This  allows  TCP  to detect packet loss by three duplicate acknowledgements and avoid RTO andthereby improve throughput.  TCP PLATO has been simulated in ns-3 and its performance is compared with TCP NewReno for various buffer size, SRU,and topologies. Results are plotted with the help of MATLAB.

## Introduction
Data center is a pool of resources (computational/ storage) interconnected using a communication network.

* DCNs were brought into put a central place for computational, storage and networking needs for the organization. With the advent of cloud computingand storage, DCN needs to be scalable and efficient to handle this growth. Companies need to manage and upgrade their personal DCN’s regularly. There are companies which
provide DCN services to the public as well, like Dropbox, Google drive.
Many applications such as cluster-based storage in DCN require many to one type of data communication with a high fan in.


* In cluster-based storage system, we use two or more storage servers which work together in order to increase performance and capacity. With the help of clustering, a file can be accessed from any server irrespective of its location. It distributes the workload between the servers and manages the transfer of workloads. The advantages of such storage are: low cost, with the increase in demand for digital content storage cluster based storage is the best solution.

<!-- Alignment options!!!!! -->
<img align="center" width="100" height="100" src="http://www.fillmurray.com/100/100">
![alt text](https://github.com/Subathra19/An-innovative-packet-labelling-scheme-TCP-PLATO-for-Data-Center-Networks/blob/main/images/cluster.png) 
*Data Center Network*

<figure>
  <img src="{{https://github.com/Subathra19/An-innovative-packet-labelling-scheme-TCP-PLATO-for-Data-Center-Networks/blob/main/images/cluster.png }}">
  <figcaption>{{ Data Center Network }}</figcaption>
</figure>

* Above figure shows a simple cluster-based storage system where one client requests data from multiple servers through the synchronized read. On receiving SRU(Server Request Unit) from the client, servers start sending data in parallel. Upon the successful receipt of all the SRU’s, the client can send out new requests for another round of SRU’s. Hence, the finish time of a round of transfers depends on the slowest server. When a server is involved in a synchronized request experiences a time-out (i.e., RTO), other servers
can finish their response, but the client must wait for a minimum of 200ms (RTOmin) before receiving a remaining response. In the DCN environment the typical Round Trip Time (RTT) is in the order of 100µs, therefore the RTO can result in TCP incast throughput collapse. Thus the RTO resulting from packet loss leads to severe link underutilization, and consequently the throughput collapse.

