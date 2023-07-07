/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "ns3/netanim-module.h" //generate animation.xml file
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-mac.h"
#include "aodv-helper.h"
#include " aodv-routing-protocol.h "
// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

void
IntTrace (Ptr<const Packet> p)
{
  std::cout << "Packet " << p->GetUid ()<< std::endl;
}

int
main (int argc, char *argv[])
{
  bool verbose = false;
  uint32_t nWifi = 20;
  bool tracing = false;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);


  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  //Creación de los nodos
  NodeContainer wifiApNode;
  wifiApNode.Create (1);
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiStaNodesCamaras;
  wifiStaNodes.Create (4);

  //Creación del canal
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  //Creación de la capa MAC y de los dispositivos WiFi
  WifiMacHelper mac;
  Ssid ssid = Ssid ("ProyectoSimulacion");

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  //Movilidad y posiciones
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                 "rho", DoubleValue (0.0),
                                 "X", DoubleValue (0.0),
                                 "Y", DoubleValue (0.0),
                                 "Z", DoubleValue (0.0));

  mobility.Install (wifiApNode);

  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                 "rho", DoubleValue (50.0),
                                 "X", DoubleValue (0.0),
                                 "Y", DoubleValue (0.0),
                                 "Z", DoubleValue (0.0));

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiStaNodes);

// mobility.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
  //"Pause", PointerValue( "0" ),
  //"Speed", PointerValue( "0.5-2 m/s" ) ,
  //"PositionAllocator" , PointerValue( positionAlloc ) );
  //mobility.Install (wifiStaNodes);

  //Pila TCP/IP
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface = address.Assign (apDevices);
  address.Assign (staDevices);

  //Aplicación
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (wifiApNode.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (100.0));

  UdpEchoClientHelper echoClient (apInterface.GetAddress (0), 9);
  //echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes);
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (100.0));

  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Ptr<WifiNetDevice> wifiAP=wifiApNode.Get (0)->GetDevice(0)->GetObject<WifiNetDevice>();
  Ptr<WifiMac> wifiMAC= wifiAP->GetMac();
  wifiMAC->TraceConnectWithoutContext("MacRx",MakeCallback(&IntTrace));

  Simulator::Stop (Seconds (100.0));

  if (tracing)
    {
      phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
      phy.EnablePcap ("SimUlaCion", apDevices.Get (0));
    }

  AnimationInterface anim("build/scratch/ejemplo_sim.xml");
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
