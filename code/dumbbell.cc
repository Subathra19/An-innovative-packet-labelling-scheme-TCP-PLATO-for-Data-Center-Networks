#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Router-Mod-Red");

uint32_t i = 0;

int main (int argc, char *argv[])
{
  
  uint32_t nSenders = 11;
  
  double totalbytes=0;
  double a=0;
  double ackrecieved=0;
  double dataRecieved=0;
    
  CommandLine cmd;
  cmd.AddValue ("nSenders", "Number of nodes to place in the star", nSenders);
  cmd.Parse (argc, argv);
  
  //PointToPointHelper
  NS_LOG_INFO ("Build Dumbbell topology.");
  PointToPointHelper pointToPoint;
  pointToPoint.SetQueue ("ns3::DropTailQueue");
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("0.025ms"));

  PointToPointHelper pointToPointBottleneckLink;
  pointToPointBottleneckLink.SetQueue ("ns3::DropTailQueue");
  pointToPointBottleneckLink.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  pointToPointBottleneckLink.SetChannelAttribute ("Delay", StringValue ("0.025ms"));


  PointToPointDumbbellHelper d(nSenders,pointToPoint,1,pointToPoint,pointToPointBottleneckLink);
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));

  //Buffer management
  Config::SetDefault ("ns3::ModRedQueueDisc::Mode", StringValue ("QUEUE_DISC_MODE_BYTES"));
  Config::SetDefault ("ns3::ModRedQueueDisc::Threshold", DoubleValue (179200));
  Config::SetDefault ("ns3::ModRedQueueDisc::QueueLimit", UintegerValue (256000));
  Config::SetDefault ("ns3::ModRedQueueDisc::Rtt", TimeValue (Seconds (0.0001)));
  Config::SetDefault ("ns3::ModRedQueueDisc::LinkBandwidth",DataRateValue (DataRate ("1Gbps")));
  Config::SetDefault ("ns3::ModRedQueueDisc::LinkDelay", TimeValue (MilliSeconds (0.025)));

  TrafficControlHelper tchModRed;
  tchModRed.SetRootQueueDisc ("ns3::ModRedQueueDisc", "LinkBandwidth", StringValue ("1Gbps"),
                           "LinkDelay", StringValue ("0.025ms"));
 
  //Set RTOmin
  Config::SetDefault ("ns3::TcpSocketBase::MinRto",  TimeValue (Seconds (0.001)));
  
  //Install Internet Stack
  NS_LOG_INFO ("Install internet stack on all nodes.");
  InternetStackHelper internet;
  d.InstallStack (internet);
  
  //Install ModRed
  tchModRed.Install(d.GetLeft()->GetDevice(0));
  tchModRed.Install(d.GetRight()->GetDevice(0));
  NS_LOG_INFO ("Assign IP Addresses.");
  d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
			 Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
 		      	 Ipv4AddressHelper ("10.3.1.0", "255.255.255.0")	);

  NS_LOG_INFO ("Create applications.");
  //
  // Create a packet sink on client to receive packets.
  // 
  uint16_t port = 50000;
  Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", localAddress);  
  ApplicationContainer clientApp = packetSinkHelper.Install (d.GetRight(0));
  clientApp.Start (Seconds (1.0));
  clientApp.Stop (Seconds (100.0));

  AddressValue remoteAddress (InetSocketAddress (d.GetRightIpv4Address (0), port));
  
  //BulkSend applications to send TCP to one on each server node.
  BulkSendHelper bulkSend ("ns3::TcpSocketFactory",Address ());
  // Set the amount of data to send in bytes.  Zero is unlimited.
  bulkSend.SetAttribute ("MaxBytes", UintegerValue (10000));
  

  ApplicationContainer senderApps;

  for (uint32_t i = 0; i < d.LeftCount (); ++i)
    {      
      bulkSend.SetAttribute ("Remote", remoteAddress);
      senderApps.Add (bulkSend.Install (d.GetLeft (i)));
    }
  senderApps.Start (Seconds (1.0));
  senderApps.Stop (Seconds (100.0));
 
  
  NS_LOG_INFO ("Enable static global routing.");
  //
  // Turn on global static routing so we can actually be routed across the star.
  //
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Enable pcap tracing.");
  //
  // Do pcap tracing on all point-to-point devices on all nodes.
  //
  pointToPoint.EnablePcapAll ("Router-Mod-Red");
  
  pointToPointBottleneckLink.EnablePcapAll ("Router-Mod-Red");

  /*Ptr<Node> n = d.GetRight ();
  Ptr<Node> n1 = d.GetRight (0);
  AnimationInterface anim ("anim.xml");
  anim.SetConstantPosition (n, 50, 0);
  anim.SetConstantPosition (n1, 60, 0);
  */
  //Flowmonitor to calculate throughput
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop(Seconds(130));
  Simulator::Run ();

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

    	  NS_LOG_UNCOND("Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
    	  NS_LOG_UNCOND("data transmitted = " << iter->second.txBytes);
          NS_LOG_UNCOND("Duration:"<<iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()); 
          NS_LOG_UNCOND("First packet recieved at:"<<iter->second.timeFirstTxPacket.GetSeconds());
          NS_LOG_UNCOND("Last packet recieved at:"<<iter->second.timeLastRxPacket.GetSeconds());
   	  totalbytes+=iter->second.rxBytes * 8.0 ;
          ackrecieved=iter->second.rxBytes * 8.0;
  	  if((iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())>0&&(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()>a))
  	  {
  		a=iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds();
          }
  }     
   
  monitor->SerializeToXmlFile("mod-red.flowmon", true, true);
  Simulator::Destroy ();
  dataRecieved= totalbytes-(ackrecieved*(d.LeftCount ()));
  std::cout<<"\nTotal data transmitted:"<< dataRecieved/(8*1e3)<<"kB";
  std::cout<<"\nTime taken:"<<a<<"sec";
  std::cout<<"\nThroughput:"<<dataRecieved/(a*1e6)<<"Mbps"<<std::endl;
 
  return 0;
}
