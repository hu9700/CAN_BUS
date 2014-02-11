
#define FRAME_EXTENSION

/*
union can_id
{
	unsigned char can_id_basic;
	unsigned int  can_id_exten;
};*/

enum can_frame_type
{
	ft_data_frame = 1,
	ft_remote_frame,
	ft_active_error_frame,
	ft_passive_error_frame,
	ft_overload_frame
};

struct can_frame_attr
{
	unsigned int id;//union can_id id;
	unsigned int id_mask;//union can_id id_mask;
	unsigned char *data;
	unsigned char data_size;
	enum can_frame_type frame_type;
	char flag_is_exten;
};

extern int send_canbus(struct can_frame_attr *frame);//if it's not data frame, data and data_size field can be 0
extern int receve_canbus(struct can_frame_attr *frame);//if want to receive only from one id, id mask can be 0, id should be specified, 
														//if just listen the whole bus, id mask should be specified
int ccp_send_datareqest()
{
	struct can_frame_attr frame_attr;
	
	#ifdef FRAME_EXTENSION
	
	#else
	
	#endif
}

int ccp_send_datareply()
{

}
