#!/usr/bin/python
#

import sys
from diskfmt import *
import ctypes
from memcrc import memcrc
blksize = 4096
blocks = []
nblks = 0
blkmap = None
sb = None
    
valid_chars = ".0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
valid_bytes = [ord(_) for _ in valid_chars]

def valid_string(s):
    if not 0 in s:
        return False
    for c in s:
        if c == 0:
            return False
        if not c in valid_bytes:
            return False
    return True

DES_PER_BLK = blksize // ctypes.sizeof(dirent)

def byte2str(b):
    s = ''
    for _b in b:
        if _b == 0:
          return s
        s += chr(_b)
    return s

msgs = []
def explore(inum, path):
    blkmap[inum] = True
    _in = inode.from_buffer(bytearray(blocks[inum]))
    
    # check that unused pointers are 0
    n = (_in.size+blksize-1) // blksize
    for i in range(n,1019):
        if _in.ptrs[i] != 0:
            print("ERROR: %d (%s) ptr[%d] = %d (nblks=%d)" %
                      (inum, path, i, _in.ptrs[i], n))

    if S_ISREG(_in.mode):
        bb = [_in.ptrs[i] for i in range(n)]
        for b in bb:
            blkmap[b] = True
            
        msg = ("inode %d: file, '%s'\n" % (inum,path))
        msg += (" mode %o len %d uid %d gid %d mtime %d\n" %
                    (_in.mode, _in.size, _in.uid, _in.gid, _in.mtime))
        msg += (" blocks: %s\n" % str([_in.ptrs[i] for i in range(n)]))

        _size = _in.size
        data = bytearray()
        for b in bb:
            data += bytearray(blocks[b][:_size])
            _size -= blksize
        msg += ("cksum: %d %d\n" % (memcrc(data),len(data)))
        msgs.append([inum, msg])
        
    elif S_ISDIR(_in.mode):
        path += '/'
        msg = ("inode %d: directory, '%s'\n" % (inum,path))
        msg += (" mode %o len %d uid %d gid %d mtime %d\n" %
                    (_in.mode, _in.size, _in.uid, _in.gid, _in.mtime))
        msg += (" blocks: %s\n" % str([_in.ptrs[i] for i in range(n)]))
        for i in range(n):
            b = _in.ptrs[i]
            blkmap[b] = True

            _des = (dirent*DES_PER_BLK).from_buffer(bytearray(blocks[b]))
            msg += ("  block %d:\n" % b)
            for i in range(DES_PER_BLK):
                _d = _des[i]
                if not _d.valid:
                    continue
                #if not valid_string(_d.name):
                #    print("ERROR: dir '%s' (%d) invalid name" % (path, inum),
                #              _d.name)
                name = byte2str(_d.name)
                msg += ("    [%d] %s -> %d\n" % (i, name, _d.inode))
                explore(_d.inode, path + name)
        msgs.append([inum,msg])
    else:
        print("ERROR: path '%s' (%d) invalid mode %o" % (path, inum, _in.mode))

def parse_disk(filename):
    global blocks, nblks, blkmap
    fp = open(filename, 'rb')
    data = fp.read()
    fp.close()
    blocks = [data[i*blksize:(i+1)*blksize] for i in range(len(data)//blksize)]
    nblks = len(blocks)
    blkmap = [False] * nblks

    sb = super.from_buffer(bytearray(blocks[0]))
    root_inum = sb.root_inode
    blkmap[0] = True

    explore(root_inum, "")

    live_blocks = [x for x in filter(lambda i: blkmap[i], range(nblks))]
    print('blocks used:', live_blocks)
    blank = bytearray(blksize)
    for i in range(nblks):
        if not blkmap[i] and blocks[i] != blank:
            print("ERROR: block %d not zeroes" % i)

    print()
    msgs.sort()
    for i,msg in msgs:
        print(msg)

if __name__ == '__main__':
    parse_disk(sys.argv[-1])
