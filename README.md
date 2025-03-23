# DuckBucks Wallet

A secure and user-friendly cryptocurrency wallet for Linux.

## Quick Start

1. Make the launcher executable:
   ```bash
   chmod +x start_duckbucks.sh
   ```

2. Start the wallet:
   ```bash
   ./start_duckbucks.sh
   ```

## Restoring from Backup

If you need to restore your wallet from a backup:

1. Stop the DuckBucks wallet if it's running
2. Navigate to your backup location:
   ```bash
   cd ~/.duckbucks/backups
   ```
3. Find your backup file (it will be named like `wallet.dat.backup.YYYYMMDD_HHMMSS`)
4. Copy the backup file to replace your current wallet:
   ```bash
   cp wallet.dat.backup.YYYYMMDD_HHMMSS ~/.duckbucks/wallet.dat
   ```
5. Start the wallet again:
   ```bash
   ./start_duckbucks.sh
   ```

### Important Notes About Backups:
- Your wallet backups are stored in `~/.duckbucks/backups/`
- Each backup is automatically created before any major changes
- Backups are named with timestamps for easy identification
- Keep your backup files safe - they contain your private keys!
- Never share your backup files with anyone

## Troubleshooting

If you experience issues:

1. Try resetting the wallet:
   ```bash
   ./start_duckbucks.sh reset
   ```
   This will reset the wallet settings while keeping your coins safe.

2. If problems persist, try restoring from a backup using the instructions above.

3. For additional help, visit: https://duckbucks.org/support

## System Requirements

- Linux operating system
- 2GB RAM minimum
- 1GB free disk space
- OpenGL 2.0 or later

## Security Notes

- Always keep your backup files in a secure location
- Never share your wallet.dat or backup files
- The wallet automatically creates backups before major changes
- Your private keys are stored in the wallet.dat file

## Support

For additional help and support:
- Visit: https://duckbucks.org/support
- Email: support@duckbucks.org
- Discord: https://discord.gg/duckbucks 