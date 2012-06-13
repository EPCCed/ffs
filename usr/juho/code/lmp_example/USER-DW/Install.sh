# Install/unInstall package files in LAMMP

if (test $1 = 1) then

   cp -p bond_doublewell.cpp ..
   cp -p bond_doublewell.h ..

elif (test $1 = 0) then

   rm -f ../bond_doublewell.cpp
   rm -f ../bond_doublewell.h

fi