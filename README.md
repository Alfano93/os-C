# os-C

This project seeks to create an easy, standard, way for applications written in the C programming language to interact with an Openstack enviroment.Seeing how I am only one person, and a busy person at that, we will see how far I get with this.

# openrc.c #

This file must be updated with the values unique to your enviroment. Think of this as your openrc file that you 'source' when using the Openstack
CLIs. 

# jansson-2.7 #

The jansson-2.7 library must be built and configured for the node that os-C will be used on. From the jansson-2.7 directory do:
 (libtools, autoconf, and make must be installed on the node)
 `autoreconf -vi`
 `./configure`
 `make`
 `make check`
 `make install`
 
# libcurl #
 
 Libcurl also needs to be installed on the node. Do `curl-config --cflags` to see the possible packages and install one with apt-get. 
 
# One more thing... #
 
 make sure you have the compiler installed...
