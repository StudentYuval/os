// this program will read xv6 fs.img and print out the contents of the file system (ls command)
// we get fs.img from the command line, and parse it to get the contents of the file system
// this program will run on a linux machine, and will not be compiled on the xv6 machine



#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// read the superblock from the file system image
void get_sb_info(FILE *fp, struct superblock *sb)
{

    // super block starts at the second block
    fseek(fp, BSIZE, SEEK_SET);
    fread(sb, sizeof(struct superblock), 1, fp);

}

void read_inodes(FILE *fp, struct superblock *sb, struct dinode *dinodes)
{
    //initialize the dinodes array
    for (int i = 0; i < sb->ninodes; i++)
    {
        dinodes[i].type = 0;
        dinodes[i].major = 0;
        dinodes[i].minor = 0;
        dinodes[i].nlink = 0;
        dinodes[i].size = 0;
        for (int j = 0; j < NDIRECT + 1; j++)
        {
            dinodes[i].addrs[j] = 0;
        }
    }
    fseek(fp, sb->inodestart * BSIZE, SEEK_SET); // seek to the start of the inode block
    fread(dinodes, sizeof(struct dinode), sb->ninodes, fp); // read the inode block into the dinodes array

}

// printing the contents of the file system
// if function is 0 - do ls, if function is 1 - do cp
void user_ls(FILE *fp, struct superblock *sb)
{
    struct dinode dinodes[sb->ninodes]; // set array of dinodes to the size of the number of inodes

    read_inodes(fp, sb, dinodes);
    
    // read all available direcotries, and print all file names in the system
    int root_inode = 1;
    struct dirent dir;
    int block_size = BSIZE / sizeof(struct dirent); // calculate the number of directory entries per block
    int block_count = dinodes[root_inode].size / BSIZE; // calculate the number of blocks in the directory
    // round up if the directory size is not a multiple of the block size
    if (dinodes[root_inode].size % BSIZE != 0)
        block_count++;
    int offset = 0; // initialize the offset
    //loop through the directory block by block
    for(int i = 0; i < block_count; i++)
    {
        // seek to the current block
        fseek(fp, dinodes[root_inode].addrs[i] * BSIZE, SEEK_SET); 
        // loop through the directory entries in the current block
        for (int j = 0; j < block_size; j++)
        {
            offset = i*block_size+j;
            fread(&dir, sizeof(struct dirent), 1, fp);
            if (dir.inum == 0)
                break;
            //print format: file name   type   inode number   size with fixed width for every column
            printf("%-15s", dir.name);
            printf("%-5d", dinodes[dir.inum].type);
            printf("%-5d", dir.inum);
            printf("%-5d", dinodes[dir.inum].size);
            printf("\n");
        }
    }
}

