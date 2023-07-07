

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
#include "ns3/yans-wifi-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/dsr-helper.h"
#include "ns3/dsr-module.h"
#include "ns3/netanim-module.h" //generate animation.xml file
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ptr.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Proyecto Simulacion");


int
main (int argc, char *argv[])
{
    bool verbose = false;
    uint32_t nRelay = 20;
    bool tracing = true;
    uint32_t packetSize = 1316;
    uint32_t seed = 2;



    CommandLine cmd (__FILE__);
    cmd.AddValue ("seed", "Seed int", seed);
    cmd.AddValue ("nRelay", "Number of wifi STA devices", nRelay); //para ingresar los argumentos
    cmd.Parse (argc,argv);

    ns3::SeedManager::SetSeed (seed);

    std::string xmlFile ="scratch/manet-routing_" + std::to_string(nRelay) + "_" + std::to_string(seed) + ".xml";

    if (verbose)
      {
        LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
      }

    //Creación de los nodos
    NodeContainer wifiApNode;
    wifiApNode.Create (1); //Crear sumidero
    NodeContainer wifiStaRelay;
    wifiStaRelay.Create (nRelay);//Crear nodos relay
    NodeContainer wifiStaNodesCamaras;
    wifiStaNodesCamaras.Create (4);// Crear nodos camaras

    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211n_2_4GHZ);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("DsssRate11Mbps"), "ControlMode",
                                  StringValue ("DsssRate11Mbps"));

    //Creación del canal
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy;
    phy.SetChannel (channel.Create ());





    //Creación de la capa MAC y de los dispositivos WiFi
    WifiMacHelper wifiMac;
    Ssid ssid = Ssid ("ProyectoSimulacion");




    wifiMac.SetType ("ns3::AdhocWifiMac");

    NetDeviceContainer sumideroDevices;
    sumideroDevices=wifi.Install (phy, wifiMac, wifiApNode);
    NetDeviceContainer camDevices;
    camDevices=wifi.Install (phy, wifiMac, wifiStaNodesCamaras);
    NetDeviceContainer RelayDevices;
    RelayDevices=wifi.Install (phy, wifiMac, wifiStaRelay);



    //Movilidad y posiciones
    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                   "rho", DoubleValue (0.0),
                                   "X", DoubleValue (125.0),
                                   "Y", DoubleValue (0.0),
                                   "Z", DoubleValue (0.0));
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiApNode); // posicion y movimiento sumidero

    //Movilidad y posicion de los relay
    mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                   "rho", DoubleValue (125.0),
                                   "X", DoubleValue (50.0),
                                   "Y", DoubleValue (50.0),
                                   "Z", DoubleValue (0.0));

    mobility.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
    "Pause", StringValue( "ns3::ConstantRandomVariable[Constant=0.0]" ),
    "Speed", StringValue( "ns3::UniformRandomVariable[Min=0.5|Max=2]") ,
    "PositionAllocator" , StringValue( "ns3::UniformDiscPositionAllocator[rho=125.0|X=50.0|Y=50.0|Z=0.0]"));


    mobility.Install (wifiStaRelay); //posicion y movimiento relay

    //Movilidad y posicion de las camaras
    mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                   "rho", DoubleValue (125.0),
                                   "X", DoubleValue (50.0),
                                   "Y", DoubleValue (50.0),
                                   "Z", DoubleValue (0.0));
    mobility.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
    "Pause", StringValue( "ns3::ConstantRandomVariable[Constant=0.0]" ),
    "Speed", StringValue( "ns3::UniformRandomVariable[Min=0.5|Max=2]") ,
    "PositionAllocator" , StringValue( "ns3::UniformDiscPositionAllocator[rho=125.0|X=50.0|Y=50.0|Z=0.0]"));
    mobility.Install (wifiStaNodesCamaras); // posicion y movimiento camaras

    //Pila TCP/IP
    AodvHelper aodv;
    Ipv4StaticRoutingHelper staticRouting;
    Ipv4ListRoutingHelper list;
    list.Add (staticRouting, 0);
    list.Add (aodv, 10);
    OlsrHelper olsr;

    InternetStackHelper stack;

    stack.SetRoutingHelper (aodv);
    //stack.SetRoutingHelper (olsr);

    stack.Install (wifiApNode);
    //dsrMain.Install (dsr, wifiApNode);
    stack.Install (wifiStaNodesCamaras);
    //dsrMain.Install (dsr, wifiStaNodesCamaras);
    stack.Install (wifiStaRelay);
    //dsrMain.Install (dsr, wifiStaRelay);


    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i;
    Ipv4InterfaceContainer camif;
    i=address.Assign (sumideroDevices);
    camif=address.Assign (camDevices);
    address.Assign (RelayDevices);


    //Aplicacion



      uint16_t port = 9;

      Address sinkLocalAddress (InetSocketAddress (i.GetAddress (0), port));
      PacketSinkHelper sinkServer ("ns3::UdpSocketFactory",sinkLocalAddress);
      ApplicationContainer sinkApp = sinkServer.Install (wifiApNode);
      sinkApp.Start (Seconds (1.0));

      AddressValue serverAddress (InetSocketAddress (i.GetAddress (0),port));
      DataRate dataRate ("200kb/s");
      OnOffHelper onOff ("ns3::UdpSocketFactory", Address (InetSocketAddress (i.GetAddress (0), port)));
      onOff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      onOff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      onOff.SetAttribute ("PacketSize", UintegerValue (packetSize));
      onOff.SetAttribute ("DataRate", DataRateValue (dataRate));

      ApplicationContainer staApps = onOff.Install (wifiStaNodesCamaras);
      staApps.Start (Seconds (100.0));
      staApps.Stop (Seconds (200.0));

      FlowMonitorHelper flowmon;
      Ptr<FlowMonitor> monitor;
      monitor = flowmon.InstallAll();



    Simulator::Stop (Seconds (200.0));
    if (tracing)
      {
        phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
        phy.EnablePcap ("Sumidero", sumideroDevices);
        phy.EnablePcap ("Camara", camDevices);

      }


    Simulator::Run ();
    monitor->SerializeToXmlFile(xmlFile, true, true);

    Simulator::Destroy ();

    return 0;

}
