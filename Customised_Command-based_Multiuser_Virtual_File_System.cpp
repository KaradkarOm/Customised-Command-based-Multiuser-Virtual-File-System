#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<iostream>

#define MAXINODE 50
#define READ 1
#define WRITE 2
#define MAXFILESIZE 2048
#define REGULAR 1
#define SPECIAL 2
#define START 0
#define CURRENT 1
#define END 2

typedef struct superblock
{
    int TotalInodes;
    int FreeInode;
}SUPERBLOCK, *PSUPERBLOCK;

typedef struct inode
{
    char FileName[50];
    int InodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;
    char *Buffer;
    int LinkCount;
    int ReferenceCount;
    int permission;
    char owner[50];        // Username of the owner

    struct user_permission {
        char username[30];
        int access_rights;  // 1=Viewer, 2=Editor
    } shared_users[10];  // Up to 10 users can be shared with

    int shared_count;  // Number of users the file is shared with

    struct inode *next;

}INODE, *PINODE, **PPINODE;

// Directory inode structure
typedef struct dir_inode
{
    char DirName[50];
    struct dir_inode *child;     // Pointer to the first child directory
    struct dir_inode *sibling;   // Pointer to the next sibling directory
    PINODE file;                 // Pointer to file associated with the directory
} DIR_INODE, *PDIR_INODE;

PDIR_INODE currentDir = NULL; 

typedef struct filetable
{
    int readoffset;
    int writeoffset;
    int count;
    int mode;
    PINODE ptrinode;
}FILETABLE, *PFILETABLE;

typedef struct ufdt
{
    PFILETABLE ptrfiletable;
}UFDT;

UFDT UFDTArr[50];
SUPERBLOCK SUPERBLOCKobj;
PINODE head = NULL;
PDIR_INODE rootDir = NULL; // Root directory pointer

//////////////////////////////////////
#define MAX_USERS 10
#define ADMIN 1
#define NORMAL_USER 0

typedef struct user
{
    char username[50];
    char password[50];
    int role;  // 1 for admin, 0 for normal user
} USER;

USER users[MAX_USERS];    // Store information about all users
int user_count = 0;       // Track the number of users
int current_user = -1;    // -1 means no user is logged in


void man(char* name)
{
    if(name == NULL) return;

    if(strcmp(name, "create") == 0)
    {
        printf("Description : Used to create new regular file\n");
        printf("Usage : create File_name Permisssion\n");
    }
    else if(strcmp(name, "read") == 0)
    {
        printf("Description : Used to read from a regular file\n");
        printf("Usage : read File_name No_of_bytes_to_read\n");
    }
    else if(strcmp(name, "write") == 0)
    {
        printf("Description : Used to write into the regular file\n");
        printf("Usage : write File_name, Click Enter, then enter the data that we want to write\n");
    }
    else if(strcmp(name, "ls") == 0)
    {
        printf("Description : Used to list all information of file\n");
        printf("Usage : ls\n");
    }
    else if(strcmp(name, "stat") == 0)
    {
        printf("Description : Used to display information of file\n");
        printf("Usage : stat File_name\n");
    }
    else if(strcmp(name, "fstat") == 0)
    {
        printf("Description : Used to display information of file\n");
        printf("Usage : fstat File_name\n");
    }
    else if(strcmp(name, "truncate") == 0)
    {
        printf("Description : Used to remove data from the regular file\n");
        printf("Usage : truncate File_name\n");
    }
    else if(strcmp(name, "open") == 0)
    {
        printf("Description : Used to open existing file\n");
        printf("Usage : open File_name mode\n");
    }
    else if(strcmp(name, "close") == 0)
    {
        printf("Description : Used to close opened file\n");
        printf("Usage : close File_name\n");
    }
    else if(strcmp(name, "closeall") == 0)
    {
        printf("Description : Used to close all opened file\n");
        printf("Usage : closeall\n");
    }
    else if(strcmp(name, "lseek") == 0)
    {
        printf("Description : Used to change file offset\n");
        printf("Usage : lseek File_name ChangeInOffset StartPoint\n");
    }
    else if(strcmp(name, "rm") == 0)
    {
        printf("Description : Used to delete a file\n");
        printf("Usage : rm File_name\n");
    }
    else
    {
        printf("Error : No manual entry available\n");
    }
}

