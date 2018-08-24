#!/usr/bin/env python

from mininet.cli import CLI
from mininet.link import *
from mininet.log import setLogLevel, info
from mininet.net import Mininet
from mininet.node import *
from mininet.topo import *
from mininet.util import *

import logging
import os
import sys
import time

logging.basicConfig( filename='./fattree.log', level=logging.DEBUG )
logger = logging.getLogger(__name__)


class Fattree( Topo ):
    logger.debug( "Class Fattree" )
    CoreSwitchList = []
    AggSwitchList = []
    EdgeSwitchList = []
    HostList = []

    def __init__( self, k, density ):
	logger.debug( "Class Fattree init" )
	self.pod = k
	self.iCoreLayerSwitch = ( k / 2 ) **2
	self.iAggLayerSwitch = k * k / 2
	self.iEdgeLayerSwitch = k * k / 2
	self.density = density
	self.iHost = self.iEdgeLayerSwitch * density

	#Init Topo
	Topo.__init__( self )

    def createTopo( self ):
	self.createCoreLayerSwitch( self.iCoreLayerSwitch )
	self.createAggLayerSwitch( self.iAggLayerSwitch )
	self.createEdgeLayerSwitch( self.iEdgeLayerSwitch )
	self.createHost( self.iHost )

    """
    Create Switch and Host
    """
    def _addSwitch( self, number, level, switch_list ):
	for x in xrange( 1, number + 1 ):
	    PREFIX = str( level ) + "00"
	    if x >= int( 10 ):
		PREFIX = str( level ) + "0"
	    switch_list.append( self.addSwitch( 's' + PREFIX + str( x ), enable_ecn=True ) )

    def createCoreLayerSwitch( self, NUMBER ):
	logger.debug( "Create Core Layer" )
	self._addSwitch( NUMBER, 1, self.CoreSwitchList )

    def createAggLayerSwitch( self, NUMBER ):
	logger.debug( "Create Agg Layer" )
	self._addSwitch( NUMBER, 2, self.AggSwitchList )

    def createEdgeLayerSwitch( self, NUMBER ):
	logger.debug( "Create Edge Layer" )
	self._addSwitch( NUMBER, 3, self.EdgeSwitchList )

    def createHost( self, NUMBER ):
	logger.debug( "Create Host" )
	for x in xrange( 1, NUMBER + 1 ):
	    PREFIX = "h00"
	    if x >= int( 10 ):
		PREFIX = "h0"
	    elif x >= int( 100 ):
		PREFIX = "h"
	    self.HostList.append( self.addHost( PREFIX + str( x ) ) )

    """
    Add Link
    """
    # bw_c2a = bandwidth core to aggregate
    # bw_a2e = bandwidth aggregate to edge
    # bw_h2a = bandwidth host to aggregate
    def createLink( self, bw_c2a=0.2, bw_a2e=0.1, bw_h2a=0.5 ):
	logger.debug( "Add link Core to Agg." )
	end = self.pod / 2
	for x in xrange( 0, self.iAggLayerSwitch, end ):
	    for i in xrange( 0, end ):
		for j in xrange( 0, end ):
		    self.addLink(
			self.CoreSwitchList[ i * end + j ],
			self.AggSwitchList[ x + i ],
			cls=TCLink,
			enable_ecn=True,
			bw=bw_c2a)

	logger.debug( "Add link Agg to Edge." )
	for x in xrange( 0, self.iAggLayerSwitch, end ):
	    for i in xrange( 0, end ):
		for j in xrange( 0, end ):
		    self.addLink(
			self.AggSwitchList[ x + i ],
			self.EdgeSwitchList[ x + j ],
			cls=TCLink,
			enable_ecn=True,
			bw=bw_a2e)

	logger.debug( "Add link Edge to Host." )
	for x in xrange( 0, self.iEdgeLayerSwitch ):
	    for i in xrange( 0, self.density ):
		self.addLink(
		    self.EdgeSwitchList[ x ],
		    self.HostList[ self.density * x + i ],
		    cls=TCLink,
		    enable_ecn=True,
		    bw=bw_h2a)

