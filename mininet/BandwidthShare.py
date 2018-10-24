#!/usr/bin/env python

from mininet.cli import CLI
from mininet.link import *
from mininet.log import setLogLevel, info
from mininet.net import Mininet
from mininet.node import *
from mininet.topo import *
from mininet.util import *
from FatTreeTopology import FatTree
from Tests import *

import argparse
import logging
import os
import sys
import time

logging.basicConfig( filename='./fattree.log', level=logging.DEBUG )
logger = logging.getLogger(__name__)


def setupTCCommand( interface, host, switchPortSpeed ):
    """
    Setup the priority queues on each switchs' port
    """
    commandReturns = []

    # delete root qdisc
    command = "tc qdisc del dev %s root" % interface
    commandReturns = host.cmd( command )

    # create qdisc for eth0 device as the root queue with handle 1:0 htb queueing discipline
    # where all traffic not covered in the filters goes to flow 11
    command = "tc qdisc add dev %s root handle 1: htb default 11" % interface
    commandReturns = host.cmd( command )

    # add class to root qdisc with id 1:1 with htb with the max rate given as the argument
    command = "tc class add dev %s parent 1: classid 1:1 htb rate %smbit" % ( interface, switchPortSpeed )
    commandReturns = host.cmd( command )

    # add class to root qdisc with id 1:2 with htb with a the rate given as the argument with a ceiling
    # as high as the max rate where the queue has the highest priority.
    # This is the BwG flow, so it has a ceiling to enforce BGAdaptors rate
    command = "tc class add dev %s parent 1:1 classid 1:11 htb rate %smbit ceil 1mbit prio 0" % ( interface, switchPortSpeed )
    commandReturns = host.cmd( command )

    # add qdisc to class 1:11 that is r.e.d. with ECN enabled
    command = "tc qdisc add dev %s parent 1:11 handle 2: red limit 1000000 min 30000 max 35000 avpkt 1500 burst 20 bandwidth %smbit probability 1 ecn" % ( interface, switchPortSpeed )
    commandReturns = host.cmd( command )

    # add class to root qdisc with id 1:12 with htb with the ceiling given as an argument
    # with the lowest priority.This is the WC flow, so it does not have a rate limit!!!!!!!!
    command = "tc class add dev %s parent 1:1 classid 1:12 htb rate 0.1kbit ceil %smbit prio 1" % ( interface, switchPortSpeed )
    commandReturns = host.cmd( command )

    # create a filter for root class that matches all tcp protocols with tos of 0, send it to flow 1:11
    # this filters all BwG tcp packets into class 1:11
    command = "tc filter add dev %s parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11" % interface
    commandReturns = host.cmd( command )

    # create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
    # this filters all WC tcp packets into class 1:12
    command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12" % interface
    commandReturns = host.cmd( command )

    # create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
    # this filters all WC tcp packets into class 1:12
    command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x39 0xff flowid 1:12" % interface
    commandReturns = host.cmd( command )

    # create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
    # this filters all WC tcp packets into class 1:12
    command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x3A 0xff flowid 1:12" % interface
    commandReturns = host.cmd( command )

    # create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
    # this filters all WC tcp packets into class 1:12
    command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x3B 0xff flowid 1:12" % interface
    commandReturns = host.cmd( command )

    return commandReturns

def startBWShare( net, bandwidthGuarantee ):
    print "Starting BG and WC"
    for host in net.hosts:
	if host.name == 'h001' or host.name == 'h1':
	    host.cmd( './BGAdaptor %s 2>&1 > /dev/null &' % bandwidthGuarantee )
	else:
	    host.cmd( './WCEnabler 2>&1 > /dev/null &' )

def setupSwitchQueues( net, switchPortSpeed ):
    print "Setting up switch queues"
    for switch in net.switches:
	cmd = "ovs-vsctl set bridge %s protocols=OpenFlow13 stp_enable=true" % switch
	switch.cmd( cmd )

	for name in switch.intfNames():
	    if name == "lo":
		continue

	    interface = switch.intf( intf=name )
	    setupTCCommand( interface, switch, switchPortSpeed )