void DisplayHelp()
{
    printf("ls :\n");
    printf("  Description : Lists all files present in the virtual file system.\n");
    printf("  Usage       : ls\n\n");

    printf("clear :\n");
    printf("  Description : Clears the console screen.\n");
    printf("  Usage       : clear\n\n");

    printf("open :\n");
    printf("  Description : Opens a file for reading or writing.\n");
    printf("  Usage       : open <File_name> <Mode>\n");
    printf("  Example     : open myfile.txt read\n\n");

    printf("close :\n");
    printf("  Description : Closes the specified file.\n");
    printf("  Usage       : close <File_descriptor>\n");
    printf("  Example     : close 1\n\n");

    printf("closeall :\n");
    printf("  Description : Closes all currently opened files.\n");
    printf("  Usage       : closeall\n\n");

    printf("read :\n");
    printf("  Description : Reads the contents from the specified file.\n");
    printf("  Usage       : read <File_descriptor> <Size>\n");
    printf("  Example     : read 1 100\n");
    printf("  (Reads 100 bytes from file descriptor 1)\n\n");

    printf("write :\n");
    printf("  Description : Writes data to the specified file.\n");
    printf("  Usage       : write <File_descriptor> <Data>\n");
    printf("  Example     : write 1 \"Hello, World!\"\n\n");

    printf("exit :\n");
    printf("  Description : Terminates the file system and closes the application.\n");
    printf("  Usage       : exit\n\n");

    printf("stat :\n");
    printf("  Description : Displays information about the specified file using its name.\n");
    printf("  Usage       : stat <File_name>\n");
    printf("  Example     : stat myfile.txt\n\n");

    printf("fstat :\n");
    printf("  Description : Displays information about a file using its file descriptor.\n");
    printf("  Usage       : fstat <File_descriptor>\n");
    printf("  Example     : fstat 1\n\n");

    printf("truncate :\n");
    printf("  Description : Removes all data from the specified file.\n");
    printf("  Usage       : truncate <File_name>\n");
    printf("  Example     : truncate myfile.txt\n\n");

    printf("rm :\n");
    printf("  Description : Deletes the specified file from the file system.\n");
    printf("  Usage       : rm <File_name>\n");
    printf("  Example     : rm myfile.txt\n\n");

    printf("man :\n");
    printf("  Description : Displays the manual/help for a given command.\n");
    printf("  Usage       : man <Command>\n");
    printf("  Example     : man ls\n\n");

    printf("lseek :\n");
    printf("  Description : Changes the offset (position) within an open file.\n");
    printf("  Usage       : lseek <File_descriptor> <Offset> <Position>\n");
    printf("  Example     : lseek 1 50 start\n\n");

    printf("shareFile :\n");
    printf("  Description : Shares a file with other users.\n");
    printf("  Usage       : shareFile <File_name> <User_name>\n\n");

    printf("listSharedUsers :\n");
    printf("  Description : Lists all users with whom a file is shared.\n");
    printf("  Usage       : listSharedUsers <File_name>\n\n");

    printf("listUsers :\n");
    printf("  Description : Lists all users registered in the virtual file system.\n");
    printf("  Usage       : listUsers\n\n");

    printf("login :\n");
    printf("  Description : Logs into the file system with a username and password.\n");
    printf("  Usage       : login <User_name> <Password>\n\n");

    printf("logout :\n");
    printf("  Description : Logs out the current user.\n");
    printf("  Usage       : logout\n\n");
}

int GetFDFromeName(char *name)
{
    int i = 0;

    while(i<50)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name) == 0)
            {
                break;
            }
        }
        i++;
    }
    if(i==50)
    {
        return -1;
    }
    else
    {
        return i;
    }
}

PINODE Get_Inode(char *name)
{
    PINODE temp = head;
    int i = 0;

    if(name == NULL)
        return NULL;

    while(temp != NULL)
    {
        if(strcmp(name, temp->FileName) == 0)
            break;
        temp = temp->next;
    }
    return temp;
}

void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head;

    while(i <= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));

        newn->LinkCount = 0;
        newn->ReferenceCount = 0;
        newn->FileType = 0;
        newn->FileSize = 0;

        newn->Buffer = NULL;
        newn->next = NULL;

        newn->InodeNumber = i;

        if(temp == NULL)
        {
            head = newn;
            temp = head;
        }
        else
        {
            temp->next = newn;
            temp = temp->next;
        }
        i++;
    }
    printf("DILB created successfully\n");
}

void InitialiseSuperBlock()
{
    int i = 0;
    while(i<=MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL;
        i++;
    }

    SUPERBLOCKobj.TotalInodes = MAXINODE;
    SUPERBLOCKobj.FreeInode = MAXINODE;
}

