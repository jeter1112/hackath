#!/bin/bash

set -e


for driver in $(find ${BACKPORT_DIR} -type f -name *.ko); do
	mod_name=${driver/${BACKPORT_DIR}/${KLIB}${KMODDIR}}${compr}
	echo "  uninstall" $mod_name
	rm -f $mod_name
done
