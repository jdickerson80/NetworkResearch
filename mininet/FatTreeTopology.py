#!/usr/bin/env python

from mininet.link import *
from mininet.log import *
from mininet.topo import *

import logging

class FatTree( Topo ):
    CoreSwitchList = []
    AggSwitchList = []
    EdgeSwitchList = []
    HostList = []

    def __init__( self, k, density, logger ):
	self.logger = logger
	self.logger.debug( "Class Fattree init" )
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
	self.logger.debug( "Create Core Layer" )
	self._addSwitch( NUMBER, 1, self.CoreSwitchList )

    def createAggLayerSwitch( self, NUMBER ):
	self.logger.debug( "Create Agg Layer" )
	self._addSwitch( NUMBER, 2, self.AggSwitchList )

    def createEdgeLayerSwitch( self, NUMBER ):
	self.logger.debug( "Create Edge Layer" )
	self._addSwitch( NUMBER, 3, self.EdgeSwitchList )

    def createHost( self, NUMBER ):
	self.logger.debug( "Create Host" )
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
	self.logger.debug( "Add link Core to Agg." )
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

	self.logger.debug( "Add link Agg to Edge." )
	for x in xrange( 0, self.iAggLayerSwitch, end ):
	    for i in xrange( 0, end ):
		for j in xrange( 0, end ):
		    self.addLink(
			self.AggSwitchList[ x + i ],
			self.EdgeSwitchList[ x + j ],
			cls=TCLink,
			enable_ecn=True,
			bw=bw_a2e)

	self.logger.debug( "Add link Edge to Host." )
	for x in xrange( 0, self.iEdgeLayerSwitch ):
	    for i in xrange( 0, self.density ):
		self.addLink(
		    self.EdgeSwitchList[ x ],
		    self.HostList[ self.density * x + i ],
		    cls=TCLink,
		    enable_ecn=True,
		    bw=bw_h2a)
