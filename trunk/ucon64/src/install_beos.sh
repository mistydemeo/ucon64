#!/bin/sh

	echo "Installing in a BeOS system..."
	cp ucon64 $HOME/config/bin
	if [ -e "$HOME/.ucon64rc" ] ; then
		cp $HOME/.ucon64rc $HOME/.ucon64rc.old
	fi
	cp .ucon64rc $HOME
	echo "Done."
