#!/bin/bash

# Colors for better readability
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Function to show help
show_help() {
    echo -e "${GREEN}DuckBucks Wallet - Simple Launcher${NC}"
    echo "----------------------------------------"
    echo "Usage:"
    echo "  ./start_duckbucks.sh          - Start the wallet normally"
    echo "  ./start_duckbucks.sh help     - Show this help message"
    echo "  ./start_duckbucks.sh reset    - Reset wallet settings (keeps your coins safe)"
    echo ""
    echo "Troubleshooting:"
    echo "  If the wallet doesn't start, try:"
    echo "  1. ./start_duckbucks.sh reset"
    echo "  2. Make sure you have enough disk space"
    echo "  3. Check if another DuckBucks wallet is running"
    echo ""
    echo "Need more help? Visit: https://duckbucks.org/support"
}

# Function to check if wallet is already running
check_running() {
    if pgrep -f "duckbucks_mw.AppImage" > /dev/null; then
        echo -e "${YELLOW}Warning: DuckBucks wallet is already running!${NC}"
        echo "Please close the existing wallet first."
        exit 1
    fi
}

# Function to ensure directories exist
setup_directories() {
    echo -e "${GREEN}Setting up DuckBucks directories...${NC}"
    mkdir -p "$HOME/.duckbucks"
    mkdir -p "$HOME/.duckbucks/database"
    chmod 700 "$HOME/.duckbucks"
    chmod 700 "$HOME/.duckbucks/database"
}

# Function to backup wallet
backup_wallet() {
    if [ -f "$HOME/.duckbucks/wallet.dat" ]; then
        echo -e "${GREEN}Creating backup of your wallet...${NC}"
        mkdir -p "$HOME/.duckbucks/backups"
        cp "$HOME/.duckbucks/wallet.dat" "$HOME/.duckbucks/backups/wallet.dat.backup.$(date +%Y%m%d_%H%M%S)"
    fi
}

# Function to reset wallet settings
reset_wallet() {
    echo -e "${YELLOW}Resetting wallet settings...${NC}"
    echo "Your coins are safe! This only resets the wallet configuration."
    
    # Backup wallet first
    backup_wallet
    
    # Remove old database files
    rm -f "$HOME/.duckbucks/database/__db*"
    rm -f "$HOME/.duckbucks/database/log.*"
    
    # Create fresh database environment
    echo -e "${GREEN}Setting up fresh database...${NC}"
    dd if=/dev/zero of="$HOME/.duckbucks/database/__db.001" bs=8192 count=1
    dd if=/dev/zero of="$HOME/.duckbucks/database/__db.002" bs=8192 count=1
    dd if=/dev/zero of="$HOME/.duckbucks/database/__db.003" bs=8192 count=1
    
    # Set proper permissions
    chmod 600 "$HOME/.duckbucks/database/__db.*"
    
    echo -e "${GREEN}Reset complete! Starting wallet...${NC}"
}

# Main script
echo -e "${GREEN}Starting DuckBucks Wallet...${NC}"

# Check command line arguments
if [ "$1" = "help" ]; then
    show_help
    exit 0
elif [ "$1" = "reset" ]; then
    check_running
    setup_directories
    reset_wallet
else
    check_running
    setup_directories
fi

# Start the wallet
echo -e "${GREEN}Launching DuckBucks Wallet...${NC}"
./duckbucks_mw.AppImage -datadir="$HOME/.duckbucks" -splash=0 -nosplash

# If wallet exits with error, try recovery
if [ $? -ne 0 ]; then
    echo -e "${YELLOW}Wallet encountered an error. Attempting recovery...${NC}"
    reset_wallet
    ./duckbucks_mw.AppImage -datadir="$HOME/.duckbucks" -splash=0 -nosplash -salvagewallet
fi 