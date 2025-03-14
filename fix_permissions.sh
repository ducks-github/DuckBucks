# Fix ownership and permissions
sudo chown -R $(whoami):$(whoami) "$HOME/.duckbucks"
find "$HOME/.duckbucks" -type d -exec chmod 700 {} \;
find "$HOME/.duckbucks" -type f -exec chmod 600 {} \;