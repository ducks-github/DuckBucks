#!/bin/bash
set -e

# Print informative messages with colors
print_info() {
    echo -e "\e[1;34m[INFO]\e[0m $1"
}

print_error() {
    echo -e "\e[1;31m[ERROR]\e[0m $1"
}

print_success() {
    echo -e "\e[1;32m[SUCCESS]\e[0m $1"
}

# Check for required tools
check_dependencies() {
    print_info "Checking for required tools..."
    
    # Check for wget
    if ! command -v wget &> /dev/null; then
        print_error "wget is not installed. Please install it first."
        exit 1
    fi
    
    # Check for imagemagick (for icon generation)
    if ! command -v convert &> /dev/null; then
        print_error "ImageMagick is not installed. Please install it first."
        exit 1
    fi
    
    # Check if FUSE is available (needed for AppImage tools)
    if ! grep -q "fuse" /proc/filesystems && ! command -v fusermount &> /dev/null; then
        print_error "FUSE is not available. It's needed to run AppImage tools."
        print_error "Please install FUSE or the fuse package for your distribution."
        exit 1
    fi
    
    print_success "All required tools are available."
}

# Check if the binary exists
check_binary() {
    print_info "Checking for Qt binary..."
    if [ ! -f "build-qt/duckbucks-qt" ]; then
        print_error "build-qt/duckbucks-qt does not exist. Please build the project first."
        print_info "You can build it by running:"
        print_info "  qmake"
        print_info "  make"
        exit 1
    fi
    print_success "Qt binary found."
}

# Create AppDir structure
create_appdir() {
    print_info "Creating AppDir structure..."
    mkdir -p AppDir/usr/bin
    mkdir -p AppDir/usr/lib
    mkdir -p AppDir/usr/share/applications
    mkdir -p AppDir/usr/share/icons/hicolor/256x256/apps
    mkdir -p AppDir/usr/share/metainfo
    mkdir -p AppDir/apprun-hooks
    print_success "AppDir structure created."
}

# Create AppStream metadata for better integration
create_appstream_metadata() {
    print_info "Creating AppStream metadata..."
    cat > AppDir/usr/share/metainfo/org.duckbucks.DuckBucksMW.appdata.xml << EOF
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>org.duckbucks.DuckBucksMW</id>
  <name>DuckBucks MW</name>
  <summary>Privacy-focused P2P cryptocurrency</summary>
  <metadata_license>MIT</metadata_license>
  <project_license>MIT</project_license>
  <description>
    <p>
      DuckBucks is a cryptocurrency featuring the Mimblewimble privacy protocol,
      which allows for confidential transactions with improved blockchain scalability.
    </p>
    <p>
      Key features include:
    </p>
    <ul>
      <li>Confidential Transactions hide transaction amounts</li>
      <li>Improved blockchain scalability through transaction aggregation</li>
      <li>Enhanced privacy with Dandelion++ protocol</li>
      <li>User-friendly interface for creating private transactions</li>
    </ul>
  </description>
  <url type="homepage">http://www.duckbucks.org</url>
  <provides>
    <binary>duckbucks-qt</binary>
  </provides>
  <categories>
    <category>Finance</category>
    <category>Qt</category>
  </categories>
</component>
EOF
    print_success "AppStream metadata created."
}

# Create desktop file
create_desktop_file() {
    print_info "Creating .desktop file..."
    cat > AppDir/usr/share/applications/duckbucks-mw.desktop << EOF
[Desktop Entry]
Type=Application
Name=DuckBucks MW
Comment=DuckBucks P2P Cryptocurrency with Mimblewimble Privacy
Exec=duckbucks-qt
Icon=duckbucks
Categories=Finance;Qt;
Terminal=false
StartupWMClass=DuckBucks-qt
X-AppImage-Version=${VERSION:-1.0}
EOF

    # Copy the desktop file to AppDir root as required by AppImage format
    cp AppDir/usr/share/applications/duckbucks-mw.desktop AppDir/duckbucks.desktop
    print_success "Desktop file created."
}

# Create or copy icon
handle_icon() {
    print_info "Setting up application icon..."
    ICON_SOURCE="src/qt/res/icons/bitcoin.png"
    if [ -f "$ICON_SOURCE" ]; then
        print_info "Copying icon file..."
        cp "$ICON_SOURCE" AppDir/usr/share/icons/hicolor/256x256/apps/duckbucks.png
    else
        print_info "Icon file not found at $ICON_SOURCE. Creating placeholder..."
        # Create a placeholder icon if none exists
        convert -size 256x256 xc:gold -fill white -gravity center -pointsize 40 -annotate 0 "DuckBucks MW" AppDir/usr/share/icons/hicolor/256x256/apps/duckbucks.png
    fi
    print_success "Icon set up completed."
}

# Copy the binary and add required library dependencies
copy_binary() {
    print_info "Copying binary and required libraries..."
    # Copy the main binary
    cp build-qt/duckbucks-qt AppDir/usr/bin/
    
    # Copy additional libraries that might not be automatically detected
    # Explicitly copy Berkeley DB
    if [ -f "/usr/lib/x86_64-linux-gnu/libdb_cxx.so.5.3" ]; then
        print_info "Copying Berkeley DB library..."
        cp /usr/lib/x86_64-linux-gnu/libdb_cxx.so.5.3 AppDir/usr/lib/
        ln -sf libdb_cxx.so.5.3 AppDir/usr/lib/libdb_cxx.so.5
    fi
    
    # Copy OpenSSL libraries (3.x might not be widely available on older distributions)
    if [ -f "/usr/lib/x86_64-linux-gnu/libssl.so.3" ]; then
        print_info "Copying OpenSSL libraries..."
        cp /usr/lib/x86_64-linux-gnu/libssl.so.3 AppDir/usr/lib/
        cp /usr/lib/x86_64-linux-gnu/libcrypto.so.3 AppDir/usr/lib/
    fi
    
    # Copy Boost libraries if needed
    print_info "Checking for Boost libraries..."
    ldd build-qt/duckbucks-qt | grep boost | awk '{print $3}' | xargs -I{} cp -v {} AppDir/usr/lib/ || true
    
    print_success "Binary and libraries copied."
}

