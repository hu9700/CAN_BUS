#include <stdlib.h>//atoi
#include <linux/limits.h>//for PATH_MAX
#include <unistd.h>//access()
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>//stat()
#include <dirent.h>//opendir()
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>//mknod
#include "../include/can_bus.h"

static char bus_path[PATH_MAX];
static char nodes_path[PATH_MAX];

char * get_image_path(pid_t pid, char *proc_dir_buff, size_t size)//sizeof proc_dir_buff should be PATH_MAX
{
	char *process_dir = NULL;
	int count = 0;
	
	memset(proc_dir_buff, 0, size);
	snprintf(proc_dir_buff, size, "/proc/%d/exe", pid);
	
	if(access(proc_dir_buff, R_OK) != 0)
	{//can not access to /proc file
		perror("access /proc/");
		memset(proc_dir_buff, 0, size);
	}
	else
	{
		ssize_t res;
		process_dir = malloc(size);
		memset(process_dir, 0, size);
		res = readlink(proc_dir_buff, process_dir, size);
		if(res <= 0)
		{//error
			perror("readlink");
			free(process_dir);
			process_dir = NULL;
			memset(proc_dir_buff, 0, size);
		}
		else if(res > size)
		{
			printf("directory length is too long");
			free(process_dir);
			process_dir = NULL;
			memset(proc_dir_buff, 0, size);
		}
		else
		{
			memset(proc_dir_buff, 0, size);
			memcpy(proc_dir_buff, process_dir, size);
			free(process_dir);
			process_dir = proc_dir_buff;
		}
	}
	return process_dir;
}

int arg_handler(int argc, char **argv)
{
	int node_num = 0;
		
	if(argc >= 2)
	{
		node_num = atoi(argv[1]);
	}
	else
	{
		node_num = -1;
	}
}