int CreateFile(char *name, int permission)
{
    int i = 0;
    PINODE temp = head;

    // Check for invalid name or permissions
    if ((name == NULL) || (permission == 0) || (permission > 3))
        return -1;  // Invalid parameters

    // Check if a user is logged in
    if (current_user == -1)  
    {
        printf("Error: No user is logged in.\n");
        return -4;  // No user logged in
    }

    // Check if there are free inodes available
    if (SUPERBLOCKobj.FreeInode == 0)
        return -2;  // No free inodes available

    // Decrease free inode count
    (SUPERBLOCKobj.FreeInode)--;

    // Check if the file already exists
    if (Get_Inode(name) != NULL)
        return -3;  // File already exists

    // Find an available inode
    while (temp != NULL)
    {
        if (temp->FileType == 0)  // Empty inode slot
            break;
        temp = temp->next;
    }

    // Check if we found a valid inode
    if (temp == NULL)
    {
        (SUPERBLOCKobj.FreeInode)++; // Restore the free inode count
        return -5; // No available inode
    }

    // Find an available slot in UFDT
    while (i < 50)
    {
        if (UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    // Allocate memory for file table
    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if (UFDTArr[i].ptrfiletable == NULL)
    {
        (SUPERBLOCKobj.FreeInode)++;  // Restore the free inode count
        return -4;  // Memory allocation failure
    }

    // Initialize file table
    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = permission;
    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;

    // Link to the inode
    UFDTArr[i].ptrfiletable->ptrinode = temp;

    // Set file attributes
    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName, name);
    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFDTArr[i].ptrfiletable->ptrinode->permission = permission;

    // Allocate memory for the file data buffer
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);
    if (UFDTArr[i].ptrfiletable->ptrinode->Buffer == NULL)
    {
        (SUPERBLOCKobj.FreeInode)++;
        free(UFDTArr[i].ptrfiletable);
        return -4;  // Memory allocation failure
    }
    
    // Initialize buffer
    memset(UFDTArr[i].ptrfiletable->ptrinode->Buffer, 0, MAXFILESIZE);

    // Associate the file with the current user (owner)
    printf("Current user index: %d, Username: %s\n", current_user, users[current_user].username);
    strcpy(UFDTArr[i].ptrfiletable->ptrinode->owner, users[current_user].username);

    printf("File '%s' created successfully by user '%s'.\n", name, users[current_user].username);
    
    return i;  // Return the file descriptor
}

int rm_File(char *name)
{
    int fd = 0;

    fd = GetFDFromeName(name);
    if(fd == -1)
        return -1;

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount) --;

    if((UFDTArr[fd].ptrfiletable->ptrinode->LinkCount) == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        free(UFDTArr[fd].ptrfiletable);
    }

    UFDTArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInode)++;

    return 0;
}

int ReadFile(int fd, char *arr, int iSize)
{
    int read_size = 0;

    if (UFDTArr[fd].ptrfiletable == NULL)
        return -1;

    // Ensure that a user is logged in
    if (current_user == -1)
    {
        printf("Error: No user is logged in.\n");
        return -5;  // Custom error for no logged-in user
    }

    // Check if the current user is either the owner or has read permissions
    if (strcmp(UFDTArr[fd].ptrfiletable->ptrinode->owner, users[current_user].username) != 0 &&
        (UFDTArr[fd].ptrfiletable->ptrinode->permission & READ) == 0)
    {
        printf("Error: User '%s' does not have read permission for file '%s'.\n", users[current_user].username, UFDTArr[fd].ptrfiletable->ptrinode->FileName);
        return -6;  // Custom error for permission denial
    }

    // Check if the file is opened in read or read+write mode
    if (UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ + WRITE)
        return -2;

    // Validate read permission
    if (UFDTArr[fd].ptrfiletable->ptrinode->permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->permission != READ + WRITE)
        return -3;

    // Ensure the file type is regular
    if (UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
        return -4;

    // Calculate the amount of data to read
    read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);

    if (read_size < iSize)
    {
        strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset), read_size);
        UFDTArr[fd].ptrfiletable->readoffset += read_size;
    }
    else
    {
        strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset), iSize);
        UFDTArr[fd].ptrfiletable->readoffset += iSize;
    }

    return iSize;
}

