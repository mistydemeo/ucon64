#!/bin/sh
# BeOS R5 install script for uCON64
cd $(dirname "$0")
CONFIG_DIR=~/.ucon64

areply=$(alert "This will start installation of uCON64 in a BeOS system.

uCON64 will be installed in $HOME/config/bin.

Do you want to continue?" "Cancel" "Install")
if [ "$areply" ==  "Install" ]; then
	cp $(pwd)/ucon64 $HOME/config/bin
	# ask for ioport driver installation
	drreply=$(alert "uCON64 needs the BeOS ioport driver by Caz Jones.

Would you like to do install it?" "No" "Yes, install driver")
	if [ "$drreply" ==  "Yes, install driver" ]; then
		if [ -e "$HOME/ioport" ]; then
			cd "$HOME/ioport/driver"
			install_ioport
		else
			if [ -e "$HOME/ioport.zip" ]; then
				unzip $HOME/ioport.zip -d $HOME
				cd "$HOME/ioport/driver"
				install_ioport
			else
				alert "Please download the latest version of the ioport driver from either
http://www.infernal.currantbun.com or
http://ucon64.sourceforge.net
and copy it to $HOME. Then retry to install the driver."
				exit
			fi
		fi
	fi

if test ! -e $CONFIG_DIR; then
  mkdir $CONFIG_DIR
fi
cp libdiscmage/libdiscmage.so $CONFIG_DIR

	alert "Done.
You can use uCON64 from the terminal now."
fi
