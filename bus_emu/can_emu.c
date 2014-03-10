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
#include "../include/can_emu.h"
#include "../include/poll_select.h"
#include <fcntl.h>

//static char bus_path[PATH_MAX];
static char nodes_path[PATH_MAX];//node_n_r is read by client, node_n_w is written by client

static char * get_image_path(pid_t pid, char *proc_dir_buff, size_t size)//sizeof proc_dir_buff should be PATH_MAX
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

static int arg_handler(int argc, char **argv)
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

static int check_file(int node_num)
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
		{//checknodes folder exist
			DIR *nodes_dir = NULL;
			
			memcpy(nodes_folder, process_image_folder, sizeof(nodes_folder));
			
			strncat(nodes_folder, "nodes/", sizeof(nodes_folder));
			
			nodes_dir = opendir(nodes_folder);
			
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
			{//check nodes files node_n_r and node_n_w
				//check node file
				int count = 0;
				char nodes_file_read[PATH_MAX];
				char nodes_file_write[PATH_MAX];
				
				for(count = 0; count < node_num; count ++)
				{
					DIR *nodes_dir = NULL;
					memset(nodes_file_read, 0, sizeof(nodes_file));
					memset(nodes_file_write, 0, sizeof(nodes_file));
					snprintf(nodes_file_read, sizeof(nodes_file), "%snode_%d_r", nodes_folder, count);
					snprintf(nodes_file_write, sizeof(nodes_file), "%snode_%d_w", nodes_folder, count);
					
					//try to open node_n_r
					nodes_dir = opendir(nodes_file_read);
					if(nodes_dir == NULL)
					{
						perror("check nodes file");
						if(errno == ENOENT)
						{
							rval = mknod(nodes_file_read, S_IFIFO, 0);
							if(rval == 0)
							{
								printf("make node_%d_r file\n", count);
								chmod(nodes_file_read, 0666);
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
						printf("exist %s\n", nodes_file_read);
						closedir(nodes_dir);
					}
					
					//try to open node_n_w
					nodes_dir = NULL;
					nodes_dir = opendir(nodes_file_write);
					if(nodes_dir == NULL)
					{
						perror("check nodes file");
						if(errno == ENOENT)
						{
							rval = mknod(nodes_file_write, S_IFIFO, 0);
							if(rval == 0)
							{
								printf("make node_%d_w file\n", count);
								chmod(nodes_file_write, 0666);
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
						printf("exist %s\n", nodes_file_write);
						closedir(nodes_dir);
					}					
				}//loop to check nodes file	
			}
		}
	}
	
	if(rval == 0)
	{
//		memset(bus_path, 0, sizeof(bus_path));
		memset(nodes_path, 0, sizeof(nodes_path));
//		snprintf(bus_path, sizeof(bus_path), "%sbus", bus_folder);
		snprintf(nodes_path, sizeof(nodes_path), "%s", nodes_folder);
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
			char node_read_dir[PATH_MAX];
			char node_write_dir[PATH_MAX];
			
			fd_set read_set;
			fd_set write_set;
			fd_set except_set;			
			
			memset(fifo_fdread_list, 0, sizeof(int) * node_num);
			memset(fifo_fdwrite_list, 0, sizeof(int) * node_num);
			clean_fdset(&read_set);
			clean_fdset(&write_set);
			clean_fdset(&except_set);
			
			for(count = 0; count < node_num; count ++)
			{//open all the node
				memcpy(node_read_dir, nodes_path, sizeof(node_read_dir));
				memcpy(node_write_dir, nodes_path, sizeof(node_write_dir));
				snprintf(node_read_dir, sizeof(node_read_dir), "%snode_%d_r", node_read_dir, count);
				snprintf(node_write_dir, sizeof(node_write_dir), "%snode_%d_w", node_write_dir, count);
				
				fifo_fdread_list[count] = open(node_write_dir, O_RDONLY);
				if(fifo_fdread_list[count] != 0)
				{
					fcntl(fifo_fdread_list[count], F_SETFL, O_NONBLOCK);
					addfd_fdset(&read_set, fifo_fdread_list[count]);
				}
				else
				{
					perror("fall to open node readonly");
					rval = -1;
					break;
				}
				
				fifo_fdwrite_list[count] = open(node_read_dir, O_WRONLY);				
				if(fifo_fdwrite_list[count] != 0)
				{
					fcntl(fifo_fdwrite_list[count], F_SETFL, O_NONBLOCK);
					//addfd_fdset(&write_set, fifo_fdwrite_list[count]);
				}
				else
				{
					perror("fall to open node writeonly");
					rval = -1;
					break;
				}
			}
			
			if(rval != 0)
			{//error happened to open node
				for(count = 0; count < node_num; count ++)
				{
					if(fifo_fdread_list[count] != 0)
					{
						close(fifo_fdread_list[count]);	
					}
					
					if(fifo_fdwrite_list[count] != 0)
					{
						close(fifo_fdwrite_list[count]);	
					}					
				}
			}
			else
			{//node is opened successfully
				int ready = 0;
				int avialable_list[node_num];
				int avialable_num = 0;
				
				while(1)
				{
					ready = select(fifo_fdread_list[node_num - 1] + 1, &read_set, /*&write_set*/NULL, NULL, NULL);
					avialable_num = 0;
					
					if(ready < 0)
					{
						perror("select error;");
						rval = ready;
						break;
					}
					else					
					{
						for(count = 0; count < node_num; count ++)
						{//get all the avialable node
							if(isfd_fdset(&read_set, fifo_fdread_list[count]))
							{
								avialable_list[avialable_num] = count;
								avialable_num ++;
							}
						}
						
						if(avialable_num <= 0)
						{
							printf("select problem\n");
							rval = -1;
						}
						else
						{//here we assume every frame is in a correct form
							struct can_frame_attr can_frame[avialable_num];
							int result = 0;
							
							for(count = 0; count < avialable_num; count ++)
							{
								result = read(fifo_fdread_list[count], can_frame + count, sizeof(struct can_frame_attr));
								
								if(result < sizeof(struct can_frame_attr))
								{
									printf("can frame size wrong\n");
									rval = -1;
									break;
								}
							}
							
							if(rval == 0)
							{//sort all the received frame by can id
								
							}//all can frame got
						}//
					}//return from select
				}//loop to receive and send can frame
			}
		}//open fifo
	}//file system created
	
	return rval;
}
