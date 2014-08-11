#!/bin/bash
#By shajf
BUILD_DIR=build

[ `id -u` -ne 0 ] && {
	echo "You must run this by root" >&2
	exit 1
}

install_deps(){
      #apt-get update

      apt-get install -y gawk || exit 2
      apt-get install -y make || exit 2
      apt-get install -y autoconf || exit 2
      apt-get install -y libtool || exit 2
      #apt-get install -y apache2-dev || exit 2
      #apt-get install -y apache2-threaded-dev || exit 2
      apt-get install -y libxml2-dev || exit 2
      apt-get install -y libcurl4-openssl-dev || exit 2
}

install_deps
cd $BUILD_DIR
sh -x build.sh


