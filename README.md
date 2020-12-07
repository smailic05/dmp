# dmp
a target for device mapper that catch all requests and create stat

To creat a .ko file use 
`make`

After you to install module
`insmod /path/to/module/dmp.ko`

Create a test block device
`dmsetup create zero1 --table "0 20000 zero"`

Check if created
`ls -al /dev/mapper/*`

Then create a dmp device
`dmsetup create dmp1 --table "0 $size dmp /dev/mapper/zero1`

Requests for read/write
`# dd if=/dev/random of=/dev/mapper/dmp1 bs=4k count=1
0+1 records in
0+1 records out
115 bytes copied, 0.000241373 s, 476 kB/s
# dd of=/dev/null if=/dev/mapper/dmp1 bs=4k count=1
1+0 records in
1+0 records out
4096 bytes (4.1 kB, 4.0 KiB) copied, 0.000166921 s, 24.5 MB/s`

Statistics
`cat /sys/module/dmp/stat/volumes
read:
reqs: 100
avg size: 4096
write:
reqs: 10
avg size: 4096
total:
reqs: 110
avg size: 4096`
