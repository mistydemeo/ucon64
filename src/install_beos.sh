#!/bin/sh
# BeOS R5 install script for ucon64
cd $(dirname "$0")

areply=$(alert "This will start installation of ucon64 in a BeOS system.

ucon64 will be installed in $HOME/config/bin and .ucon64rc copied to $HOME.

Do you want to continue?" "Cancel" "Install")
if [ "$areply" ==  "Install" ]; then
	cp $(pwd)/ucon64 $HOME/config/bin
	if [ -e "$HOME/.ucon64rc" ] ; then
		cp $HOME/.ucon64rc $HOME/.ucon64rc.old
	fi
	cp $(pwd)/.ucon64rc $HOME
	# ask for ioport driver installation
	drreply=$(alert "For parallel port access with ucon64, you have to install the BeOS ioport driver by Caz Jones.

Would you like to do that?" "No" "Yes, install driver")
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
	
	alert "Done.
You can use ucon64 from the terminal now."
fi
