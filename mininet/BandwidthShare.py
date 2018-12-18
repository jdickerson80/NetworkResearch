#!/usr/bin/env python

from mininet.cli import CLI
from mininet.link import *
from mininet.log import setLogLevel, info
from mininet.net import Mininet
from mininet.node import *
from mininet.topo import *
from mininet.util import *
from FatTreeTopology import FatTreeTopo
from MapReduceScheduler import MapReduceScheduler

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
	counter = 0
	bgAdaptorAddress = net.hosts[ len( net.hosts ) - 1 ].IP()

	for host in net.hosts:
		if counter == len( net.hosts ) - 1:
			host.cmd( './BGAdaptor %s 2>&1 > /dev/null &' % bandwidthGuarantee )
		else:
			command = './WCEnabler -b %s 2>&1 > /dev/null &' % bgAdaptorAddress
			host.cmd( command )
		counter += 1 

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
	os.system( "sysctl -w net.mptcp.mptcp_enabled=1 >> commands.log" )
	os.system( "sysctl -w net.ipv4.tcp_congestion_control=reno >> commands.log" )
	os.system( "sysctl -w net.ipv4.tcp_ecn=1 >> commands.log" )
	os.system( "sysctl -w net.mptcp.mptcp_scheduler=default >> commands.log" )
	os.system( "sysctl -w net.mptcp.mptcp_path_manager=ndiffports >> commands.log" )
	#   os.system( "echo 1 > /sys/module/mptcp_ndiffports/parameters/num_subflows >> commands.log" )
	os.system( "sysctl -w net.mptcp.mptcp_debug=1 >> commands.log" )
	os.system( "echo --------------------------------------- >> commands.log" )

def createTopo( k = 4, speed = 1.0 ):
#   ENABLE ECN!!
#   k = 8 for > 100 hosts
	topo = FatTreeTopo( k, speed )

#   topo.draw(filename = "test")
	return topo


def parseCommandLineArgument():
	parser = argparse.ArgumentParser( description="Bandwidth Share Test Script" )

	parser.add_argument('--pod', '-k',
						action="store",
						help="Number of Pods",
						default=4)

	parser.add_argument('--congestion', '-c',
						action="store",
						help="Congestion control algorithm",
						default=2)

	parser.add_argument('--bandwidthGuarantee', '-b',
						action="store",
						help="Bandwidth guarantee in Mb/s",
						default=1000)

	parser.add_argument('--linkSpeed', '-s',
						action="store",
						help="Link Speed in Gbs",
						default=1.0)

	parser.add_argument('--switchPortSpeed', '-p',
						action="store",
						help="Per Switch Port Speed in Mb/s",
						default=1000)

	parser.add_argument('--traceFile', '-t',
						action="store",
						help="Trace file to generate traffic",
						default=None)

	parser.add_argument('--outputDirectory', '-d',
						action="store",
						help="Directory where results will be put",
						default=None)

	parser.add_argument('--controllerIP', '-i',
						action="store",
						help="IP address of the SDN controller",
						default=None)

	args = parser.parse_args()
	args.pod = int( args.pod )
	args.congestionAlgorithm = args.congestion
	args.linkSpeed = float( args.linkSpeed )
	args.bandwidthGuarantee = int( args.bandwidthGuarantee )
	args.traceFile = args.traceFile 

	return args


if __name__ == '__main__':
	if os.getuid() != 0:
		print "You are NOT root"
		sys.exit()

	arguments = parseCommandLineArgument()

	if not arguments.controllerIP:
		print "You did not give the IP address of the SDN Controller"
		sys.exit()

	setLogLevel( 'info' )
	setupHostMachine()
	topology = createTopo( k = arguments.pod, speed = arguments.linkSpeed )
	privateDirs = [ ( '/var/log', '/tmp/%(name)s/var/log' ), ( '/var/run', '/tmp/%(name)s/var/run' ), '/var/mn' ]

	host = partial( Host, privateDirs=privateDirs )

	# net = Mininet( topo=topology, host=host, link=TCLink, controller=None, autoSetMacs=True, autoStaticArp=True )
	net = Mininet( topo=topology, host=host, controller=None )

	net.addController( 'c0', controller=RemoteController, ip=arguments.controllerIP, port=6633 )

	net.start()

	directories = [ directory[ 0 ] if isinstance( directory, tuple )
					else directory for directory in privateDirs ]

	setupSwitchQueues( net, arguments.switchPortSpeed )
	startBWShare( net, arguments.bandwidthGuarantee )

	net.pingAll()

	if arguments.traceFile and arguments.outputDirectory:

		mrs = MapReduceScheduler( net.hosts )

		try:
			mrs.runTest( arguments.outputDirectory, arguments.traceFile,  )
		except LookupError as error:
			print error

		mrs.terminate()

	CLI( net )
	net.stop()

#   Ryu command
#   ryu-manager ryu.app.rest_qos ryu.app.qos_simple_switch_13 ryu.app.rest_conf_switch
