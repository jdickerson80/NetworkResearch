#!/usr/bin/python
from mininet.cli import *
from mininet.link import *
from mininet.log import setLogLevel, info
from mininet.net import Mininet
from mininet.node import *
from mininet.topo import *
from mininet.util import custom, waitListening

from mininet.util import *
from time import sleep
from multiprocessing import Process, Pipe, current_process
from MapReduceScheduler import *
from multiprocessing.connection import *
import time, random
import sys


from ReducerManager import *
def TCCommand( interface, host, ifbInterfaceNumber ):
    commandReturns = []

    command = "tc qdisc del dev %s root" % interface
    commandReturns = host.cmd( command )
    #    print command

    # create qdisc for eth0 device as the root queue with handle 1:0 htb queueing discipline
    # where all traffic not covered in the filters goes to flow 11
    command = "tc qdisc add dev %s root handle 1: htb default 11" % interface
    commandReturns = host.cmd( command )
    #    print command

    # add class to root qdisc with id 1:1 with htb with the max rate given as the argument
    command = "tc class add dev %s parent 1: classid 1:1 htb rate 1mbit" % interface
    commandReturns = host.cmd( command )
    #    print command

    # add class to root qdisc with id 1:2 with htb with a the rate given as the argument with a ceiling
    # as high as the max rate where the queue has the highest priority.
    # This is the BwG flow, so it has a ceiling to enforce BGAdaptors rate
    command = "tc class add dev %s parent 1:1 classid 1:11 htb rate 10mbit ceil 1mbit prio 0" % interface
    commandReturns = host.cmd( command )
    #    print command

    # add qdisc to class 1:11 that is r.e.d. with ECN enabled
    command = "tc qdisc add dev %s parent 1:11 handle 2: red limit 1000000 min 30000 max 35000 avpkt 1500 burst 20 bandwidth 10mbit probability 1 ecn" % interface
    commandReturns = host.cmd( command )
    #    print command

    # add class to root qdisc with id 1:12 with htb with the ceiling given as an argument
    # with the lowest priority.This is the WC flow, so it does not have a rate limit!!!!!!!!
    command = "tc class add dev %s parent 1:1 classid 1:12 htb rate 0.1kbit ceil 1mbit prio 1" % interface
    commandReturns = host.cmd( command )
    #    print command

    # create a filter for root class that matches all tcp protocols with tos of 0, send it to flow 1:11
    # this filters all BwG tcp packets into class 1:11
    command = "tc filter add dev %s parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11" % interface
    commandReturns = host.cmd( command )
    #    print command

    # create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
    # this filters all WC tcp packets into class 1:12
    command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12" % interface
    commandReturns = host.cmd( command )
    #    print command

    #    # create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
    #    # this filters all WC tcp packets into class 1:12
    #    command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x39 0xff flowid 1:12" % interface
    #    commandReturns = host.cmd( command )

    #    # create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
    #    # this filters all WC tcp packets into class 1:12
    #    command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x3A 0xff flowid 1:12" % interface
    #    commandReturns = host.cmd( command )

    #    # create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
    #    # this filters all WC tcp packets into class 1:12
    #    command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x3B 0xff flowid 1:12" % interface
    #    commandReturns = host.cmd( command )

    #    # create a filter for root class that matches all udp protocols with tos of 0, send it to flow 1:11
    #    # this filters all BwG udp packets into class 1:11
    #    command = "tc filter add dev %s parent 1: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11" % interface
    #    commandReturns = host.cmd( command )

    return commandReturns


class MySingleSwitch( Topo ):
    def build( self, k=2, **_opts ):
        self.k = k
        switch = self.addSwitch( 's1', enable_ecn=True )

        for h in irange( 1, k ):
            host = self.addHost( 'h%s' % h, bw=100 )
            link = self.addLink( host, switch, cls=TCLink, enable_ecn=True, bw=100 )

def Test( net ):
    number = 0
    for switch in net.switches:
        cmd = "ovs-vsctl set bridge %s protocols=OpenFlow13 stp_enable=true" % switch
        switch.cmd( cmd )

        for name in switch.intfNames():
            if name == "lo":
                continue

            interface = switch.intf( intf=name )
          #  TCCommand( interface, switch, number )
            number += 1


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