def setupTCCommand( interface, host ):
    commandReturns = []

    command = "tc qdisc del dev %s root" % interface
    commandReturns = host.cmd( command )

    # create qdisc for eth0 device as the root queue with handle 1:0 htb queueing discipline
    # where all traffic not covered in the filters goes to flow 11
    command = "tc qdisc add dev %s root handle 1: htb default 11" % interface
    commandReturns = host.cmd( command )

    # add class to root qdisc with id 1:1 with htb with the max rate given as the argument
    command = "tc class add dev %s parent 1: classid 1:1 htb rate 1mbit" % interface
    commandReturns = host.cmd( command )

    # add class to root qdisc with id 1:1 with htb with the max rate given as the argument
    command = "tc class add dev %s parent 1: classid 1:1 htb rate 1mbit" % interface
    commandReturns = host.cmd( command )

    # add class to root qdisc with id 1:2 with htb with a the rate given as the argument with a ceiling
    # as high as the max rate where the queue has the highest priority.
    # This is the BwG flow, so it has a ceiling to enforce BGAdaptors rate
    command = "tc class add dev %s parent 1:1 classid 1:11 htb rate 10mbit ceil 1mbit prio 0" % interface
    commandReturns = host.cmd( command )

    # add qdisc to class 1:11 that is r.e.d. with ECN enabled
    command = "tc qdisc add dev %s parent 1:11 handle 2: red limit 1000000 min 30000 max 35000 avpkt 1500 burst 20 bandwidth 10mbit probability 1 ecn" % interface
    commandReturns = host.cmd( command )

    # add class to root qdisc with id 1:12 with htb with the ceiling given as an argument
    # with the lowest priority.This is the WC flow, so it does not have a rate limit!!!!!!!!
    command = "tc class add dev %s parent 1:1 classid 1:12 htb rate 0.1kbit ceil 1mbit prio 1" % interface
    commandReturns = host.cmd( command )

    # create a filter for root class that matches all tcp protocols with tos of 0, send it to flow 1:11
    # this filters all BwG tcp packets into class 1:11
    command = "tc filter add dev %s parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11" % interface
    commandReturns = host.cmd( command )

    # create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
    # this filters all WC tcp packets into class 1:12
    command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12" % interface
    commandReturns = host.cmd( command )

    # create a filter for root class that matches all udp protocols with tos of 0, send it to flow 1:11
    # this filters all BwG udp packets into class 1:11
    command = "tc filter add dev %s parent 1: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11" % interface
    commandReturns = host.cmd( command )

    return commandReturns

def startBWShare( net ):
    print "Starting BG and WC"
    for host in net.hosts:
	if host.name == 'h001':
	    host.cmd( './BGAdaptor 10000 &> /dev/null &' )
	else:
	    host.cmd( './WCEnabler &> /dev/null &' )

def setupSwitchQueues( net ):
    print "Setting up switch queues"
    for switch in net.switches:
	cmd = "ovs-vsctl set bridge %s protocols=OpenFlow13 stp_enable=true" % switch
	switch.cmd( cmd )

	for name in switch.intfNames():
	    if name == "lo":
		continue

	    interface = switch.intf( intf=name )
	    setupTCCommand( interface, switch )

def createTopo(pod, density, ip="127.0.0.1", port=6633, bw_c2a=0.2, bw_a2e=0.1, bw_h2a=0.05):
    topo = Fattree( pod, density )
    topo.createTopo()
    topo.createLink( bw_c2a=bw_c2a, bw_a2e=bw_a2e, bw_h2a=bw_h2a )

    privateDirs = [ ( '/var/log', '/tmp/%(name)s/var/log' ),
		    ( '/var/run', '/tmp/%(name)s/var/run' ),
			'/var/mn' ]

    host = partial( Host,
		    privateDirs=privateDirs )

    net = Mininet( topo=topo, host=host, link=TCLink, controller=None, autoSetMacs=True,
		  autoStaticArp=True )

    net.addController(
	'c0', controller=RemoteController,
	ip="127.0.0.1", port=6633 )

    net.start()

    directories = [ directory[ 0 ] if isinstance( directory, tuple )
		    else directory for directory in privateDirs ]

    setupSwitchQueues( net )
    startBWShare( net )

    CLI( net )
    net.stop()

if __name__ == '__main__':
    if os.getuid() != 0:
	print "You are NOT root"
	sys.exit()

    setLogLevel ('info' )
    os.system( "sysctl -w net.mptcp.mptcp_enabled=1 >> commands.log" )
    os.system( "sysctl -w net.ipv4.tcp_congestion_control=reno >> commands.log" )
    os.system( "sysctl -w net.ipv4.tcp_ecn=1 >> commands.log" )
    os.system( "sysctl -w net.mptcp.mptcp_scheduler=default >> commands.log" )
    os.system( "sysctl -w net.mptcp.mptcp_path_manager=ndiffports >> commands.log" )
#   os.system( "echo 1 > /sys/module/mptcp_ndiffports/parameters/num_subflows >> commands.log" )
    os.system( "sysctl -w net.mptcp.mptcp_debug=1 >> commands.log" )
    os.system( "echo --------------------------------------- >> commands.log" )
    createTopo( 2, 4 )
