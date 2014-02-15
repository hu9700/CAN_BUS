#include <stdlib.h>//atoi
#include <linux/limits.h>//for PATH_MAX
#include <unistd.h>//access()
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>//stat()
#include <dirent.h>//opendir()
#include <sys/types.h>

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
		node_num = atoi(argv[2]);
	}
	else
	{
		node_num = -1;
	}
}

int check_file(int node_num)
{//check node file, check bus file
	char process_image_dir[PATH_MAX];
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
			char bus_folder[PATH_MAX];
			char nodes_folder[PATH_MAX];
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
					rval = mkdir(bus_dir, 0777);
					printf("mkdir %s", bus_dir);
				}
				else
				{
					rval = -1;
				}
			}
			else
			{
				closedir(bus_dir);
			}
			
			if(nodes_dir == NULL)
			{
				perror("check nodes_dir");
				if(errno == ENOENT)
				{
					rval = mkdir(nodes_dir, 0777);
					printf("mkdir %s", nodes_dir);
				}
				else
				{
					rval = -1;
				}
			}
			else
			{
				closedir(nodes_dir);
			}
			
			if(rval == 0)		
			{//check nodes and bus files
				
			}
		}
	}
}

int main(int argc, char **argv)
{
	int node_num = 0;
	
	node_num = arg_handler(argc, argv);
	if(node_num < 1)
	{
		exit(-1);
	}
	
	umask(0);
}
