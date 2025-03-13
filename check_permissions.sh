# Create and check config directory
CONFIG_DIR="$HOME/.duckbucks"
mkdir -p "$CONFIG_DIR"
ls -la "$CONFIG_DIR"

# Set correct permissions
chmod 700 "$CONFIG_DIR"

# Create basic config if doesn't exist
if [ ! -f "$CONFIG_DIR/duckbucks.conf" ]; then
    cat > "$CONFIG_DIR/duckbucks.conf" << EOF
rpcuser=duckbucksuser
rpcpassword=$(openssl rand -hex 32)
rpcallowip=127.0.0.1
server=1
listen=1
daemon=1
EOF
    chmod 600 "$CONFIG_DIR/duckbucks.conf"
fi

# Verify permissions
ls -la "$CONFIG_DIR/duckbucks.conf"