int WriteFile(int fd, char *arr, int iSize) {
    // Ensure that a user is logged in
    if (current_user == -1) {
        printf("Error: No user is logged in.\n");
        return -5;  // Custom error for no logged-in user
    }

    // Check if the file descriptor is valid
    if (fd < 0 || fd >= 50 || UFDTArr[fd].ptrfiletable == NULL) {
        printf("Error: Invalid file descriptor.\n");
        return -1;  // Custom error for invalid file descriptor
    }

    // Debugging statements
    printf("Current user : %s\n", users[current_user].username);
    printf("Owner user : %s\n", UFDTArr[fd].ptrfiletable->ptrinode->owner);

    // Check if the current user is the owner of the file
    if (strcmp(UFDTArr[fd].ptrfiletable->ptrinode->owner, users[current_user].username) != 0) {
        // Check if the current user is in the shared_users list with Editor rights
        int is_editor = 0;
        for (int i = 0; i < UFDTArr[fd].ptrfiletable->ptrinode->shared_count; i++) {
            if (strcmp(UFDTArr[fd].ptrfiletable->ptrinode->shared_users[i].username, users[current_user].username) == 0 &&
                UFDTArr[fd].ptrfiletable->ptrinode->shared_users[i].access_rights == 2) {  // 2 is for Editor
                is_editor = 1;
                break;
            }
        }
        
        if (!is_editor) {
            printf("Error: User '%s' does not have write permission for file '%s'.\n", 
                   users[current_user].username, 
                   UFDTArr[fd].ptrfiletable->ptrinode->FileName);
            return -6;  // Custom error for permission denial
        }
    }

    // Validate write mode
    if (UFDTArr[fd].ptrfiletable->mode != WRITE && 
        UFDTArr[fd].ptrfiletable->mode != (READ + WRITE)) {
        printf("Error: File '%s' is not opened in write mode.\n", 
               UFDTArr[fd].ptrfiletable->ptrinode->FileName);
        return -1;  // Custom error for wrong mode
    }

    // Ensure write offset is valid
    if (UFDTArr[fd].ptrfiletable->writeoffset >= MAXFILESIZE) {
        printf("Error: Write offset has reached the maximum file size for file '%s'.\n", 
               UFDTArr[fd].ptrfiletable->ptrinode->FileName);
        return -2;  // Custom error for reaching max file size
    }

    // Ensure the file type is regular
    if (UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR) {
        printf("Error: File '%s' is not a regular file.\n", 
               UFDTArr[fd].ptrfiletable->ptrinode->FileName);
        return -3;  // Custom error for file type
    }

    // Check remaining space in the buffer
    int remaining_space = MAXFILESIZE - UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize;
    if (iSize > remaining_space) {
        printf("Error: Not enough space to write %d bytes to file '%s'. Only %d bytes available.\n", 
               iSize, 
               UFDTArr[fd].ptrfiletable->ptrinode->FileName, 
               remaining_space);
        return -4;  // Custom error for insufficient space
    }

    // Write data into the file buffer
    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset), arr, iSize);

    // Update the write offset and the file's actual size
    UFDTArr[fd].ptrfiletable->writeoffset += iSize;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize += iSize;

    printf("Written %d bytes to file '%s' successfully.\n", iSize, UFDTArr[fd].ptrfiletable->ptrinode->FileName);
    
    return iSize;  // Return number of bytes written
}

int OpenFile(char *name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if(name == NULL || mode <= 0)
        return -1;

    temp = Get_Inode(name);
    if(temp == NULL)
        return -2;

    if(temp->permission < mode)
        return -3;

    while(i<50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if(UFDTArr[i].ptrfiletable == NULL) 
        return -1;
    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = mode;
    if(mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    else if(mode == READ)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
    }
    else if(mode  == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    UFDTArr[i].ptrfiletable->ptrinode = temp;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

    return i;
}

void CloseFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    (UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
}

int CloseFileByName(char *name)
{
    int i = 0;
    i = GetFDFromeName(name);
    if(i == -1)
        return -1;

    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;

    return 0;
}

void CloseAllFile()
{
    int i = 0;
    while(i<50)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable->readoffset = 0;
            UFDTArr[i].ptrfiletable->writeoffset = 0;
            (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
            break;
        }
        i++;
    }
}
 
int LseekFile(int fd, int size, int from)
{
    if(fd<0 || (from > 2)) 
        return -1;

    if(UFDTArr[fd].ptrfiletable == NULL) 
        return -1;

    if((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr[fd].ptrfiletable->mode == READ + WRITE))
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->readoffset) + size) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
                return -1;
            if(((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0) 
                return -1;
            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + size;
        }
        else if(from == START)
        {
            if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                return -1;
            if(size < 0)
                return -1;
            (UFDTArr[fd].ptrfiletable->readoffset) = size;
        }
        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
                return -1;
            if(((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0) 
                return -1;
            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + size;
        }
    }
    else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE)
                return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
                return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
        }
        else if(from == START)
        {
            if(size > MAXFILESIZE)
                return -1;
            if(size < 0)
                return -1;
            if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = size;
            (UFDTArr[fd].ptrfiletable->writeoffset) = size;
        }
        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
                return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
                return -1;
            (UFDTArr[fd].ptrfiletable->writeoffset) = ((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)) + size;
        }
    }
    return 0;
}

