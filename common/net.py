#i/usr/bin/env python
import os,sys,time

#get network RX,TX
def rxtxFunction():
    fileName='/proc/net/dev'
    try:
        fd=open(fileName)
    except IOError, e:
        print e
        exit(1)
    content=((fd.read()).replace(':',': ')).split('\n')
    rxTotal=0
    txTotal=0
    for item in content:
        if 'eth' in item:
            array=str(item).split()
            rxTotal=rxTotal+int(array[1])
            txTotal=txTotal+int(array[9])
    return rxTotal,txTotal


## main function
def main():
    rxTotal0=0
    txTotal0=0
    sleepTime=1
    if len(sys.argv)<2:
        sleepTime=1
    elif (sys.argv[1]).isdigit()==False:
        print 'argv[1]:please give a number,unit: second'
        exit(1)
    else:
        sleepTime=int(sys.argv[1])
        if sleepTime<=0:
            print 'argv[1]: must larger than 0'
            exit(1)
    while True:
        rxTotal0,txTotal0=rxtxFunction()
        time.sleep(sleepTime)
        rxTotal1,txTotal1=rxtxFunction()
        print time.strftime('%H:%M:%S',time.localtime(time.time()))+ \
           '  arg(RX)=' + str("%.4f" % ((rxTotal1-rxTotal0)*1.0/1024/1024/sleepTime)) + ' MB/s' + \
           ' '*5 + 'arg(TX)=' + str("%.4f" % ((txTotal1-txTotal0)*1.0/1024/1024/sleepTime)) + ' MB/s' 
if __name__=='__main__':
    main()
