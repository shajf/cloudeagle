LIB_URL=http://10.2.252.0:89/
TAR_WIRESHARK=wireshark-1.10.8.tar.bz2
WIRESHARK_SRC=wireshark-1.10.8
WIRESHARK_PREFIX=/usr/local/wireshark

for s in ${TAR_WIRESHARK}; do
    if [ ! -s $s ] ; then
        download_path=${LIB_URL}$s
        echo "###Download ${download_path}"
        wget -c ${download_path} || exit 2 
    fi
done

build_wireshark(){
    echo 'compile wireshark and install it..............................'
    
    if [ ! -d ${WIRESHARK_SRC} ] ; then
        tar -jxf ${TAR_WIRESHARK} || exit 2
    else
        echo 'rebuild wireshark ......................................'
    fi
    
    cp ../src/print.h ${WIRESHARK_SRC}
    cp ../src/print.c ${WIRESHARK_SRC}
    cp ../src/tshark.c ${WIRESHARK_SRC}
    cp ../src/packet-* ${WIRESHARK_SRC}/epan/dissectors
    cp ../src/cJSON.* ${WIRESHARK_SRC}
    cp ../src/Makefile.common ${WIRESHARK_SRC}
    cd ${WIRESHARK_SRC}
    sh -x autogen.sh
    ./configure --prefix=${WIRESHARK_PREFIX}  --enable-wireshark=false
    make
    make install
    cd ../
}

build_wireshark
