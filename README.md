# Customized Command-Based Multiuser Virtual File System

This project implements a command-based, multi-user, virtual file system in C. The system allows multiple users to create, manage, and share files with different permissions.

## Project Overview
This virtual file system provides a robust environment for file management operations such as creating, reading, writing, deleting, and sharing files among multiple users with specific permissions.

## Main Objective
To provide a command-line interface for managing files in a virtual file system that supports multiple users and enforces access permissions.

## Key Features
- Create and manage regular and special files.
- Command-based interaction for file operations.
- Multi-user support with different roles (Owner, Editor, Viewer).
- File sharing with specific access rights (view or edit).
- Comprehensive manual and help commands for user guidance.
- Directory structure with child and sibling directories.
- User authentication and role-based access control.

## Target Audience
This project is ideal for:
- Developers and engineers looking to understand and implement virtual file systems.
- Students and educators exploring file management and access control systems.
- System administrators managing multi-user environments.

## How It Works
Users interact with the file system through a set of predefined commands. The system supports creating, reading, writing, deleting, and sharing files with different access rights. Users must log in to perform operations, and their actions are governed by their role (admin or normal user).

## Technologies Used
- **C Programming Language**: Core language for the implementation.
- **File I/O Operations**: For reading and writing file contents.
- **Data Structures**: Inode, directory inode, and user structures for managing files and users.
- **User Authentication**: Username and password-based login system.

## Commands Overview
- `ls`: Lists all files present in the virtual file system.
- `clear`: Clears the console screen.
- `open`: Opens a file for reading or writing.
- `close`: Closes the specified file.
- `closeall`: Closes all currently opened files.
- `read`: Reads the contents from the specified file.
- `write`: Writes data to the specified file.
- `exit`: Terminates the file system and closes the application.
- `stat`: Displays information about the specified file using its name.
- `fstat`: Displays information about a file using its file descriptor.
- `truncate`: Removes all data from the specified file.
- `rm`: Deletes the specified file from the file system.
- `man`: Displays the manual/help for a given command.
- `lseek`: Changes the offset (position) within an open file.
- `shareFile`: Shares a file with other users.
- `listSharedUsers`: Lists all users with whom a file is shared.
- `listUsers`: Lists all users registered in the virtual file system.
- `login`: Logs into the file system with a username and password.
- `logout`: Logs out the current user.


## Inspiration or Motivation
The project was inspired by the need for a customizable, command-line virtual file system that supports multi-user access with specific permissions, mimicking real-world file systems in a simplified environment.

## Future Plans/Development
- Implement additional file operations like file renaming and copying.
- Enhance security features with encryption for sensitive files.
- Develop a graphical user interface (GUI) for easier interaction.
- Add support for network-based file sharing.

## Credits & Contributors
Developed by **Om Karadkar**.

## Contact Information
- **Email**: om.22210454@viit.ac.in
