#!/bin/sh
ipcrm -S 0x5b051f9d
ipcrm -Q 0x7b051f9d
ipcrm -Q 0x7c051f9d
ipcrm -Q 0x7d051f9d
ipcrm -Q 0x7e051f9d
ipcrm -M 0x6b051f9d
ipcrm -M 0x6c051f9d
rm -f SIGHUP_PPID_lista_proc.txt