void ListFilesAndDirectories() {
    int i = 0;
    PINODE temp = head;

    if (SUPERBLOCKobj.FreeInode == MAXINODE) {
        printf("Error: There are no files or directories.\n");
        return;
    }

    printf("\nName\t\tType\t\tInode Number\tSize\tLink Count\n");
    printf("-----------------------------------------------------------\n");

    // Iterate through the linked list of inodes to display files and directories
    while (temp != NULL) {
        if (temp->FileType == REGULAR) {
            printf("%s\t\tFile\t\t%d\t\t%d\t%d\n", temp->FileName, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
        } else if (temp->FileType == SPECIAL) { // Assuming SPECIAL is used for directories
            printf("%s\t\tDirectory\t%d\t\t%d\t%d\n", temp->FileName, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
        }
        temp = temp->next;
    }
    printf("-----------------------------------------------------------\n");
}

int fstat_file(int fd)
{
    PINODE temp = head;
    int i = 0;

    if(fd < 0)
        return -1;

    if(UFDTArr[fd].ptrfiletable == NULL)
        return -2;

    temp = UFDTArr[fd].ptrfiletable->ptrinode;

    printf("\n-----------Statistical information about file-----------\n");
    printf("File name : %s\n",temp->FileName);
    printf("Inode Number : %d\n",temp->InodeNumber);
    printf("File size : %d\n",temp->FileSize);
    printf("Actual File size : %d",temp->FileActualSize);
    printf("Link count : %d\n",temp->LinkCount);
    printf("Reference count : %d\n",temp->ReferenceCount);

    if(temp->permission == 1)
        printf("File Permission : Read only\n");
    else if(temp->permission == 2)
        printf("File Permission : Write only\n");
    else if(temp->permission == 3)
        printf("File Permission : Read and Write\n");

    printf("-----------------------------------------------------------\n");

    return 0;
}

int stat_file(char *name)
{
    PINODE temp = head;
    int i = 0;

    if (name == NULL)
        return -1;

    // Find the file inode by name
    while (temp != NULL)
    {
        if (strcmp(name, temp->FileName) == 0)
            break;
        temp = temp->next;
    }

    if (temp == NULL)
        return -2;

    // Ensure that a user is logged in
    if (current_user == -1)
    {
        printf("Error: No user is logged in.\n");
        return -5;  // Custom error for no logged-in user
    }

    // Check if the current user is the owner or has read permission
    if (strcmp(temp->owner, users[current_user].username) != 0 && (temp->permission & READ) == 0)
    {
        printf("Error: User '%s' does not have permission to view the stats of file '%s'.\n", users[current_user].username, temp->FileName);
        return -6;  // Custom error for permission denial
    }

    // Display file statistics
    printf("\n-----------Statistical Information about File-----------\n");
    printf("File Name : %s\n", temp->FileName);
    printf("Owner : %s\n", temp->owner);  // Display file owner
    printf("Inode Number : %d\n", temp->InodeNumber);
    printf("File Size : %d\n", temp->FileSize);
    printf("Actual File Size : %d\n", temp->FileActualSize);
    printf("Link Count : %d\n", temp->LinkCount);
    printf("Reference Count : %d\n", temp->ReferenceCount);

    // Display file permissions
    if (temp->permission == 1)
        printf("File Permission : Read only\n");
    else if (temp->permission == 2)
        printf("File Permission : Write only\n");
    else if (temp->permission == 3)
        printf("File Permission : Read and Write\n");

    // Display shared users
    if (temp->shared_count > 0) {
        printf("Shared Users : ");
        for (int i = 0; i < temp->shared_count; i++) {
            printf("%s(%s)", temp->shared_users[i].username, 
                   temp->shared_users[i].access_rights == 1 ? "Viewer" : "Editor");
            if (i < temp->shared_count - 1)
                printf(", ");
        }
        printf("\n");
    }

    printf("--------------------------------------------------------\n");

    return 0;
}

int truncate_File(char *name)
{
    int fd = GetFDFromeName(name);
    if(fd == -1)
        return -1;

    memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,1024);
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;

    return 0;
}

void initialize_users() {
    // Initialize the users
    strcpy(users[0].username, "Admin");
    strcpy(users[0].password, "Admin");
    users[0].role = ADMIN; // Assign role as admin
    strcpy(users[1].username, "Om");
    strcpy(users[1].password, "Om");
    users[1].role = NORMAL_USER; // Assign role as normal user

    // Set user_count to the number of initialized users
    user_count = 2; // Since we have added two users
}

int authenticate_user(char *username, char *password) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(users[i].username, username) == 0 && 
            strcmp(users[i].password, password) == 0) {
            return i;  // Return user index on successful authentication
        }
    }
    return -1;  // Return -1 if authentication fails
}

void login() {
    char username[50], password[50];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    int user_index = authenticate_user(username, password);
    if (user_index != -1) {
        current_user = user_index;  // Set the current user index
        printf("Login successful. Current user: %s\n", users[current_user].username);
    } else {
        printf("Invalid username or password.\n");
    }
}

void logout()
{
    current_user = -1;  // Reset current user to indicate no user is logged in
    printf("User logged out successfully.\n");
}

void listSharedUsers(char *filename) {
    PINODE current = head;
    while (current != NULL) {
        if (strcmp(current->FileName, filename) == 0) {
            printf("Shared users for file '%s':\n", filename);
            for (int i = 0; i < current->shared_count; i++) {
                printf("%s (Access Rights: %s)\n", 
                       current->shared_users[i].username, 
                       (current->shared_users[i].access_rights == 1) ? "Viewer" : "Editor");
            }
            return;
        }
        current = current->next;
    }
    printf("ERROR: File '%s' does not exist or has no shared users.\n", filename);
}

