cd src/drivers/ps2

sed -i 's/fileXioOpen/open/g' *.c
sed -i 's/fileXioClose/close/g' *.c
sed -i 's/fileXioRead/read/g' *.c
sed -i 's/fileXioWrite/write/g' *.c
sed -i 's/fileXioLseek64/lseek/g' *.c
sed -i 's/fileXioLseek/lseek/g' *.c
#sed -i 's/fileXioGetStat/stat/g' *.c

sed -i 's/fileXioRemove/unlink/g' *.c
sed -i 's/fileXioRename/rename/g' *.c

#sed -i 's/fileXioDopen/opendir/g' *.c
#sed -i 's/fileXioDclose/closedir/g' *.c
#sed -i 's/fileXioDread/readdir/g' *.c
sed -i 's/fileXioMkdir/mkdir/g' *.c
#sed -i 's/fileXioRmdir/rmdir/g' *.c

sed -i 's/fioOpen/open/g' *.c
sed -i 's/fioClose/close/g' *.c
sed -i 's/fioRead/read/g' *.c
sed -i 's/fioWrite/write/g' *.c
sed -i 's/fioLseek64/lseek/g' *.c
sed -i 's/fioLseek/lseek/g' *.c
#sed -i 's/fioGetStat/stat/g' *.c

sed -i 's/fioRemove/unlink/g' *.c
sed -i 's/fioRename/rename/g' *.c

#sed -i 's/fioDopen/opendir/g' *.c
#sed -i 's/fioDclose/closedir/g' *.c
#sed -i 's/fioDread/readdir/g' *.c
sed -i 's/fioMkdir/mkdir/g' *.c
#sed -i 's/fioRmdir/rmdir/g' *.c

cd ..
