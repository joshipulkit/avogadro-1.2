#!/usr/bin/bash

WORKDIR_AVOGADRO=`pwd`

echo "========================================="
echo "This is Avogadro v1.2 installation script."
echo "WARNING: You may be asked to enter sudo passwd multiple times."
echo "Complete installation process should take about a few minutes."
echo "========================================="
echo ""
echo ""

# Attach qt4 ppa to sources list
echo "Attaching PPA:rock-core-ubuntu-qt4 to sources list"
sudo cp $WORKDIR_AVOGADRO/rock-core-ubuntu-qt4-jammy.list /etc/apt/sources.list.d/
sudo cp $WORKDIR_AVOGADRO/rock-core-ubuntu-qt4.gpg /etc/apt/trusted.gpg.d/
echo "========================================="
echo ""
echo ""

# Installing necessary dependencies
echo "Installing necessary dependencies..."
sudo apt update
sudo apt install cmake zlib1g-dev qt4-dev-tools qt4-qmake qttool5-dev-tools
echo "Installing necessary dependencies complete!"
echo "========================================="
echo ""
echo ""

# Begin building dependencies
echo "Begin building Eigen 3.0..."
cd $WORKDIR_AVOGADRO/eigen-3.0/
mkdir build ; cd build
cmake ../
sudo make install
cd $WORKDIR_AVOGADRO
echo "Building Eigen 3.0 complete!"
echo "========================================="
echo ""
echo ""

echo "Begin building Openbabel-2.4.1..."
cd $WORKDIR_AVOGADRO/openbabel-2.4.1/
mkdir build ; cd build
cmake ../
make -j2
sudo make install
cd $WORKDIR_AVOGADRO
echo "Building Openbabel-2.4.1 complete!"
echo "========================================="
echo ""
echo ""

echo "Begin building Avogadro-1.2.0..."
cd $WORKDIR_AVOGADRO/avogadro-1.2.0/
mkdir build ; cd build
cmake ../
make -j2
sudo make install
cd $WORKDIR_AVOGADRO
echo "Building Avogadro-1.2.0 complete!"
echo "========================================="
echo ""
echo "Avogadro v1.2 installation complete. You can run it using terminal or using GUI version in your application list. Exiting successfully!"
