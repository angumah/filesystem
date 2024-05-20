#!/usr/bin/python
#

import sys
from diskfmt import *

blksize = 4096

def div_round_up(n, m):
    return int((n+m-1)//m)

#int(sys.argv[1])

nblocks = 1024

sb = super()
sb.magic = 0x33363530
sb.num_blocks = nblocks
sb.root_inode = 1

output_blocks = [bytearray(4096) for i in range(nblocks)]
output_blocks[0] = bytearray(sb)

import random
random.seed(17)                           # the most random number :-)

data_start = 2
free_blocks = [i for i in range(data_start, nblocks)]
random.shuffle(free_blocks)

def get_free_block():
    return free_blocks.pop(0)

t1 = 100000000
t2 = 200000000
S_IFREG = 0o0100000  # regular file
S_IFDIR = 0o0040000  # directory

perm1 = 0o777
perm2 = 0o755
perm3 = 0o640

# directories:
rootnames = [("foo", 'x', 0, 0), ("dir", 'd', perm1, 0), ("bar", 'x', 0, 0),
                 ("dir1", 'd', perm2, 0), ("file.1", 'f', perm1, 900),
                 ("xyz22", 'x', 0, 0), ("dir-with-long-name", 'd', perm2, 0),
                 ("dir2", 'd', perm2, 0), ("file.2", 'f', perm3, 2000)]

inode_dict = dict()
inode_dict[""] = 1

file_blocks = dict()
file_data = dict()
dir_contents = dict()
inodes = dict()

all_paths = []
path_type = dict()

from functools import reduce
def checksum(st):
    #return reduce(lambda x,y:x+y, map(ord, st))
    return sum(st)

def alloc_inode():
    return get_free_block()

def xbytes(s):
    return bytes(s, 'UTF-8')

def do_dir(_path, dirents, names):
    print('do_dir', _path)
    for i in range(len(names)):
        name,typ,perm,sz = names[i]
        de = dirents[i]
        de.name = xbytes(name)
        path = _path + '/' + name
        if typ == 'x':
            inum = random.randint(2, nblocks-1)
            de.valid = 0
            de.isDir = 1
            de.inode = inum
        else:
            path_type[path] = typ
            all_paths.append(path)
            inum = alloc_inode()
            inode_dict[path] = inum
            de.valid = 1
            de.isDir = 1 if typ == 'd' else 0
            de.inode = inum
            _in = inode()
            _in.uid = 1001
            _in.gid = 1002
            _in.mode = perm | (S_IFREG if typ == 'f' else S_IFDIR)
            _in.ctime = t1 + random.randint(0,100000)
            _in.mtime = t2 + random.randint(0,100000)
            if typ == 'd':
                sz = 4096
            _in.size = sz
            inodes[inum] = _in
            nblks = div_round_up(sz, 4096)
            blks = [get_free_block() for _ in range(nblks)]
            if typ == 'f':
                file_blocks[inum] = blks
                data = bytearray(sz)
                for i in range(sz):
                    data[i] = random.randint(32, 126)
                file_data[path] = data
            else:
                dir_blocks[inum] = blks[0]
                _in.ptrs[0] = blks[0]

dir_blocks = dict()
dir_data = dict()

dir_blocks[1] = get_free_block()

dirsz = blksize // sizeof(dirent)
rootdir = [dirent() for i in range(dirsz)]
do_dir("", rootdir, rootnames)
#dir_blocks[""] = get_free_block()
_data = bytearray(4096)
offset = 0
for _de in rootdir:
    _data[offset:offset+32] = bytearray(_de)
    offset += 32
dir_data[""] = _data

path = "/dir"
dirnames = [("xy", 'x', 0, 0), ('file1', 'f', perm1, 100), ('yz', 'x', 0, 0),
                ('file2', 'f', perm2, 1200), ('file3', 'f', perm1, 10111)]
dirdir = [dirent() for i in range(blksize // sizeof(dirent))]
do_dir(path, dirdir, dirnames)
#n = get_free_block()
#dir_blocks[path] = n
_data = bytearray(4096)
offset = 0
for _de in dirdir:
    _data[offset:offset+32] = bytearray(_de)
    offset += 32
dir_data[path] = _data

path = "/dir1"
dir1names = [("long-file-name", 'f', perm1, 1025), ("subdir", 'd', perm2, 0)]
dir1dir = [dirent() for i in range(blksize // sizeof(dirent))]
do_dir("/dir1", dir1dir, dir1names)
#n = get_free_block()
#dir_blocks["/dir1"] = n
_data = bytearray(4096)
offset = 0
for _de in dir1dir:
    _data[offset:offset+32] = bytearray(_de)
    offset += 32
dir_data["/dir1"] = _data

dir2names = [("twenty-seven-character-name", 'f', perm1, 100),
                 ("twenty-six--character-name", 'f', perm1, 100)]
dir2dir = [dirent() for i in range(blksize // sizeof(dirent))]
do_dir("/dir2", dir2dir, dir2names)
#n = get_free_block()
#dir_blocks["/dir2"] = n
_data = bytearray(4096)
offset = 0
for _de in dir2dir:
    _data[offset:offset+32] = bytearray(_de)
    offset += 32
dir_data["/dir2"] = _data

dirdnames = [("2nd-file-with-long-name", 'f', perm1, 200)]
dirddir = [dirent() for i in range(blksize // sizeof(dirent))]
do_dir("/dir-with-long-name", dirddir, dirdnames)
#n = get_free_block()
#dir_blocks["/dir-with-long-name"] = n
_data = bytearray(4096)
offset = 0
for _de in dirddir:
    _data[offset:offset+32] = bytearray(_de)
    offset += 32
dir_data["/dir-with-long-name"] = _data

for path in dir_data.keys():
    inum = inode_dict[path]
    _in = inode()
    _in.size = 4096
    if _in.mode == 0:
        _in.mode = S_IFDIR | 0o777
        _in.ctime = t1
        _in.mtime = t2
    print('dir:', path, '=', inum, 'mode', _in.mode)
    blknum = dir_blocks[inum]
    print('    in block', blknum)
    _in.ptrs[0] = blknum
    output_blocks[blknum] = dir_data[path]
    assert(len(output_blocks[blknum]) == 4096)

for b in output_blocks:
    assert(len(b) == 4096)

def write_data(inum, blk, data):
    b = file_blocks[inum][blk]
    _in = inodes[inum]
    _in.ptrs[blk] = b
    n = len(data)
    assert(n <= 4096)
    buf = output_blocks[b]
    buf[0:n] = data


for path in file_data.keys():
    inum = inode_dict[path]
    _in = inodes[inum]
    blklist = file_blocks[inum]
    data = file_data[path]
    _in.size = len(data)
    nb = div_round_up(len(data), blksize)

    print(path, len(data), checksum(data))
    
    offset = 0
    while len(data) > 0:
        _d = data[0:4096]
        data = data[4096:]
        write_data(inum, offset, _d)
        offset += 1

    for i in range(len(output_blocks)):
        assert(len(output_blocks[i]) == 4096)

root = inode()
root.mode = S_IFDIR | 0o777
root.size = 4096
root.ptrs[0] = dir_blocks[1]
inodes[1] = root

for i in inodes:
    print('inode', i, 'written')
    output_blocks[i] = bytearray(inodes[i])
    assert(len(output_blocks[i]) == 4096)
    
outfile = sys.argv[1]
fp = open(outfile, 'wb')
for i in range(len(output_blocks)):
    b = output_blocks[i]
    assert(len(b) == 4096)
    fp.write(b)
