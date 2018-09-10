#!/usr/bin/env python

from mininet.net import *
from mininet.node import *

import multiprocessing
import time
import os
import signal

def iperfServerWrapper( host, port ):
    serverCommand = 'iperf3 -s -i 1 -p %s -1 >> /tmp/%s/%sServer.log &' % ( port, host.name, host.name )
    print serverCommand
    host.cmd( serverCommand )

def iperfClientWrapper( host, port, time ):
    clientCommand = 'iperf3 -c %s -i 1 -p %s -t %s >> /tmp/%s/%sClient.log &' % ( host.IP(), port, time, host.name, host.name )
    print clientCommand
    host.cmd( clientCommand )

def iperfPair( hostOne, hostTwo, testDuration, net, port ):
    jobs = []
    pid = []
    p = multiprocessing.Process( target=iperfServerWrapper, args=( hostOne, int( port ) ) )
    jobs.append( p )
    p.start()
    pid.append( p.ident )

    p = multiprocessing.Process( target=iperfServerWrapper, args=( hostTwo, int( port ) + 1 ) )
    jobs.append( p )
    p.start()
    pid.append( p.ident )

    p = multiprocessing.Process( target=iperfClientWrapper, args=( hostOne, int( port ), testDuration ) )
    jobs.append( p )
    p.start()
    pid.append( p.ident )

    p = multiprocessing.Process( target=iperfClientWrapper, args=( hostTwo, int( port ) + 1, testDuration ) )
    jobs.append( p )
    p.start()
    pid.append( p.ident )

#    while jobs[ 2 ].is_alive() or jobs[ 3 ].is_alive():
#        print 'sleeping'
#        time.sleep( 0.5 )

    for j in jobs:
#	print 'joining %s with %i' % ( pid[ j ], signal.SIGTERM )
	j.join()


def iperfTester( numberOfLinks, hostOne, hostTwo, testDuration, net ):
    jobs = []
    port = 5001
    for i in range( numberOfLinks ):
	print port
	p = multiprocessing.Process( target=iperfPair, args=( hostOne, hostTwo, testDuration, net, port ) )
        jobs.append( p )
        p.start()
	port += 2

    for j in jobs:
        j.join()
        print '%s.exitcode = %s' % (j.name, j.exitcode)