void shareFile(char *filename, char *username, int access_rights) 
{
    PINODE current = head;  // Use a pointer to traverse the linked list
    int file_found = 0;     // Flag to indicate if the file was found
    int user_index = -1;

    // Find the file in the linked list
    while (current != NULL) {
        if (strcmp(current->FileName, filename) == 0) {
            file_found = 1; // Indicate that the file exists
            break;
        }
        current = current->next; // Traverse the linked list of inodes
    }

    // Error handling for file existence
    if (!file_found) {
        printf("ERROR: File '%s' does not exist........\n", filename);
        return;
    }

    // Find the user index
    for (int j = 0; j < user_count; j++) {
        if (strcmp(users[j].username, username) == 0) {
            user_index = j;
            break;
        }
    }

    // Error handling for user existence
    if (user_index == -1) {
        printf("ERROR: User '%s' does not exist.........\n", username);
        return;
    }

    // Check if the current user is the owner of the file
    if (strcmp(current->owner, users[current_user].username) != 0) {
        printf("ERROR: You do not have permission to share this file.\n");
        return;
    }

    // Check if the file is already shared with this user
    for (int k = 0; k < current->shared_count; k++) {
        if (strcmp(current->shared_users[k].username, username) == 0) {
            printf("ERROR: File '%s' is already shared with user '%s'.\n", filename, username);
            return;
        }
    }

    // Share the file with the user
    if (current->shared_count < 10/*MAX_SHARED_USERS*/) { // Ensure the limit is not exceeded
        strcpy(current->shared_users[current->shared_count].username, username);
        current->shared_users[current->shared_count].access_rights = access_rights;
        current->shared_count++;
        printf("File '%s' has been successfully shared with '%s' as a %s.\n", 
               filename, username, (access_rights == 1) ? "Viewer" : "Editor");
    } else {
        printf("ERROR: Cannot share file '%s'. Maximum number of shared users reached.\n", filename);
    }
}

void listUsers() {
    if (user_count == 0) {
        printf("No users found.\n");
        return;
    }

    printf("List of users:\n");
    for (int i = 0; i < user_count; i++) {
        printf("%s\n", users[i].username);
    }
}

PINODE GetInodeByName(char *name) {
    PINODE temp = head;  // Start from the head of the inode list

    while (temp != NULL) {
        if (strcmp(temp->FileName, name) == 0) {
            return temp;  // Return the inode if the name matches
        }
        temp = temp->next;  // Move to the next inode in the list
    }

    return NULL;  // Return NULL if no matching inode is found
}
// Initialize root directory
PDIR_INODE CreateRootDirectory() {
    PDIR_INODE root = (PDIR_INODE)malloc(sizeof(DIR_INODE));
    if (root == NULL) {
        return NULL; // Memory allocation failure
    }
    strcpy(root->DirName, "root");
    root->child = NULL; // No child directories
    root->sibling = NULL; // No sibling directories
    return root;
}

int CreateDirectory(char *name) {
    // Check for valid name
    if (name == NULL) {
        return -1; // Invalid name
    }

    // Check if the directory already exists
    if (Get_Inode(name) != NULL) {
        printf("ERROR: Directory '%s' already exists.\n", name);
        return -2; // Directory already exists
    }

    // Allocate memory for the new directory
    PDIR_INODE newDir = (PDIR_INODE)malloc(sizeof(DIR_INODE));
    if (newDir == NULL) {
        return -3; // Memory allocation failure
    }

    // Initialize the new directory
    strcpy(newDir->DirName, name);
    newDir->child = NULL;  // No children initially
    newDir->sibling = NULL; // No siblings initially
    newDir->file = NULL;    // No files initially

    // Link the new directory to the current directory or root
    if (currentDir == NULL) { // Creating in the root directory
        if (rootDir == NULL) {
            rootDir = (PDIR_INODE)malloc(sizeof(DIR_INODE)); // Initialize root if not set
            strcpy(rootDir->DirName, "root");
            rootDir->child = NULL;
            rootDir->sibling = NULL;
            rootDir->file = NULL;
        }
        newDir->sibling = rootDir->child; // Link to the first child
        rootDir->child = newDir;           // New directory becomes the first child
    } else {
        newDir->sibling = currentDir->child; // Link to the first child of currentDir
        currentDir->child = newDir;           // New directory becomes the first child of currentDir
    }

    printf("Directory '%s' created successfully.\n", name);
    return 0; // Success
}

int ChangeDirectory(char *name) {
    PINODE dirNode = GetInodeByName(name); // Implement GetInodeByName to find inode by name
    if (dirNode == NULL || dirNode->FileType != SPECIAL) {
        printf("ERROR: No such directory '%s'.\n", name);
        return -1; // Directory does not exist or is not a directory
    }

    currentDir = (PDIR_INODE)dirNode; // Change current directory
    printf("Changed directory to '%s'.\n", name);
    return 0; // Success
}

