#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <stdio.h>
#include "gw.h"
static char page_buf[2048];
unsigned int find_all_room(unsigned int room_list[][2])
{
	int i = 0;
	unsigned int count = 0;
	room_t *room_to = (room_t*)ROOM_BASE_ADDR;
	for(i = 0; i < 200; i++)
	{
		if(room_to->room_id[0]==EXIST)
		{
			room_list[count][0] = (unsigned int)room_to,
			room_list[count][1] = *(unsigned int*)(room_to->room_id);
			count++;
		}

		room_to++;
	}
	return count;
}
unsigned int find_room(room_t *room)
{
	int i = 0;
	char* addr = (char*)ROOM_BASE_ADDR;
	char *room_id = room->room_id;
	room_id[0] = 0x5a;
	for(i = 0; i < 200; i++)
	{
		if(memcmp(addr, room_id, 4)==0)
		{
			return (unsigned int)addr;
		}
		else
		{
			addr += sizeof(room_t);
		}
	}
	return 0;
}
unsigned int write_room(room_t *room)
{
	int i = 0;

	room->room_id[0] = EXIST;
	room_t *room_to = (room_t*)ROOM_BASE_ADDR;
	for(i = 0; i < 200; i++)
	{
		if(room_to->room_id[0]!=0x5a)
		{
			int page_addr_start = ((unsigned int)room_to/2048)*2048;
			int offset = (unsigned int)room_to%2048;
			memcpy(page_buf,page_addr_start,2048);
			memcpy(page_buf+offset,room,sizeof(room_t));
			FLASH_Unlock();
			FLASH_ErasePage((unsigned int)room_to);
			for(i = 0;i<2048;i+=4)
			{
				FLASH_ProgramWord(page_addr_start+i, *((unsigned int*)(page_buf+i)));
			}
			FLASH_Lock();	
			return (unsigned int)room_to;
		}
		else
		{
			room_to++;
		}
	}
	return 0;
}
unsigned int find_write_room(room_t *room)
{
	int i = 0;
	room->room_id[0] = EXIST;
	room_t *room_to = (room_t*)ROOM_BASE_ADDR;
	for(i = 0; i < 200; i++)
	{
		if(memcmp(room, room_to, 4)==0)
		{
			int page_addr_start = ((unsigned int)room_to/2048)*2048;
			int offset = (unsigned int)room_to%2048;
			memcpy(page_buf,page_addr_start,2048);
			memcpy(page_buf+offset,room,sizeof(room_t));
			FLASH_Unlock();
			FLASH_ErasePage((unsigned int)room_to);
			for(i = 0;i<2048;i+=4)
			{
				FLASH_ProgramWord(page_addr_start+i, *((unsigned int*)(page_buf+i)));
			}
			FLASH_Lock();	
			return (unsigned int)room_to;
		}
		else
		{
			room_to++;
		}
	}
	return 0;
}
unsigned int find_delete_room(room_t *room_read_only)
{
	int i = 0;
	room_t room_rw;
	room_t *room = &room_rw;
	memcpy(room, room_read_only,sizeof(room_t));
	room->room_id[0] = 0x5a;
	room_t *room_to = (room_t*)ROOM_BASE_ADDR;
	for(i = 0; i < 200; i++)
	{
		if(memcmp(room, room_to, 4)==0)
		{
			int page_addr_start = ((unsigned int)room_to/2048)*2048;
			int offset = (int)room_to%2048;
			room->room_id[0] = 0;
			memcpy(page_buf,page_addr_start,2048);
			memcpy(page_buf+offset,room,sizeof(room_t));
			FLASH_Unlock();
			FLASH_ErasePage((int)room_to);
			for(i = 0;i<2048;i+=4)
			{
				FLASH_ProgramWord(page_addr_start+i, *((int*)(page_buf+i)));
			}
			FLASH_Lock();	
			return (unsigned int)room_to;
		}
		else
		{
			room_to++;
		}
	}
	return 0;
}
int find_detector_in_room(int detector_id,room_t* room)
{
	int i;
	for(i = 0;i<6;i++)
	{
		if(
			(memcmp(room->detector[i]+1,&detector_id,3)==0)
			&&(room->detector[i][0]==EXIST)
			)

		{
			return i;
		}
	}
	return -1;
}
unsigned int find_detector(unsigned int detector_id,int *offset)
{
	int i = 0;
	room_t *room_to = (room_t*)ROOM_BASE_ADDR;
	for(i = 0; i < 200; i++)
	{
		if(((room_to->room_id[0])==EXIST)&&(find_detector_in_room(detector_id,room_to)>-1))
		{
			*offset = find_detector_in_room(detector_id,room_to)+1;
			return (unsigned int)room_to;
		}
		else
		{
			room_to++;
		}
	}
	return 0;
}
unsigned int find_delete_detector(int detector_id)
{
	int i = 0;
	room_t *room_to = (room_t*)ROOM_BASE_ADDR;
	for(i = 0; i < 200; i++)
	{
		if(((room_to->room_id[0])==EXIST)&&(find_detector_in_room(detector_id,room_to)>-1))
		{
			room_t room_buf;
			memcpy(&room_buf,room_to,sizeof(room_t));
			room_buf.detector[find_detector_in_room(detector_id,room_to)][0] = 0;
			find_write_room(&room_buf);
			return (unsigned int)room_to;
		}
		else
		{
			room_to++;
		}
	}
	return 0;
}