def setupHostMachine():
    os.system( "sysctl -w net.mptcp.mptcp_enabled=0 >> commands.log" )
    os.system( "sysctl -w net.ipv4.tcp_congestion_control=reno >> commands.log" )
    os.system( "sysctl -w net.ipv4.tcp_ecn=1 >> commands.log" )
    os.system( "sysctl -w net.mptcp.mptcp_scheduler=default >> commands.log" )
    os.system( "sysctl -w net.mptcp.mptcp_path_manager=ndiffports >> commands.log" )
#   os.system( "echo 1 > /sys/module/mptcp_ndiffports/parameters/num_subflows >> commands.log" )
    os.system( "sysctl -w net.mptcp.mptcp_debug=1 >> commands.log" )
    os.system( "echo --------------------------------------- >> commands.log" )

def createTopo( pod, density, bw_c2a=0.2, bw_a2e=0.1, bw_h2a=0.05 ):
    topo = FatTree( pod, density, logger )
    topo.createTopo()
    topo.createLink( bw_c2a=bw_c2a, bw_a2e=bw_a2e, bw_h2a=bw_h2a )
    return topo


def parseCommandLineArgument():
    parser = argparse.ArgumentParser( description="Bandwidth Share Test Script" )

    parser.add_argument('--pod', '-P',
			action="store",
			help="Number of Pods",
			default=2)

    parser.add_argument('--density', '-D',
			action="store",
			help="Density of pods",
			default=2)

    parser.add_argument('--congestion', '-C',
			action="store",
			help="Congestion control algorithm",
			default=2)

    parser.add_argument('--bandwidthGuarantee', '-B',
			action="store",
			help="Congestion control algorithm",
			default=1000)

    parser.add_argument('--ae',
			action="store",
			help="Bandwidth Aggregate to Edge",
			default=0.1)

    parser.add_argument('--ca',
			action="store",
			help="Bandwidth Core to Aggregate",
			default=0.2)

    parser.add_argument('--ha',
			action="store",
			help="Bandwidth Host to Aggregate",
			default=0.05)

    parser.add_argument('--switchPortSpeed',
			action="store",
			help="Per Switch Port Speed",
			default=1)

    args = parser.parse_args()
    args.ae = float( args.ae )
    args.ca = float( args.ca )
    args.ha = float( args.ha )
    args.density = int( args.density )
    args.pod = int( args.pod )
    args.bandwidthGuarantee = int( args.bandwidthGuarantee )

    return args


if __name__ == '__main__':
    if os.getuid() != 0:
	print "You are NOT root"
	sys.exit()

    arguments = parseCommandLineArgument()
    setLogLevel( 'info' )
    setupHostMachine()
    topology = createTopo( pod=arguments.pod, density=arguments.density, bw_c2a=arguments.ca, bw_a2e=arguments.ae, bw_h2a=arguments.ha )
    privateDirs = [ ( '/var/log', '/tmp/%(name)s/var/log' ),
		    ( '/var/run', '/tmp/%(name)s/var/run' ),
			'/var/mn' ]

    host = partial( Host,
		    privateDirs=privateDirs )

    net = Mininet( topo=topology, host=host, link=TCLink, controller=None, autoSetMacs=True,
		  autoStaticArp=True )

    net.addController(
	'c0', controller=RemoteController,
	ip="127.0.0.1", port=6633 )

    net.start()

    directories = [ directory[ 0 ] if isinstance( directory, tuple )
		    else directory for directory in privateDirs ]

    setupSwitchQueues( net, arguments.switchPortSpeed )
    startBWShare( net, arguments.bandwidthGuarantee )

    net.pingAll()

    number = 0
#    while number < 2:
#	print 'number = %s' % number
#    WCLogic( loggingDirectory="~/Desktop", testDuration=10, network=net ).runTest()
    number += 1

#    net.pingAll()
#    net.iperf()

    CLI( net )
    net.stop()
