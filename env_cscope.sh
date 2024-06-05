rm -rf tags
rm -rf cscope.files
rm -rf cscope.in.out
rm -rf cscope.out
rm -rf cscope.po.out
tag=`git branch | grep "*" | awk '{print $NF}'`
echo $tag > ./current_tag
ctags -R
#CWD=$(pwd)	#/root/scsi_storage/workspace/iostat_issue/staging/linux-uek
CWD=.
find $CWD 	\
     -path "$CWD/arch/a*" -prune -o	\
     -path "$CWD/arch/b*" -prune -o	\
     -path "$CWD/arch/c*" -prune -o	\
     -path "$CWD/arch/f*" -prune -o	\
     -path "$CWD/arch/h*" -prune -o	\
     -path "$CWD/arch/i*" -prune -o	\
     -path "$CWD/arch/m*" -prune -o	\
     -path "$CWD/arch/n*" -prune -o	\
     -path "$CWD/arch/o*" -prune -o	\
     -path "$CWD/arch/p*" -prune -o	\
     -path "$CWD/arch/s*" -prune -o	\
     -path "$CWD/arch/t*" -prune -o	\
     -path "$CWD/arch/u*" -prune -o	\
     -path "$CWD/arch/xtensa" -prune -o	\
     -path "$CWD/tools/*" -prune -o	\
     -name "*.[c,h,s,S]"  > cscope.files
cscope -b -q