if __name__ == '__main__':
    os.system( "sysctl -w net.mptcp.mptcp_enabled=1 >> commands.log" )
    os.system( "sysctl -w net.ipv4.tcp_congestion_control=reno >> commands.log" )
    os.system( "sysctl -w net.ipv4.tcp_ecn=1 >> commands.log" )
    os.system( "sysctl -w net.mptcp.mptcp_scheduler=default >> commands.log" )
    os.system( "sysctl -w net.mptcp.mptcp_path_manager=ndiffports >> commands.log" )
    #   os.system( "echo 1 > /sys/module/mptcp_ndiffports/parameters/num_subflows >> commands.log" )
    os.system( "sysctl -w net.mptcp.mptcp_debug=1 >> commands.log" )

#    os.system( "modprobe ifb numifbs=100 >> commands.log" )
#    os.system( "modprobe sch_fq_codel >> commands.log" )
#    os.system( "modprobe act_mirred >> commands.log" )
    os.system( "echo --------------------------------------- >> commands.log" )

    setLogLevel( 'info' )
    mytopo = MySingleSwitch( 101 )
    privateDirs = [ ( '/var/log', '/tmp/%(name)s/var/log' ),
    ( '/var/run', '/tmp/%(name)s/var/run' ),
    '/var/mn' ]

    host = partial( Host,
    privateDirs=privateDirs )
#    net = Mininet( topo=mytopo, host=host, controller=None )
    net = Mininet( topo=mytopo, host=host )
#    net.addController(
#    'c0', controller=Controller,
#    ip="127.0.0.1", port=6633)

    net.start()
    # startBWShare( net, 100000 )
    directories = [ directory[ 0 ] if isinstance( directory, tuple )
    else directory for directory in privateDirs ]

    # par, child = Pipe()
    # manager = ReducerManager( net.hosts )
    # manager.start()
    
    # CLI( net )
    # # host = JobHost( 0, 1 )
    # manager.killAndRestartReducer( 0, 0 )
    # time.sleep( 3 )
    # manager.killAndRestartReducer( 1, 1 )
    # time.sleep( 3 )
    # manager.killAndRestartReducer( 0, 0 )
    # time.sleep( 3 )
    # manager.killAndRestartReducer( 1, 1 )
    # time.sleep( 3 )
    # manager.killAndRestartReducer( 5, 0 )
    # time.sleep( 3 )
    # manager.killAndRestartReducer( 7, 0 )
    # time.sleep( 3 )
    # CLI( net )
    # manager.killAndRestartReducer( -1, -1 )
    # time.sleep( 3 )
    # manager.terminate()
    # manager.join()
    # time.sleep( 10 )
    # host.hostName = -1
    # host.hostIndex = -1

    # par.send( host )
    #mp.set_start_method('spawn')
    mrs = MapReduceScheduler( net.hosts )
    
    CLI( net )
    try:
        mrs.runTest( "test/", "WholeTest.tsv", 100 )
    except LookupError as error:
        print error
#    net.pingAll()
#    print ( thisChannel.recv() )

#    while True:
#	poll = thisChannel.poll( .50 )
#	if poll == True:
#	    message = thisChannel.recv()
#	    thisChannel.send( [ 3, 1, 5001 ])
#	    thisChannel.send( [ 1, 0, "10.0.0.4", 100, 5001 ])
#	    print message
  #  CLI( net )
    mrs.terminate()

    # for i in processes:
    #     i.kill()
    #     i.wait()
            

    time.sleep( 1 )
    net.stop()

