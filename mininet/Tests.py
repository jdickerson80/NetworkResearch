#!/usr/bin/env python

from mininet.net import *
from mininet.node import *

import multiprocessing
import time
import os
import signal
#import subprocesses
import psutil
#try:
#   import cPickle as pickle
#except:
#   import pickle

#iperf3 -s -p 5002
#iperf3 -c iperf3.example.com -p 5002

#def kill_child_processes( parent_pid, sig=signal.SIGTERM ):
#    ps_command = subprocess.Popen("ps -o pid --ppid %d --noheaders" % parent_pid, shell=True, stdout=subprocess.PIPE)
#    ps_output = ps_command.stdout.read()
#    retcode = ps_command.wait()
#    assert retcode == 0, "ps command returned %d" % retcode
#    for pid_str in ps_output.split("\n")[:-1]:
#            os.kill(int(pid_str), sig)

def iperfServerWrapper( host, port, duration ):
    serverCommand = 'iperf -s -e -p %s > /tmp/%ss.log &' % ( port, host.name )
    host.cmd( serverCommand )
#    time.sleep( duration + 2 )
#    multiprocessing.current_process().terminate()

def iperfClientWrapper( host, port, time ):
    clientCommand = 'iperf -c %s -e -p %s -t 5 > /tmp/%sc.log' % ( host.IP(), port, host.name )
    host.cmd( clientCommand )

def iperfWrapper( hostOne, hostTwo, testDuration, net, port ):
#    hostOneServerCommand = 'iperf -e -s -p %s > /tmp/ones.log' % 5001
#    hostTwoServerCommand = 'iperf -e -s -p %s > /tmp/twos.log' % 5002

#    hostOneClientCommand = 'iperf -e -c %s -p %s > /tmp/onec.log' % ( hostOne.IP(), 5001 )
#    hostTwoClientCommand = 'iperf -e -c %s -p %s > /tmp/twoc.log' % ( hostTwo.IP(), 5002 )

#    print hostOneServerCommand
#    print hostTwoServerCommand

#    print hostOneClientCommand
#    print hostTwoClientCommand
    jobs = []
    pid = []
    p = multiprocessing.Process( target=iperfServerWrapper, args=( hostOne, 5001, testDuration ) )
    jobs.append( p )
    p.start()
    pid.append( p.ident )

    p = multiprocessing.Process( target=iperfServerWrapper, args=( hostTwo, 5002, testDuration  ) )
    jobs.append( p )
    p.start()
    pid.append( p.ident )

    p = multiprocessing.Process( target=iperfClientWrapper, args=( hostOne, 5001, testDuration ) )
    jobs.append( p )
    p.start()
    pid.append( p.ident )

    p = multiprocessing.Process( target=iperfClientWrapper, args=( hostTwo, 5002, testDuration ) )
    jobs.append( p )
    p.start()
    pid.append( p.ident )

    for j in pid:
        print j

    while jobs[ 2 ].is_alive() or jobs[ 3 ].is_alive():
        print 'sleeping'
        time.sleep( 0.5 )


#    jobs[ 2 ].join()
#    jobs[ 3 ].join()

    for j in range( 0, 2 ):
        print 'killing %s with %i' % ( pid[ j ], signal.SIGTERM )
#        kill_child_processes( pid[ j ], signal.SIGTERM )
        jobs[ j ].join()
#    for j in range( 0, 2 ):
#        print 'killing %s with %i' % ( pid[ j ], signal.SIGTERM )
#        os.kill( pid[ j ], signal.SIGTERM )
#        jobs[ j ].join()

#    for j in jobs:
#        if j.is_alive():
#            j.terminate()
#        j.join()
#        print '%s.exitcode = %s' % ( j.name, j.exitcode)


#    hostOne.cmd( hostOneServerCommand )
#    hostTwo.cmd( hostTwoServerCommand )

#    hostOne.cmd( hostOneClientCommand )
#    hostTwo.cmd( hostTwoClientCommand )


def iperfTester( numberOfLinks, hostOne, hostTwo, testDuration, net ):
    jobs = []
    port = 5001
    for i in range( numberOfLinks ):
        p = multiprocessing.Process( target=iperfWrapper, args=( hostOne, hostTwo, testDuration, net, port ) )
        jobs.append( p )
        p.start()
        port += 1

    for j in jobs:
        j.join()
        print '%s.exitcode = %s' % (j.name, j.exitcode)