// Function to print the directory and file structure
// Function to print the directory and file structure
void PrintTree(PDIR_INODE dir, int level) {
    if (dir != NULL) {
        // Print the current directory name
        for (int i = 0; i < level; i++) {
            printf("    ");  // Indentation for child directories
        }
        printf("%s/\n", dir->DirName);  // Print directory name

        // Print all files in this directory
        PINODE file_temp = dir->file;
        while (file_temp != NULL) {
            for (int i = 0; i <= level; i++) {
                printf("    ");  // Indent for files
            }
            printf("%s\n", file_temp->FileName);  // Print file name
            file_temp = file_temp->next;  // Move to the next file in the linked list
        }

        // Recursively call PrintTree for all child directories
        PrintTree(dir->child, level + 1);  // Recursion for child directories
        PrintTree(dir->sibling, level);     // Recursion for sibling directories
    }
}

// Function to initiate tree printing from the root directory
void Tree() {
    if (rootDir == NULL) {
        printf("Error: No directories exist.\n");
        return;
    }
    printf("Directory structure:\n");
    PrintTree(rootDir->child, 0);  // Start printing from the root's child directories
}


int main()
{
    initialize_users();  // Initialize user data
    rootDir = CreateRootDirectory();
    currentDir = rootDir; // Set the current directory to root

    char *ptr = NULL;
    int ret = 0, fd = 0, count = 0;
    char command[4][80], str[80], arr[1024];

    // Enforce login check before username
    while (current_user == -1)
    {
        printf("Please log in to access the CVFS.\n");
        login(); // Call the login function
        
        // Only initialize after successful login
        if (current_user != -1) {
            printf("DILB created successfully\n");
            InitialiseSuperBlock();
            CreateDILB();
        }
    }

    InitialiseSuperBlock();
    CreateDILB();

    while (1)
    {
        strcpy(str, "");
        printf("\n%s@CVFS : >  ", users[current_user].username); // Show current user in the prompt
        fgets(str, 80, stdin);

        count = sscanf(str, "%s %s %s %s", command[0], command[1], command[2], command[3]);

        if (count == 1)
        {

            if (strcmp(command[0], "logout") == 0) 
            {
                logout();  // Call the logout function
                   
                while (current_user == -1) 
                {
                    login();
                }
            
                continue;  // Skip to the next iteration to prompt for commands again
            }

            if (strcmp(command[0], "ls") == 0)
            {
                ListFilesAndDirectories();
            }
            else if (strcmp(command[0], "closeall") == 0)
            {
                CloseAllFile();
                printf("All files closed successfully\n");
                continue;
            }
            else if (strcmp(command[0], "clear") == 0)
            {
                system("clear");
                continue;
            }
            else if (strcmp(command[0], "help") == 0)
            {
                DisplayHelp();
                continue;
            }
            else if (strcmp(command[0], "exit") == 0)
            {
                printf("Terminating Om's Virtual File System\n");
                break;
            }
            else if (strcmp(command[0], "listUsers") == 0) 
            {
                listUsers(); // Call the function to list all users
                continue;
            }
            else if (strcmp(command[0], "listSharedUsers") == 0) 
            {
                char filename[50];  // Declare a single string for the filename
                printf("Enter file name: ");
                scanf("%s", filename);  // Read the filename input
                listSharedUsers(filename); // Pass the filename to the function
                continue;
            }
            else if (strcmp(command[0], "tree") == 0) 
            {
                Tree();
            }
            else
            {
                printf("\nERROR : Command not found\n");
            }
        }
        else if (count == 2)
        {
            if (strcmp(command[0], "stat") == 0)
            {
                ret = stat_file(command[1]);
                if (ret == -1)
                    printf("ERROR : Incorrect parameters\n");
                else if (ret == -2)
                    printf("ERROR : There is no such file\n");
                continue;
            }
            else if (strcmp(command[0], "fstat") == 0)
            {
                ret = fstat_file(atoi(command[1]));
                if (ret == -1)
                    printf("ERROR : Incorrect parameters\n");
                else if (ret == -2)
                    printf("ERROR : There is no such file\n");
                continue;
            }
            else if (strcmp(command[0], "close") == 0)
            {
                ret = CloseFileByName(command[1]);
                if (ret == -1)
                    printf("ERROR : There is no such file\n");
                continue;
            }
            else if (strcmp(command[0], "rm") == 0)
            {
                ret = rm_File(command[1]);
                if (ret == -1)
                    printf("ERROR : There is no such file\n");
                continue;
            }
            else if (strcmp(command[0], "man") == 0)
            {
                man(command[1]);
            }
            else if (strcmp(command[0], "write") == 0)
            {
                fd = GetFDFromeName(command[1]);
                if (fd == -1)
                {
                    printf("Error : Incorrect parameter\n");
                    continue;
                }

                printf("Enter the data : \n");
                fgets(arr, sizeof(arr), stdin);  // Using fgets for safer input

                ret = strlen(arr);
                if (ret == 0)
                {
                    printf("Error : Incorrect parameter\n");
                    continue;
                }
                ret = WriteFile(fd, arr, ret);
                if (ret == -1)
                    printf("ERROR : Permission denied\n");
                if (ret == -2)
                    printf("ERROR : There is no sufficient memory\n");
                if (ret == -3)
                    printf("ERROR : It is not a regular file\n");
            }
            else if (strcmp(command[0], "truncate") == 0)
            {
                ret = truncate_File(command[1]);
                if (ret == -1)
                    printf("Error : Incorrect parameter\n");
            }
            else if (strcmp(command[0], "mkdir") == 0) // Create directory
            {
                ret = CreateDirectory(command[1]);
                if (ret == 0) {
                    printf("Directory '%s' created successfully.\n", command[1]);
                } else {
                    printf("ERROR: Failed to create directory '%s'.\n", command[1]);
                }
                continue;
            }
            else if (strcmp(command[0], "cd") == 0) // Change directory
            {
                ret = ChangeDirectory(command[1]);
                if (ret == 0) {
                    printf("Changed to directory '%s'.\n", command[1]);
                } else {
                    printf("ERROR: Directory '%s' does not exist.\n", command[1]);
                }
                continue;
            }
            else
            {
                printf("\nERROR : Command not found!!\n");
                continue;
            }
        }
        else if (count == 3)
        {
            if (strcmp(command[0], "create") == 0)
            {
                ret = CreateFile(command[1], atoi(command[2]));
                if (ret >= 0)
                    printf("File is successfully created with file descriptor : %d\n", ret);
                else if (ret == -1)
                    printf("ERROR : Incorrect parameters\n");
                else if (ret == -2)
                    printf("ERROR : There are no inodes available\n");
                else if (ret == -3)
                    printf("ERROR : File already exists\n");
                else if (ret == -4)
                    printf("ERROR : Memory allocation failure\n");
                continue;
            }
            else if (strcmp(command[0], "open") == 0)
            {
                ret = OpenFile(command[1], atoi(command[2]));
                if (ret >= 0)
                    printf("File is successfully opened with file descriptor : %d\n", ret);
                else if (ret == -1)
                    printf("ERROR : Incorrect parameters\n");
                else if (ret == -2)
                    printf("ERROR : File not present\n");
                else if (ret == -3)
                    printf("ERROR : Permission denied\n");
                continue;
            }
            else if (strcmp(command[0], "read") == 0)
            {
                fd = GetFDFromeName(command[1]);
                if (fd == -1)
                {
                    printf("Error : Incorrect parameter\n");
                    continue;
                }
                ptr = (char *)malloc(atoi(command[2]) + 1);

                if (ptr == NULL)
                {
                    printf("Error : Memory allocation failure\n");
                    continue;
                }
                ret = ReadFile(fd, ptr, atoi(command[2]));
                if (ret == -1)
                    printf("ERROR : File not existing\n");
                else if (ret == -2)
                    printf("ERROR : Permission denied\n");
                else if (ret == -3)
                    printf("ERROR : Reached the end of the file\n");
                else if (ret == -4)
                    printf("ERROR : It is not a regular file\n");
                else if (ret == 0)
                    printf("ERROR : File is empty\n");
                else if (ret > 0)
                {
                    write(2, ptr, ret);
                }
                free(ptr);  // Free the dynamically allocated memory
                continue;
            }
            else
            {
                printf("\nERROR : Command not found!!!\n");
            }
        }
        else if (count == 4)
        {
            if (strcmp(command[0], "lseek") == 0)
            {
                fd = GetFDFromeName(command[1]);
                if (fd == -1)
                {
                    printf("Error : Incorrect parameter\n");
                    continue;
                }
                ret = LseekFile(fd, atoi(command[2]), atoi(command[3]));
                if (ret == -1)
                {
                    printf("ERROR : Unable to perform lseek\n");
                }
            }
            else if (strcmp(command[0], "shareFile") == 0)
            {
                // Handle the shareFile command
                int access_rights = atoi(command[3]);

                if (access_rights != 1 && access_rights != 2) {
                    printf("Invalid access rights. Use 1 for Viewer or 2 for Editor.\n");
                    continue;
                }

                shareFile(command[1], command[2], access_rights);
            }
            else
            {
                printf("\nERROR : Command not found!!!\n");
                continue;
            }
        }
        else
        {
            printf("\nERROR : Command not found!!!\n");
            continue;
        }
    }

    return 0;
}