# Create AppRun script with compatibility hooks
create_apprun() {
    print_info "Creating enhanced AppRun script..."
    cat > AppDir/AppRun << EOF
#!/bin/bash

# Find the AppDir and set paths
APPDIR=\$(dirname \$(readlink -f "\$0"))
export PATH="\$APPDIR/usr/bin:\$PATH"
export LD_LIBRARY_PATH="\$APPDIR/usr/lib:\$APPDIR/usr/lib/x86_64-linux-gnu:\$LD_LIBRARY_PATH"
export XDG_DATA_DIRS="\$APPDIR/usr/share:\$XDG_DATA_DIRS"
export QT_PLUGIN_PATH="\$APPDIR/usr/lib/qt5/plugins:\$QT_PLUGIN_PATH"
export QML2_IMPORT_PATH="\$APPDIR/usr/lib/qml:\$QML2_IMPORT_PATH"

# Run any hooks in the apprun-hooks directory
if [ -d "\$APPDIR/apprun-hooks" ]; then
    for hook in "\$APPDIR/apprun-hooks/"*.sh; do
        if [ -f "\$hook" ] && [ -x "\$hook" ]; then
            . "\$hook"
        fi
    done
fi

# Make Qt use the system theme on various desktop environments
export QT_QPA_PLATFORMTHEME=xdgdesktopportal

# Workaround for older distributions with incompatible libraries
for lib in "\$APPDIR/usr/lib"/*.so*; do
    if [ -f "\$lib" ] && ! [ -L "\$lib" ]; then
        lib_name=\$(basename "\$lib")
        if ! ldconfig -p | grep -q "\$lib_name"; then
            export LD_PRELOAD="\$lib:\$LD_PRELOAD"
        fi
    fi
done

# Execute the application
exec "\$APPDIR/usr/bin/duckbucks-qt" "\$@"
EOF
    chmod +x AppDir/AppRun
    print_success "Enhanced AppRun script created."
}

# Create a qt.conf file to ensure Qt finds its plugins
create_qt_conf() {
    print_info "Creating qt.conf..."
    mkdir -p AppDir/usr/bin
    cat > AppDir/usr/bin/qt.conf << EOF
[Paths]
Plugins = ../lib/qt5/plugins
Imports = ../lib/qml
Qml2Imports = ../lib/qml
Data = ../share
EOF
    print_success "qt.conf created."
}

# Create AppRun hook for compatibility with various distributions
create_apprun_hook() {
    print_info "Creating AppRun hook for library compatibility..."
    cat > AppDir/apprun-hooks/ld_library_path.sh << EOF
#!/bin/bash

# This hook ensures libraries are found on various distributions

# Special handling for OpenSSL
if [ ! -e "/usr/lib/libssl.so.3" ] && [ -e "\$APPDIR/usr/lib/libssl.so.3" ]; then
    export LD_LIBRARY_PATH="\$APPDIR/usr/lib:\$LD_LIBRARY_PATH"
fi

# Handle differences in Berkeley DB versions
if [ ! -e "/usr/lib/libdb_cxx.so.5" ] && [ -e "\$APPDIR/usr/lib/libdb_cxx.so.5.3" ]; then
    export LD_LIBRARY_PATH="\$APPDIR/usr/lib:\$LD_LIBRARY_PATH"
fi
EOF
    chmod +x AppDir/apprun-hooks/ld_library_path.sh
    print_success "AppRun hook created."
}

# Download or use existing linuxdeploy tools
get_linuxdeploy_tools() {
    print_info "Setting up AppImage creation tools..."
    # Download or use existing linuxdeploy
    if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
        print_info "Downloading linuxdeploy..."
        wget -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
        chmod +x linuxdeploy-x86_64.AppImage
    fi

    # Download or use existing linuxdeploy-plugin-qt
    if [ ! -f "linuxdeploy-plugin-qt-x86_64.AppImage" ]; then
        print_info "Downloading linuxdeploy-plugin-qt..."
        wget -c "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
        chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
    fi
    print_success "AppImage creation tools ready."
}

# Create the AppImage
create_appimage() {
    print_info "Creating AppImage with all dependencies included..."
    
    # Set environment variables for linuxdeploy-plugin-qt
    export QML_SOURCES_PATHS="src/qt/qml"
    
    # Run linuxdeploy with Qt plugin and additional parameters
    OUTPUT="duckbucks_mw.AppImage" ./linuxdeploy-x86_64.AppImage \
        --appdir AppDir \
        --plugin qt \
        --output appimage \
        --verbosity=2
    
    print_success "AppImage creation process completed!"
}

# Main execution
main() {
    print_info "Starting DuckBucks MW AppImage creation process..."
    
    check_dependencies
    check_binary
    create_appdir
    create_appstream_metadata
    create_desktop_file
    handle_icon
    copy_binary
    create_qt_conf
    create_apprun
    create_apprun_hook
    get_linuxdeploy_tools
    create_appimage
    
    if [ -f "duckbucks_mw.AppImage" ]; then
        chmod +x duckbucks_mw.AppImage
        print_success "DuckBucks MW AppImage successfully created!"
        print_info "You can now run it with: ./duckbucks_mw.AppImage"
    else
        print_error "AppImage creation failed. Please check the errors above."
        exit 1
    fi
}

# Execute main function
main 