int check_file(int node_num)
{//check node file, check bus file
	char process_image_dir[PATH_MAX];
	char bus_folder[PATH_MAX];
	char nodes_folder[PATH_MAX];	
	char *process_image_folder = NULL;
	void *res;
	int rval = 0;
	
	res = get_image_path(getpid(), process_image_dir, sizeof(process_image_dir));
	if(res == NULL)
	{
		rval = -1;
	}
	else
	{//get the directory part of image path
		char *point_slush = process_image_dir;
		char *point_file = process_image_dir;
		
		while(point_slush != NULL)
		{
			point_slush = memchr(point_file, '/', strlen(point_file));
			if(point_slush != NULL)
			{
				point_file = point_slush + 1;
			}
		}
		
		memset(point_file, 0, sizeof(process_image_dir) - (point_file - process_image_dir) + 1);
	}
	
	process_image_folder = process_image_dir;
	
	if(rval == 0)
	{//check folder of emulator
		DIR *dir = NULL;
		dir = opendir(process_image_folder);
		if(dir == NULL)
		{
			perror(process_image_folder);
			rval = -1;
		}
		else
		{
			printf("ok for %s\n", process_image_folder);
			closedir(dir);
		}
		
		if(rval == 0)
		{//check bus folder and nodes folder exist
			DIR *bus_dir = NULL;
			DIR *nodes_dir = NULL;
			
			memcpy(bus_folder, process_image_folder, sizeof(bus_folder));
			memcpy(nodes_folder, process_image_folder, sizeof(nodes_folder));
			
			strncat(bus_folder, "bus/", sizeof(bus_folder));
			strncat(nodes_folder, "nodes/", sizeof(nodes_folder));
			
			bus_dir = opendir(bus_folder);
			nodes_dir = opendir(nodes_folder);
			
			if(bus_dir == NULL)
			{
				perror("check bus_dir");
				if(errno == ENOENT)
				{
					rval = mkdir(bus_folder, 0777);
					if(rval == 0)
					{
						printf("mkdir %s", bus_folder);
					}
					else
					{
						perror("make bus folder");
					}
				}
				else
				{
					rval = -1;
				}
			}
			else
			{
				printf("exist %s\n", bus_folder);
				closedir(bus_dir);
			}
			
			if(nodes_dir == NULL)
			{
				perror("check nodes_dir");
				if(errno == ENOENT)
				{
					rval = mkdir(nodes_folder, 0777);
					if(rval == 0)
					{
						printf("mkdir %s", nodes_folder);
					}
					else
					{
						perror("make nodes folder");
					}
				}
				else
				{
					rval = -1;
				}
			}
			else
			{
				printf("exist %s\n", nodes_folder);
				closedir(nodes_dir);
			}
			
			if(rval == 0)		
			{//check nodes and bus files
				char bus_file[PATH_MAX];
				DIR *bus_dir = NULL;
				memcpy(bus_file, bus_folder, sizeof(bus_file));
				strncat(bus_file, "bus", sizeof(bus_file));
				bus_dir = opendir(bus_file);
				if(bus_dir == NULL)
				{
					perror("bus file");
					if(errno == ENOENT)
					{
						rval = mknod(bus_file, S_IFREG, 0);
						if(rval == 0)
						{
							printf("make bus file\n");
							chmod(bus_file, 0666);
						}
						else
						{
							perror("make bus file");
						}
					}
					else
					{
						rval = -1;
					}
				}
				else
				{
					printf("exist %s\n", bus_file);
					closedir(bus_dir);
				}
				
				if(rval == 0)
				{//check node file
					int count = 0;
					char nodes_file[PATH_MAX];
					
					for(count = 0; count < node_num; count ++)
					{
						DIR *nodes_dir = NULL;
						memset(nodes_file, 0, sizeof(nodes_file));
						snprintf(nodes_file, sizeof(nodes_file), "%snode_%d", nodes_folder, count);
						nodes_dir = opendir(nodes_file);
						if(nodes_dir == NULL)
						{
							perror("check nodes file");
							if(errno == ENOENT)
							{
								rval = mknod(nodes_file, S_IFIFO, 0);
								if(rval == 0)
								{
									printf("make nodes_%d file\n", count);
									chmod(nodes_file, 0666);
								}							
								else
								{
									perror("make nodes");
								}
							}
							else
							{
								rval = -1;
							}						
						}
						else
						{
							printf("exist %s\n", nodes_file);
							closedir(nodes_dir);
						}
					}//loop to check nodes file
				}
			}
		}
	}
	
	if(rval == 0)
	{
		memset(bus_path, 0, sizeof(bus_path));
		memset(nodes_path, 0, sizeof(nodes_path));
		snprintf(bus_path, sizeof(bus_path), "%sbus", bus_folder);
		snprintf(nodes_path, sizeof(nodes_path), "%snode_", nodes_folder);
	}
	
	return rval;
}

//int main(int argc, char **argv)
int start_can_emu(int node_num)
{
	//int node_num = 0;
	int rval = 0;
	
	//node_num = arg_handler(argc, argv);
	if(node_num < 1)
	{
		exit(-1);
	}
	
	umask(0);	
	rval = check_file(node_num);//create bus file and node file
	
	if(rval != 0)
	{//fail
		printf("\nfilesystem create FAIL!!!\n");
	}
	else
	{//success
		printf("\nfilesystem create SUCCESS!!!\n");
		
		//loop to wait and receive data from fifo
		{
			int fifo_fdread_list[node_num];
			int fifo_fdwrite_list[node_num];
			int bus_fd;
			int count;
			char node_dir[PATH_MAX];
			
			memset(fifo_list, 0, sizeof(int) * node_num);
			
			for(count = 0; count < node_num; count ++)
			{
				memcpy(node_dir, nodes_path, sizeof(node_dir));
				snprintf(node_dir, sizeof(node_dir), "%snode_%d", node_dir, count);
				fifo_fdread_list[count] = open(node_dir, O_RDONLY);
				fifo_fdwrite_list[count] = open(node_dir, O_WRONLY);
			}
		}
	}//file system created
	
	return rval;
}
