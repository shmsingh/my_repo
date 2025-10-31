#!/bin/bash

# Author: Shaleen Bathla <shaleen.bathla@oracle.com>

# TODO: specify image like OL8U4 etc
# TODO: Show installation progress
# TODO: Network configuration like VF specification

# ISO_LOCATION="http://ca-cobbler-whq.us.oracle.com/cobbler/links/+OL-R8-U8-x86_64/"
#ISO_LOCATION="/root/mountpt/fidelity_bug/OracleLinux-R9-U6-x86_64-dvd.iso"
IMAGE_LOCATION="/root/mountpt/images"
ISO_LOCATION="/root/mountpt/fidelity_bug/OracleLinux-R10-U0-x86_64-dvd.iso"

URL=qemu:///system

# create VM : first arg is VM name
create_vm () {
        if [ $# -eq 0 ]; then
                echo "$FUNCNAME(): vm name is a required argument."
                return -1
        fi

        echo "$FUNCNAME(): qemu-img create -f qcow2 "$IMAGE_LOCATION/$1.qcow2" 20G"
        qemu-img create -f qcow2 "$IMAGE_LOCATION/$1.qcow2" 50G
        if [ $? -eq 0 ]; then
                echo "$FUNCNAME(): disk created!"
        else
                echo "$FUNCNAME(): disk creation failed!"
                return -1
        fi

        echo "$FUNCNAME(): downloading kickstarter configuration..."
        if [[ ! -f /var/lib/libvirt/ks.cfg ]]; then
                wget https://kernel.us.oracle.com/~shmsingh/create_vm/ks.cfg -P /var/lib/libvirgt/
        fi
        echo "$FUNCNAME(): installing VM using virt-install..."

        virt-install --connect $URL \
        --virt-type=kvm \
        --name=$1 \
        --vcpus=2 \
        --memory=4096 \
        --location $ISO_LOCATION \
        --console pty,target_type=serial \
        --disk path=$IMAGE_LOCATION/$1.qcow2,format=qcow2,size=50 \
        --network default \
        --graphics none \
        --noautoconsole \
        --initrd-inject=/var/lib/libvirt/ks.cfg \
        --os-variant="ol10.0" \
        --extra-args="inst.ks=file:/ks.cfg console=ttyS0,115200" &

        #--os-variant=rhel7.9 \
        #--os-variant detect=on \
        #--os-variant detect=on,require=on \
        #--debug \
        #--print-xml --dry-run --debug \
        sleep 10
        echo "create_vm(): installing VM in background..."
        # echo "TODO: display installation progress..."
}

#i=1

#while [ 1 ]
#do
#       echo "###################creating vm ol7_$i"
#       create_vm "ol7_$i";
#       i=$(( $i+1 ));
#done

create_vm $1
