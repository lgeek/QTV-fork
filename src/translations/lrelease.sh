#!/bin/sh

echo "# running lrelease"
lrelease ../../QTV.pro -verbose
echo "# moving binaries to 'bin' folder'"
mv *.qm ../../bin/
echo "DONE"