unsigned int find_all_rec(unsigned int rec_list[][2])
{
	int i = 0;
	unsigned int count = 0;
	rec_t *rec = (rec_t*)ALARM_RECORD_ADDR;
	for(i = 0; i < 200; i++)
	{
		if(rec->available==EXIST)
		{
			rec_list[count][0] = (unsigned int)rec,
			rec_list[count][1] = (unsigned int)(rec->date);
			count++;
		}

		rec++;

	}
	return count;
}
unsigned int add_rec(rec_t *rec)
{
	int i = 0;

	rec->available = EXIST;
	rec_t *rec_to = (rec_t*)ALARM_RECORD_ADDR;
	for(i = 0; i < 200; i++)
	{
		if(rec_to->available!=EXIST)
		{
			int page_addr_start = ((int)rec_to/2048)*2048;
			int offset = (unsigned int)rec_to%2048;
			memcpy(page_buf,page_addr_start,2048);
			memcpy(page_buf+offset,rec,sizeof(rec_t));
			FLASH_Unlock();
			FLASH_ErasePage((unsigned int)rec_to);
			for(i = 0;i<2048;i+=4)
			{
				FLASH_ProgramWord(page_addr_start+i, *((int*)(page_buf+i)));
			}
			FLASH_Lock();	
			return (unsigned int)rec_to;
		}
		else
		{
			rec_to++;
		}
	}
	return 0;
}
void wipe_all_rec(void)
{
	int i = 0;
	for(i = 0; i < 7; i++)
	{
			FLASH_Unlock();
			FLASH_ErasePage(ALARM_RECORD_ADDR+(2048*i));
			FLASH_Lock();	
	}
}
void wipe_all_room(void)
{
	int i = 0;
	for(i = 0; i < 7; i++)
	{
			FLASH_Unlock();
			FLASH_ErasePage(ROOM_BASE_ADDR+(2048*i));
			FLASH_Lock();	
	}
}
void wipe_all_publici_phone(void)
{

		FLASH_Unlock();
		FLASH_ErasePage(PUBLIC_PHONE_ADDR);
		FLASH_Lock();	

}
void wipe_passwd(void)
{

		FLASH_Unlock();
		FLASH_ErasePage(PASSWD_ADDR);
		FLASH_Lock();	

}

unsigned int find_delete_rec(rec_t *rec)
{
	int i = 0;
	rec->available = EXIST;
	rec_t *rec_to = (rec_t*)ALARM_RECORD_ADDR;
	for(i = 0; i < 200; i++)
	{

		if(memcmp(rec, rec_to, 15)==0)
		{
			int page_addr_start = ((unsigned int)rec_to/2048)*2048;
			int offset = (unsigned int)rec_to%2048;
			rec->available = 0;
			memcpy(page_buf,page_addr_start,2048);
			memcpy(page_buf+offset,rec,sizeof(rec_t));
			FLASH_Unlock();
			FLASH_ErasePage((unsigned int)rec_to);
			for(i = 0;i<2048;i+=4)
			{
				FLASH_ProgramWord(page_addr_start+i, *((unsigned int*)(page_buf+i)));
			}
			FLASH_Lock();	
			return (unsigned int)rec_to;
		}
		else
		{
			rec_to++;
		}
	}
	return 0;
}
unsigned int write_public_phone(public_phone_t *public)
{
	int i;
	FLASH_Unlock();
	FLASH_ErasePage(PUBLIC_PHONE_ADDR);
	for(i = 0;i<sizeof(public_phone_t);i+=4)
	{
		FLASH_ProgramWord(PUBLIC_PHONE_ADDR+i, *(unsigned int *)((unsigned int)public + i));
	}
	FLASH_Lock();	
	return 0;
}
