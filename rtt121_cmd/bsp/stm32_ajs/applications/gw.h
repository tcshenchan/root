#ifndef __GW_H
#define __GW_H
#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <stdio.h>
#define EXIST 0x5a
#define ROOM_BASE_ADDR  0x8060000 //用于寸放房间数据的起始地址，12.5k，
#define PASSWD_ADDR     (0x8060000 + 1024*14)
#define PUBLIC_PHONE_ADDR (0x8060000 + 1024*16)
#define ALARM_RECORD_ADDR (0x8060000 + 1024*18)


typedef struct {
	//数组的第一个数表示该组数据有无效，值为0x5a有效，否则无效
	unsigned char room_id[4]; //栋，单元，号
	char phone[3][12];
	unsigned char detector[6][4];
} room_t;
typedef struct {
	unsigned char available; 
	unsigned char date[15];
	unsigned char text[48];
} rec_t;
typedef struct {
	unsigned char phone[6][12];
} public_phone_t;
unsigned int find_room(room_t *room);
unsigned int write_room(room_t *room);
unsigned int find_delete_room(room_t *room_read_only);
unsigned int find_write_room(room_t *room);
unsigned int find_all_room(unsigned int room_list[][2]);
unsigned int find_detector(unsigned int detector_id,int *offset);
unsigned int find_delete_detector(int detector_id);
unsigned int find_delete_rec(rec_t *rec);
unsigned int add_rec(rec_t *rec);
unsigned int find_all_rec(unsigned int rec_list[][2]);
unsigned int write_public_phone(public_phone_t *public);



void wipe_all_rec(void);
void wipe_all_room(void);
void wipe_all_publici_phone(void);
void wipe_passwd(void);


extern char key_table[20];
#define KEY_VALUE key_table[key_value_read()]
int modif_input_panel(const char* title, char* str, int len, int  fix);
#define input_panel(a,b,c) modif_input_panel(a,b,c,0)
int str_separation(char* src, int dst[],int max_count);
int show_menu_left(const char menu[][20],int (*func_list[3])(void*), void* para[], int all_row_count,int x);
int show_menu_full_func(const char menu[][20],int (*func_list[3])(void*), void* para[], int all_row_count,int x,int row);
int input_room_id(room_t* room);
int input_detector(unsigned int *detector_id);
int str_to_unicode(char* str,char* str_out, int len, int big_endian);
int edit_phone(void *para);
int save_signal(void *para);

int room_manage(void* para);
int detector_manage(void* para);
int settings(void* parameter);
int scan_recording(void* para);

#define show_menu(a,b,c,d) show_menu_left(a,b,c,d,16)

int wav_out(int sampling,unsigned char* start, int len);
void gsm_thread_run(void);
void listen_thread_run(void);
void alarm_thread_run(void);
void passwd_init(void);
extern int rssi;
extern int net_sta_in;
extern char *time_str_display;
extern unsigned int voice_running;
extern int alarm_swicth;
extern char alarm_str[30];
extern char focus_on_home_ui;
extern int back_led_life;
extern rt_thread_t main_ui_thread;
#include "config.h"

#define back_led_on() {GPIO_SetBits(GPIOA,GPIO_Pin_4);back_led_life = 10;}
#define back_led_off() GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define pulse_led(a) PCout(9)=!a
#define alarm_led(a) PCout(7)=!a
#define breakdown_led(a) PCout(8)=!a
#define ext_control(a) PEout(12)=a

#define dc_detecte() !PBin(1)
#define bat_detecte() !PBin(0)
#endif