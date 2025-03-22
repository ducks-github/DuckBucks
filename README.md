Duckbucks integration/staging tree
================================


Copyright (c) 2009-2014 Bitcoin Developers
Copyright (c) 2011-2014 Duckbucks Developers

What is Duckbucks?
----------------

Duckbucks is a lite version of Bitcoin using scrypt as a proof-of-work algorithm.
 - 2.5 minute block targets
 - subsidy halves in 694200 blocks (~4 years)
 - 69,420,666 total coins
 - 52 coins per block
 - 2016 blocks to retarget difficulty
 - Uses port 6335

For more information, as well as an immediately useable, binary version of
the Duckbucks client software, see http://www.duckbucks.org.

Mimblewimble Privacy Feature
---------------------------

Duckbucks now includes Mimblewimble protocol integration for enhanced privacy and scalability:

 - Confidential Transactions hide the transaction amounts
 - Improved blockchain scalability through transaction aggregation
 - Enhanced privacy for users through the Dandelion++ protocol
 - Accessible through both GUI and RPC interfaces

Available Mimblewimble RPC commands:
 - `runmwtest` - Run a test transaction using Mimblewimble
 - `getmwbalance` - Get your private Mimblewimble balance
 - `createmwoutput` - Create a test Mimblewimble output
 - `createmwtransaction` - Create a Mimblewimble private transaction

AppImage Installation
-------------------

We provide AppImage releases for easy installation on Linux systems:

1. Download the appropriate AppImage from the releases page:
   - `duckbucks_v02.AppImage` - Standard version
   - `duckbucks_mw.AppImage` - Version with Mimblewimble privacy features

2. Make the AppImage executable:
   ```bash
   chmod +x duckbucks_mw.AppImage
   ```

3. Run the AppImage:
   ```bash
   ./duckbucks_mw.AppImage
   ```

No installation is required, and the application will run on most modern Linux distributions.

Building Cross-Compatible AppImages
----------------------------------

The enhanced build script creates AppImages that work across a wide range of Linux distributions by bundling all necessary dependencies. To build a fully compatible AppImage:

### Prerequisites

Install the required build dependencies:

```bash
# For Debian/Ubuntu-based systems
sudo apt-get update
sudo apt-get install -y build-essential libssl-dev libdb++-dev libboost-all-dev \
  qt5-default libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools \
  libqrencode-dev libprotobuf-dev protobuf-compiler wget fuse libfuse2 \
  imagemagick libqt5svg5-dev

# For Fedora/RHEL-based systems
sudo dnf install -y gcc-c++ openssl-devel libdb-cxx-devel boost-devel \
  qt5-qtbase-devel qt5-qttools-devel qrencode-devel protobuf-devel \
  protobuf-compiler wget fuse fuse-libs imagemagick qt5-qtsvg-devel
```

### Building the AppImage

1. Clone the repository and build the Qt application:
   ```bash
   git clone https://github.com/ducks-github/DuckBucks.git
   cd DuckBucks
   qmake
   make -j$(nproc)
   ```

2. Run the enhanced AppImage creation script:
   ```bash
   chmod +x create_mw_appimage.sh
   ./create_mw_appimage.sh
   ```

3. The script will:
   - Check for required tools (wget, ImageMagick, FUSE)
   - Create a properly structured AppDir with all necessary components
   - Include AppStream metadata for better desktop integration
   - Bundle required libraries to ensure compatibility across distributions
   - Set up compatibility hooks for various Linux environments
   - Create the final `duckbucks_mw.AppImage` file

### Troubleshooting

If you encounter issues:

- **Missing FUSE**: Install FUSE with `sudo apt-get install fuse libfuse2` (Debian/Ubuntu) or `sudo dnf install fuse fuse-libs` (Fedora/RHEL)
- **Runtime library errors**: The AppImage includes compatibility hooks but may need additional libraries. Run with `--appimage-extract` to extract the AppImage contents and examine library dependencies
- **GUI issues**: Ensure Qt5 is properly installed on your system
- **Permission denied**: Make sure the AppImage is executable (`chmod +x duckbucks_mw.AppImage`)

### Custom Builds

You can customize the AppImage by modifying `create_mw_appimage.sh`:

- Change the AppImage version by setting `export VERSION=X.Y.Z` before running the script
- Add additional libraries by modifying the `copy_binary()` function
- Customize the application metadata in the AppStream XML file

Creating Your Own AppImage
-------------------------

If you want to build your own AppImage from source:

1. Build the Qt application:
   ```bash
   qmake
   make
   ```

2. Run the AppImage creation script:
   ```bash
   ./create_mw_appimage.sh
   ```

This will create a `duckbucks_mw.AppImage` file in the project root directory.

License
-------

Duckbucks is released under the terms of the MIT license. See `COPYING` for more
information or see http://opensource.org/licenses/MIT.

Development process
-------------------

Developers work in their own trees, then submit pull requests when they think
their feature or bug fix is ready.

If it is a simple/trivial/non-controversial change, then one of the Duckbucks
development team members simply pulls it.

If it is a *more complicated or potentially controversial* change, then the patch
submitter will be asked to start a discussion with the devs and community.

The patch will be accepted if there is broad consensus that it is a good thing.
Developers should expect to rework and resubmit patches if the code doesn't
match the project's coding conventions (see `doc/coding.txt`) or are
controversial.

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/duckbucks-project/duckbucks/tags) are created
regularly to indicate new official, stable release versions of Duckbucks.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test. Please be patient and help out, and
remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write unit tests for new code, and to
submit new unit tests for old code.

Unit tests for the core code are in `src/test/`. To compile and run them:

    cd src; make -f makefile.unix test

Unit tests for the GUI code are in `src/qt/test/`. To compile and run them:

    qmake BITCOIN_QT_TEST=1 -o Makefile.test bitcoin-qt.pro
    make -f Makefile.test
    ./duckbucks-qt_test