"""
	    @staticmethod
	    def intToEnum( integer ):
		lookup = {
		    0 : HostMapReduce.MapReduceClassIndex.MapperOne,
		    1 : HostMapReduce.MapReduceClassIndex.MapperTwo,
		    2 : HostMapReduce.MapReduceClassIndex.ReducerOne,
		    3 : HostMapReduce.MapReduceClassIndex.ReducerTwo }

		return lookup.get( integer, "INVALID" )

    #    for r in list:
        #	try:
            #	    msg = r.recv()
            #	except EOFError:
                #	    list.remove(r)
                #	else:
                    #	    print(msg)
                                            #    os.system( "rmmod ifb >> commands.log" )
                                            #    os.system( "rmmod sch_fq_codel >> commands.log" )
                                            #    os.system( "rmmod act_mirred >> commands.log" )





                                            #    p = multiprocessing.Process( target=iperfServerWrapper, args=( hostOne, int( port ) ) )
                                            #    jobs.append( p )
                                            #    p.start()
                                            #    pid.append( p.ident )

                                            #    p = multiprocessing.Process( target=iperfServerWrapper, args=( hostTwo, int( port ) + 1 ) )
                                            #    jobs.append( p )
                                            #    p.start()
                                            #    pid.append( p.ident )

                                            #    time.sleep( 0.25 )

                                            #    p = multiprocessing.Process( target=iperfClientWrapper, args=( hostTwo, int( port ) + 1, testDuration, hostOne.name ) )
                                            #    jobs.append( p )
                                            #    p.start()
                                            #    pid.append( p.ident )

                                            #    p = multiprocessing.Process( target=iperfClientWrapper, args=( hostOne, int( port ), testDuration, hostTwo.name ) )
                                            #    jobs.append( p )
                                            #    p.start()
                                            #    pid.append( p.ident )

                                            #    counter = 0
                                            #    while jobs[ 2 ].is_alive() or jobs[ 3 ].is_alive():
                                                #	if counter > 6:
                                                    #	    print 'breaking'
                                                    #	    break
                                                    #	print 'sleeping'
                                                    #	counter += 1
                                                    #	time.sleep( 0.25 )

                                                    #    for j in jobs:
                                                        #	print 'Pair joining %s' % ( j.name )
                                                        #	j.join()


                                                        #    // Create ingress on external interface
                                                        #    stream << "tc qdisc add dev " << interface <<
                                                        #		      " handle ffff: ingress;\n";

                                                        #     if the interace is not up bad things happen
                                                        #    stream <<  "ifconfig ifb" << interface <<
                                                        #		       " up;\n";
                                                        #    stream <<  "ifconfig ifb0 up;\n";

                                                        #    // Forward all ingress traffic to the IFB device
                                                        #    stream << "tc filter add dev ifb0 parent ffff: protocol all u32 match u32 0 0 action mirred egress redirect dev ifb0;\n";

                                                        #    // Create an EGRESS filter on the IFB device
                                                        #    stream << "tc qdisc add dev ifb0 root handle 6: htb default 11;\n";

                                                        #    stream << "tc class add dev ifb0 parent 6: classid 6:1 htb rate 7100kbit prio 0 quantum 1514;\n";

                                                        #    stream << "tc class add dev ifb0 parent 6:1 classid 6:11 htb rate 7100kbit prio 0 quantum 1514;\n";


                                                        #    stream << "tc class add dev ifb0 parent 6:1 classid 6:12 htb rate 7100kbit prio 0 quantum 1514;\n";

                                                        #    // Add FQ_CODEL qdisc with ECN support (if you want ecn)
                                                        #    stream << "tc qdisc add dev ifb0 parent 6:11 fq_codel quantum 300 ecn;\n";

                                                        #    // Forward all ingress traffic to the IFB device
                                                        #    stream << "tc filter add dev ifb" << interface <<
                                                        #		      " parent ffff: protocol all u32 match u32 0 0 action mirred egress redirect dev ifb" << interface <<
                                                        #		      ";\n";

                                                        #    // Create an EGRESS filter on the IFB device
                                                        #    stream << "tc qdisc add dev ifb" << interface <<
                                                        #		      " root handle 6: htb default 11;\n";

                                                        #    stream << "tc class add dev ifb" << interface <<
                                                        #		      " parent 6: classid 6:1 htb rate 7100kbit prio 0 quantum 1514;\n";

                                                        #    stream << "tc class add dev ifb" << interface <<
                                                        #		      " parent 6:1 classid 6:11 htb rate 7100kbit prio 0 quantum 1514;\n";


                                                        #    stream << "tc class add dev ifb" << interface <<
                                                        #		      " parent 6:1 classid 6:12 htb rate 7100kbit prio 0 quantum 1514;\n";

                                                        #    // Add FQ_CODEL qdisc with ECN support (if you want ecn)
                                                        #    stream << "tc qdisc add dev ifb" << interface <<
                                                        #		      " parent 6:11 fq_codel quantum 300 ecn;\n";

                                                        #    stream << "tc qdisc add dev ifb" << interface <<
                                                        #		      " parent 6:11 handle 2: red limit 1000000 min 30000 max 35000 avpkt 1500 burst 20 bandwidth 10mbit probability 1 ecn;\n";

                                                        #    stream << "tc filter add dev ifb" << interface <<
                                                        #		      " parent 6: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x3 0xff flowid 6:12;\n";

                                                        #    // if the interace is not up bad things happen
                                                        #    stream <<  "ifconfig ifb" << interface <<
                                                        #		       " up;\n";

                                                        #    // Forward all ingress traffic to the IFB device
                                                        #    stream << "tc filter add dev ifb" << interface <<
                                                        #		      " parent ffff: protocol all u32 match u32 0 0 action mirred egress redirect dev ifb" << interface <<
                                                        #		      ";\n";

                                                        #    // Create an EGRESS filter on the IFB device
                                                        #    stream << "tc qdisc add dev ifb0 root handle 6: htb default 11;\n";

                                                        #    stream << "tc class add dev ifb0 parent 6: classid 6:1 htb rate 7100kbit prio 0 quantum 1514;\n";

                                                        #    stream << "tc class add dev ifb0 parent 6:1 classid 6:11 htb rate 7100kbit prio 0 quantum 1514;\n";


                                                        #    stream << "tc class add dev ifb0 parent 6:1 classid 6:12 htb rate 7100kbit prio 0 quantum 1514;\n";

                                                        #    // Add FQ_CODEL qdisc with ECN support (if you want ecn)
                                                        #    stream << "tc qdisc add dev ifb0 parent 6:11 fq_codel quantum 300 ecn;\n";

                                                        #    stream << "tc qdisc add dev ifb0 parent 6:11 handle 2: red limit 1000000 min 30000 max 35000 avpkt 1500 burst 20 bandwidth 10mbit probability 1 ecn;\n";

                                                        #    stream << "tc filter add dev ifb0 parent 6: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x3 0xff flowid 6:12;\n";





                                                        #    commandReturns = []
                                                        #    command = "tc qdisc del dev %s root" % interface
                                                        #    commandReturns = host.cmd( command )

                                                        #    # create qdisc for eth0 device as the root queue with handle 1:0 htb queueing discipline
                                                        #    # where all traffic not covered in the filters goes to flow 11
                                                        #    command = "tc qdisc add dev %s root handle 1: htb default 11" % interface
							#    commandReturns = host.cmd( command )

                                                        #    # add class to root qdisc with id 1:1 with htb with the max rate given as the argument
                                                        #    command = "tc class add dev %s parent 1: classid 1:1 htb rate 1mbit" % interface
                                                        #    commandReturns = host.cmd( command )

                                                        #    # add class to root qdisc with id 1:2 with htb with a the rate given as the argument with a ceiling
                                                        #    # as high as the max rate where the queue has the highest priority.
                                                        #    # This is the BwG flow, so it has a ceiling to enforce BGAdaptors rate
                                                        #    command = "tc class add dev %s parent 1:1 classid 1:11 htb rate 10mbit ceil 1mbit prio 0" % interface
                                                        #    commandReturns = host.cmd( command )

                                                        #    # add class to root qdisc with id 1:12 with htb with the ceiling given as an argument
                                                        #    # with the lowest priority.This is the WC flow, so it does not have a rate limit!!!!!!!!
                                                        #    command = "tc class add dev %s parent 1:1 classid 1:12 htb rate 0.1kbit ceil 1mbit prio 1" % interface
                                                        #    commandReturns = host.cmd( command )

                                                        #    # create a filter for root class that matches all tcp protocols with tos of 0, send it to flow 1:11
                                                        #    # this filters all BwG tcp packets into class 1:11
                                                        #    command = "tc filter add dev %s parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11" % interface
                                                        #    commandReturns = host.cmd( command )

                                                        #    # create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
                                                        #    # this filters all WC tcp packets into class 1:12
                                                        #    command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12" % interface
                                                        #    commandReturns = host.cmd( command )

                                                        #    # create a filter for root class that matches all udp protocols with tos of 0, send it to flow 1:11
                                                        #    # this filters all BwG udp packets into class 1:11
                                                        #    command = "tc filter add dev %s parent 1: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11" % interface
                                                        #    commandReturns = host.cmd( command )

                                                        #    return commandReturns















                                                        #command = "tc qdisc del dev %s root" % interface
                                                        #commandReturns = host.cmd( command )

                                                        ## create qdisc for eth0 device as the root queue with handle 1:0 htb queueing discipline
                                                        ## where all traffic not covered in the filters goes to flow 11
                                                        #command = "tc qdisc add dev %s root handle 1: htb default 11" % interface
                                                        #commandReturns = host.cmd( command )

                                                        ## add class to root qdisc with id 1:1 with htb with the max rate given as the argument
                                                        #command = "tc class add dev %s parent 1: classid 1:1 htb rate 1mbit" % interface
                                                        #commandReturns = host.cmd( command )

                                                        ## add class to root qdisc with id 1:1 with htb with the max rate given as the argument
                                                        #command = "tc class add dev %s parent 1: classid 1:1 htb rate 1mbit" % interface
                                                        #commandReturns = host.cmd( command )

                                                        ## add class to root qdisc with id 1:2 with htb with a the rate given as the argument with a ceiling
                                                        ## as high as the max rate where the queue has the highest priority.
                                                        ## This is the BwG flow, so it has a ceiling to enforce BGAdaptors rate
                                                        #command = "tc class add dev %s parent 1:1 classid 1:11 htb rate 10mbit ceil 1mbit prio 0" % interface
                                                        #commandReturns = host.cmd( command )

                                                        ## add qdisc to class 1:11 that is r.e.d. with ECN enabled
                                                        ##    command = "tc qdisc add dev %s parent 1:11 handle 2: red limit 1000000 min 30000 max 35000 avpkt 1500 burst 20 bandwidth 10mbit probability 1 ecn" % interface
                                                        ##    commandReturns = host.cmd( command )

                                                        ## add class to root qdisc with id 1:12 with htb with the ceiling given as an argument
                                                        ## with the lowest priority.This is the WC flow, so it does not have a rate limit!!!!!!!!
                                                        #command = "tc class add dev %s parent 1:1 classid 1:12 htb rate 0.1kbit ceil 1mbit prio 1" % interface
                                                        #commandReturns = host.cmd( command )

                                                        ## create a filter for root class that matches all tcp protocols with tos of 0, send it to flow 1:11
                                                        ## this filters all BwG tcp packets into class 1:11
                                                        #command = "tc filter add dev %s parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11" % interface
                                                        #commandReturns = host.cmd( command )

                                                        ## create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
                                                        ## this filters all WC tcp packets into class 1:12
                                                        #command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12" % interface
                                                        #commandReturns = host.cmd( command )

                                                        ## create a filter for root class that matches all udp protocols with tos of 0, send it to flow 1:11
                                                        ## this filters all BwG udp packets into class 1:11
                                                        #command = "tc filter add dev %s parent 1: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11" % interface
                                                        #commandReturns = host.cmd( command )

                                                        #command = "tc qdisc add dev %s handle ffff: ingress" % interface
                                                        #commandReturns = host.cmd( command )

                                                        ##    command = "tc filter add dev %s parent ffff: protocol all u32 match u32 0 0 action mirred egress redirect parent 1:" % ( interface )
                                                        ##    commandReturns = host.cmd( command )

                                                        ##    command = "tc filter add dev %s parent ffff: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11" % interface
                                                        ##    commandReturns = host.cmd( command )

                                                        ##    command = "tc filter add dev %s parent ffff: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12" % interface
                                                        ##    commandReturns = host.cmd( command )

                                                        ##    command = "tc filter add dev %s parent ffff: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11" % interface
                                                        ##    commandReturns = host.cmd( command )

                                                        #return commandReturns



                                                        #command = "tc qdisc del dev %s root" % interface
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        #command = "tc qdisc add dev %s handle ffff: ingress" % interface
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        #command = "ifconfig ifb%i up" % number
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        #command = "tc filter add dev %s parent ffff: protocol all u32 match u32 0 0 action mirred egress redirect dev ifb%i" % ( interface, number )
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        ## Create an EGRESS filter on the IFB device
                                                        #command = "tc qdisc add dev ifb%i root handle 1: htb default 11" % number
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        ## Add root class HTB with rate limiting
                                                        #command = "tc class add dev ifb%i parent 1: classid 1:1 htb rate 1000kbit" % number
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        #command = "tc class add dev ifb%i parent 1:1 classid 1:11 htb rate 1000kbit prio 0 quantum 1514" % number
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        ## Add FQ_CODEL qdisc with ECN support (if you want ecn)
                                                        #command = "tc qdisc add dev ifb%i parent 1:11 fq_codel quantum 300 ecn" % number
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        ##########
                                                        ## EGRESS
                                                        ##########
                                                        ## create qdisc for eth0 device as the root queue with handle 1:0 htb queueing discipline
                                                        ## where all traffic not covered in the filters goes to flow 11
                                                        #command = "tc qdisc add dev %s root handle 1: htb default 11" % interface
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        ## add class to root qdisc with id 1:1 with htb with the max rate given as the argument
                                                        #command = "tc class add dev %s parent 1: classid 1:1 htb rate 1mbit" % interface
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        ## add class to root qdisc with id 1:2 with htb with a the rate given as the argument with a ceiling
                                                        ## as high as the max rate where the queue has the highest priority.
                                                        ## This is the BwG flow, so it has a ceiling to enforce BGAdaptors rate
                                                        #command = "tc class add dev %s parent 1:1 classid 1:11 htb rate 10mbit ceil 1mbit prio 0" % interface
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        ##    # add qdisc to class 1:11 that is r.e.d. with ECN enabled
                                                        ##    command = "tc qdisc add dev %s parent 1:11 handle 2: red limit 1000000 min 30000 max 35000 avpkt 1500 burst 20 bandwidth 10mbit probability 1 ecn" % interface
                                                        ##    commandReturns = host.cmd( command )

                                                        ## add class to root qdisc with id 1:12 with htb with the ceiling given as an argument
                                                        ## with the lowest priority.This is the WC flow, so it does not have a rate limit!!!!!!!!
                                                        #command = "tc class add dev %s parent 1:1 classid 1:12 htb rate 0.1kbit ceil 1mbit prio 1" % interface
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        ## create a filter for root class that matches all tcp protocols with tos of 0, send it to flow 1:11
                                                        ## this filters all BwG tcp packets into class 1:11
                                                        #command = "tc filter add dev %s parent 1: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11" % interface
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        ## create a filter for root class that matches all tcp protocols with tos of 0x38, send it to flow 1:12
                                                        ## this filters all WC tcp packets into class 1:12
                                                        #command = "tc filter add dev %s parent 1: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12" % interface
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        ## create a filter for root class that matches all udp protocols with tos of 0, send it to flow 1:11
                                                        ## this filters all BwG udp packets into class 1:11
                                                        #command = "tc filter add dev %s parent 1: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11" % interface
                                                        #print command
                                                        #commandReturns = host.cmd( command )

                                                        ##    command = "tc qdisc add dev %s handle ffff: ingress" % interface
                                                        ##    commandReturns = host.cmd( command )

                                                        ##    command = "tc filter add dev %s parent ffff: protocol all u32 match u32 0 0 action mirred egress redirect parent 1:" % ( interface )
                                                        ##    commandReturns = host.cmd( command )

                                                        ##    command = "tc filter add dev %s parent ffff: protocol ip prio 0 u32 match ip protocol 0x06 0xff match ip tos 0x00 0xff flowid 1:11" % interface
                                                        ##    commandReturns = host.cmd( command )

                                                        ##    command = "tc filter add dev %s parent ffff: protocol ip prio 1 u32 match ip protocol 0x06 0xff match ip tos 0x38 0xff flowid 1:12" % interface
                                                        ##    commandReturns = host.cmd( command )

                                                        ##    command = "tc filter add dev %s parent ffff: protocol ip prio 0 u32 match ip protocol 0x11 0xff match ip tos 0x00 0xff flowid 1:11" % interface
                                                        ##    commandReturns = host.cmd( command )
"""
