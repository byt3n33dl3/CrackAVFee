System Information

    uname -a - Displays all system information.
    top - Displays an ongoing view of system processes.
    htop (requires installation) - An interactive process viewer, considered an enhanced version of top.
    df -h - Shows disk usage in human-readable format.
    free -m - Displays free and used memory in the system in megabytes.

File and Directory Operations

    ls - Lists files and directories in the current directory.
    cd directory_name - Changes the current directory to directory_name.
    pwd - Prints the current directory path.
    mkdir directory_name - Creates a new directory.
    rm file_name - Deletes a file. Use -r for directories (rm -r directory_name).
    cp source destination - Copies files or directories.
    mv source destination - Moves or renames files or directories.

File Viewing and Manipulation

    cat file_name - Displays the content of a file.
    less file_name - Views the content of a file one page at a time.
    grep 'search_pattern' file_name - Searches for a specific pattern in a file.
    head file_name - Displays the first 10 lines of a file (customize with -n option).
    tail file_name - Displays the last 10 lines of a file (customize with -n option).

Network Operations

    ping host_or_IP - Checks connectivity to a host.
    ifconfig (or ip a on newer systems) - Displays network interfaces and IP addresses.
    netstat -tuln - Lists all active listening ports.
    ssh user@host - Connects to a host via SSH.
    scp source user@host:destination - Securely copies files between hosts over SSH.

Package Management (Debian-based systems)

    sudo apt update - Updates package lists.
    sudo apt upgrade - Upgrades all upgradable packages.
    sudo apt install package_name - Installs a package.
    sudo apt remove package_name - Removes a package.

Permissions and Ownership

    chmod permissions file_name - Changes the permissions of a file or directory.
    chown user:group file_name - Changes the owner and group of a file or directory.

Process Management

    ps aux - Displays running processes.
    kill PID - Kills a process by its PID (Process ID).
    killall process_name - Kills all processes with the name process_name.

Text Manipulation

    echo "text" - Displays a line of text.
    sed 's/find/replace/' file_name - Finds and replaces text in a file.
    awk '{print}' file_name - Processes and analyzes file content.