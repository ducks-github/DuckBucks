# Check core dependencies
dpkg -l | grep -E 'libboost|libssl|libdb|qt5|miniupnpc|libqrencode'

# Verify specific versions
openssl version  # Should be 1.0.1c or later
ldconfig -p | grep libdb  # Should show Berkeley DB 4.8
boost-version  # Should be 1.37 or later

# Check Qt installation
qmake --version