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
	# ask for parallel driver installation
	drreply=$(alert "For parallel Port access with ucon64, you have to install the BeOS parnew driver by Caz Jones.

Would you like to do that?" "No" "Yes, install driver")
	if [ "$drreply" ==  "Yes, install driver" ]; then
		cd "$(pwd)/beos_pardriver"
		install_wildcarddriver
	fi
	
	alert "Done.
You can use ucon64 from the terminal now."
fi
