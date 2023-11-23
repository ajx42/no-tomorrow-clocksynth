import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from argparse import ArgumentParser
from collections import namedtuple

plt.rcParams["figure.figsize"] = [10.00, 10.00]
plt.rcParams["figure.autolayout"] = True

class InParams:
    def __init__(self):
        self.limits = None
        self.source = None
        self.sinks = []
        self.blockages = []
    def __repr__(self):
        return  'limits:{}'.format(self.limits) + '\n' +\
                'source:{}'.format(self.source) + '\n' +\
                'sinks:{}'.format(self.sinks) + '\n' +\
                'blockages:{}'.format(self.blockages)

class OutParams:
    def __init__(self):
        self.sourcename = None
        self.nodes = []
        self.sinks = []
        self.wires = []
        self.buffers = []
    def __repr__(self):
        return  'source:{}'.format(self.sourcename) + '\n' +\
                'nodes:{}'.format(self.nodes) + '\n' +\
                'sinks:{}'.format(self.sinks) + '\n' +\
                'wires:{}'.format(self.wires) + '\n' +\
                'buffers:{}'.format(self.buffers)
    
def parse_output(filename):
    data = []
    with open(filename, 'r') as inpfile:
        data = [x.strip() for x in inpfile]
    ret = OutParams()
    # source
    curr, *data = data
    _, name, _ = curr.split()
    ret.sourcename = name
    
    # nodes
    curr, *data = data
    tag, _, num = curr.split()
    num = int(num)
    for _ in range(num):
        curr, *data = data
        name, xx, yy = curr.split()
        xx, yy = int(xx), int(yy)
        ret.nodes.append((name, xx, yy))
    # sink nodes
    curr, *data = data
    tag, _, num = curr.split()
    num = int(num)
    for _ in range(num):
        curr, *data = data
        nodename, sinkname = curr.split()
        ret.sinks.append((nodename, sinkname))
    # wires
    curr, *data = data
    tag, _, num = curr.split()
    num = int(num)
    for _ in range(num):
        curr, *data = data
        fromnode, tonode, kind = curr.split()
        ret.wires.append((fromnode, tonode, kind))
    # buffers
    curr, *data = data
    tag, _, num = curr.split()
    num = int(num)
    for _ in range(num):
        curr, *data = data
        fromnode, tonode, kind = curr.split()
        ret.buffers.append((fromnode, tonode, kind))
    return ret

def parse_input(filename):
    data = []
    with open(filename, 'r') as inpfile:
        data = [x.strip() for x in inpfile]
    ret = InParams()
    # set graph limits
    curr, *data = data
    ll_x, ll_y, ur_x, ur_y = [int(x) for x in curr.split()]
    ret.limits = (ll_x, ll_y, ur_x, ur_y)
    # source position
    curr, *data = data
    tag, name, xx, yy, bname = curr.split()
    xx, yy = int(xx), int(yy)
    ret.source = (name, xx, yy, bname)
    # sinks
    curr, *data = data
    tag, _, num = curr.split()
    num = int(num)
    for _ in range(num):
        curr, *data = data
        name, xx, yy, cap = curr.split()
        xx, yy = int(xx), int(yy)
        ret.sinks.append((name, xx, yy, cap))
    # blockages
    while True:
        curr, *data = data
        if 'blockage' in curr:
            break
    tag, name, num = curr.split()
    num = int(num)
    for _ in range(num):
        curr, *data = data
        llx, lly, urx, ury = [int(x) for x in curr.split()]
        ret.blockages.append((llx, lly, urx, ury))
    return ret

def plot_input(filename, outfilename):
    fig, ax = plt.subplots()
    data = parse_input(filename)
    llx, lly, urx, ury = data.limits
    tot_len = max(urx-llx, ury-lly)
    pad = int(tot_len*0.1)
    plt.xlim(llx-pad, urx+pad)
    plt.ylim(lly-pad, ury+pad)
    name, xx, yy, bname = data.source
    plt.plot(xx, yy, 'ro', markersize=15)
    for sink in data.sinks:
        name, xx, yy, cap = sink
        plt.plot(xx, yy, 'bx')
    for blockage in data.blockages:
        llx, lly, urx, ury = blockage
        ax.add_patch(Rectangle((llx, lly), urx-llx, ury-lly, color="cyan"))
    plt.savefig(outfilename)

def plot_output(filename, infilename, outfilename):
    fig, ax = plt.subplots()
    indata = parse_input(infilename)
    outdata = parse_output(filename)
    
    llx, lly, urx, ury = indata.limits
    tot_len = max(urx-llx, ury-lly)
    pad = int(tot_len*0.1)
    plt.xlim(llx-pad, urx+pad)
    plt.ylim(lly-pad, ury+pad)
    
    # plot the source
    name, xx, yy, bname = indata.source
    plt.plot(xx, yy, 'ro', markersize=15)
    
    nodes = {}
    
    # handle sinks and plot them
    sinkinfo = {}
    for sink in indata.sinks:
        name, xx, yy, cap = sink
        sinkinfo[name] = (xx, yy)
        plt.plot(xx, yy, 'bx')

    for sink in outdata.sinks:
        nodename, sinkname = sink
        nodes[nodename] = sinkinfo[sinkname]
        
    # plot all the blockages
    for blockage in indata.blockages:
        llx, lly, urx, ury = blockage
        ax.add_patch(Rectangle((llx, lly), urx-llx, ury-lly, color="cyan"))

        
    _, xx, yy, _ = indata.source
    nodes[outdata.sourcename] = (xx, yy)
        
    # plot all interior node, we will overwrite for special nodes
    # like buffers
    for node in outdata.nodes:
        name, xx, yy = node
        nodes[name] = (xx, yy)
        plt.plot(xx, yy, 'cs')
    
    # join nodes using wire segments
    for wire in outdata.wires:
        # we are not considering the type of wire for now
        fromnode, tonode, kind = wire
        pt_fx, pt_fy = nodes[fromnode]
        pt_tx, pt_ty = nodes[tonode]
        plt.plot((pt_fx, pt_tx), (pt_fy, pt_ty), 'g-')
    
    # plot the buffers
    for buffer in outdata.buffers:
        # we are not considering the type of buffer for now
        fromnode, _, kind = buffer
        # for buffer insertion, the from and to nodes are the same
        xx, yy = nodes[fromnode]
        plt.plot(xx, yy, 'yo', markersize='10')
    
    plt.savefig(outfilename)

def main():
    parser = ArgumentParser()

    parser.add_argument("--mode", help="`input` or `output`")
    parser.add_argument("--infile", help="ISPD input file")
    parser.add_argument("--outfile", help="ISPD output file (only if mode is `output`)")
    parser.add_argument("--output", help="print plot to this file")
    args = parser.parse_args()

    if args.mode == 'input':
        plot_input(args.infile, args.output)
    else:
        plot_output(args.outfile, args.infile, args.output)

if __name__ == '__main__':
    main()