// copy a file from xv6 to linux
void user_cp(FILE *fp, struct superblock *sb, char *xv6_file_name, char *linux_file_name)
{
        struct dinode dinodes[sb->ninodes]; // set array of dinodes to the size of the number of inodes

    read_inodes(fp, sb, dinodes);
    
    // read all available direcotries, and print all file names in the system
    int root_inode = 1;
    struct dirent dir;
    int block_size = BSIZE / sizeof(struct dirent); // calculate the number of directory entries per block
    int block_count = dinodes[root_inode].size / BSIZE; // calculate the number of blocks in the directory
    // round up if the directory size is not a multiple of the block size
    if (dinodes[root_inode].size % BSIZE != 0)
        block_count++;
    int offset = 0; // initialize the offset
    int inode_num = -1; // initialize the inode number to -1
    //loop through the directory block by block
    for(int i = 0; i < block_count; i++)
    {
        // seek to the current block
        fseek(fp, dinodes[root_inode].addrs[i] * BSIZE, SEEK_SET); 
        // loop through the directory entries in the current block
        for (int j = 0; j < block_size; j++)
        {
            offset = i*block_size+j;
            fread(&dir, sizeof(struct dirent), 1, fp);
            if (dir.inum == 0)
                break;
            if(strcmp(dir.name, xv6_file_name) == 0)
            {
                if(dinodes[dir.inum].type == T_FILE)
                {
                    inode_num = dir.inum;
                    break;
                }
                else
                {
                    printf("error: %s is not a file\n", xv6_file_name); 
                    exit(1);
                }  
            }
        }
    }

    // if we found the file, copy it to linux
    if (inode_num != -1)
    {
        // open the file to write
        FILE *fp_linux = fopen(linux_file_name, "w");
        if (fp_linux == NULL)
        {
            fprintf(stderr, "error opening linux file\n");
            exit(1);
        }

        // copy the file
        int size = dinodes[inode_num].size; // get the size of the file
        int num_blocks = size / BSIZE; // get the number of blocks the file is using
        int is_last_block_partial = 0;
        if (size % BSIZE != 0) // round up if the file size is not a multiple of the block size
            is_last_block_partial=1;
        int offset = 0;
        for (int i = 0; i < num_blocks; i++)
        {
            if (i >= NDIRECT) // if we are on the indirect block, break
                break;
            char buf[BSIZE];
            fseek(fp, dinodes[inode_num].addrs[i] * BSIZE, SEEK_SET); // seek to the start of the block
            fread(buf, BSIZE, 1, fp); // read the block into the buffer
            fwrite(buf, BSIZE, 1, fp_linux); // write the block to the linux file
            offset += BSIZE; // update the offset
        }

        if(is_last_block_partial && num_blocks < NDIRECT)
        {
            char buf[BSIZE];
            fseek(fp, dinodes[inode_num].addrs[num_blocks] * BSIZE, SEEK_SET); // seek to the start of the block
            fread(buf, BSIZE, 1, fp); // read the block into the buffer
            fwrite(buf, size % BSIZE, 1, fp_linux); // write the block to the linux file
        }

        // on block number NDIRECT+1 we have the indirect block
        // we need to read the indirect block and copy the blocks it points to
        if (num_blocks > NDIRECT)
        {
            int indirect_block[BSIZE / sizeof(int)];
            fseek(fp, dinodes[inode_num].addrs[NDIRECT] * BSIZE, SEEK_SET); // seek to the start of the indirect block
            fread(indirect_block, BSIZE, 1, fp); // read the indirect block into the buffer

            // copy the blocks the indirect block points to
            for (int i = 0; i < (size - offset) / BSIZE; i++)
            {
                char buf[BSIZE];
                fseek(fp, indirect_block[i] * BSIZE, SEEK_SET); // seek to the start of the block
                fread(buf, BSIZE, 1, fp); // read the block into the buffer
                fwrite(buf, BSIZE, 1, fp_linux); // write the block to the linux file
            }

            // copy the last block if the file size is not a multiple of BSIZE
            if ((size - offset) % BSIZE != 0)
            {
                char buf[BSIZE];
                fseek(fp, indirect_block[(size - offset) / BSIZE] * BSIZE, SEEK_SET); // seek to the start of the block
                fread(buf, BSIZE, 1, fp); // read the block into the buffer
                fwrite(buf, (size - offset) % BSIZE, 1, fp_linux); // write the block to the linux file
            }
        }
        
        fclose(fp_linux);
        printf("The File \"%s\" copied successfuly to local system as \"%s\"\n", xv6_file_name, linux_file_name);
    }
    else
    {
        fprintf(stderr, "File %s does not exist in the root directory", xv6_file_name);
        exit(1);
    }
}


int main(int argc, char *argv[])
{

    // read the file system image
    if (argc < 3) {
        fprintf(stderr, "too few arguments\n");
        exit(1);
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "image not found\n");
        exit(1);
    }

    // read the superblock
    struct superblock sb;
    //initialize the superblock struct
    sb= (struct superblock) {0};

    get_sb_info(fp, &sb);

    // check which command we need to run
    if (!strcmp(argv[2], "ls")) 
    {
        // run ls command
        user_ls(fp, &sb);
    }

    else if (!strcmp(argv[2], "cp"))
    {
        // make sure we have xv6 file name and linux file name
        if (argc != 5)
        {
            fprintf(stderr, "Usage: fsinfo <file_system_image> cp <xv6_file_name> <linux_file_name>\n");
            exit(1);
        }
        else
        {
            // run cp command
            user_cp(fp, &sb, argv[3], argv[4]);
        }
    }
    else
    {
        fprintf(stderr, "invalid command\n");
        exit(1);
    }

    fclose(fp);    
    return 0;
}