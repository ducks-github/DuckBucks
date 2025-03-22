#!/bin/bash
set -e

# Check if the binary exists
if [ ! -f "build-qt/duckbucks-qt" ]; then
    echo "Error: build-qt/duckbucks-qt does not exist. Please build the project first."
    exit 1
fi

# Create AppDir structure
echo "Creating AppDir structure..."
mkdir -p AppDir/usr/bin
mkdir -p AppDir/usr/share/applications
mkdir -p AppDir/usr/share/icons/hicolor/256x256/apps

# Create desktop file
echo "Creating .desktop file..."
cat > AppDir/usr/share/applications/duckbucks-mw.desktop << EOF
[Desktop Entry]
Type=Application
Name=DuckBucks MW
Comment=DuckBucks P2P Cryptocurrency with Mimblewimble Privacy
Exec=duckbucks-qt
Icon=duckbucks
Categories=Finance;
Terminal=false
EOF

# Copy the desktop file to AppDir root as required by AppImage format
cp AppDir/usr/share/applications/duckbucks-mw.desktop AppDir/duckbucks.desktop

# Create icon file if not exist
ICON_SOURCE="src/qt/res/icons/bitcoin.png"
if [ -f "$ICON_SOURCE" ]; then
    echo "Copying icon file..."
    cp "$ICON_SOURCE" AppDir/usr/share/icons/hicolor/256x256/apps/duckbucks.png
else
    echo "Warning: Icon file not found at $ICON_SOURCE. Using placeholder."
    # Create a placeholder icon if none exists
    convert -size 256x256 xc:gold -fill white -gravity center -pointsize 40 -annotate 0 "DuckBucks MW" AppDir/usr/share/icons/hicolor/256x256/apps/duckbucks.png
fi

# Copy the binary
echo "Copying binary..."
cp build-qt/duckbucks-qt AppDir/usr/bin/

# Create AppRun
echo "Creating AppRun..."
cat > AppDir/AppRun << EOF
#!/bin/bash
APPDIR=\$(dirname \$(readlink -f "\$0"))
exec "\$APPDIR/usr/bin/duckbucks-qt" "\$@"
EOF
chmod +x AppDir/AppRun

# Download or use existing linuxdeploy
if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
    echo "Downloading linuxdeploy..."
    wget -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    chmod +x linuxdeploy-x86_64.AppImage
fi

# Download or use existing linuxdeploy-plugin-qt
if [ ! -f "linuxdeploy-plugin-qt-x86_64.AppImage" ]; then
    echo "Downloading linuxdeploy-plugin-qt..."
    wget -c "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
    chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
fi

# Create AppImage with custom name
echo "Creating AppImage..."
OUTPUT="duckbucks_mw.AppImage" ./linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt --output appimage

echo "AppImage created!" 