/*
 * (C) Copyright 2015 - 2016
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#ifndef WIN32
#include <endian.h>
#else

#define htole32(a) (a)
#endif

#ifndef WIN32
char *filename="/tmp/bootsetting.txt";
char *log_filename="/tmp/bootswitch_log.txt";
#else
char *filename="C:\\temp\\bootsetting.txt";
char *log_filename="C:\\temp\\bootswitch_log.txt";
#endif

FILE *fp_log = NULL;
#define DEF_BOOT_MODE 5 /* SD Card */
#define MLO_MAX_LEN (2048)

#define MAX_RETRIES (10)

char mlo_name[MLO_MAX_LEN];

char *boot_modes[] =
	{
		[1] = "XIP",
		[2] = "XIP with wait monitoring",
		[3] = "NAND",
		[5] = "SD Card",
		[6] = "eMMC boot partition",
		[7] = "eMMC",
		[9] = "SATA",
		[10] = "QSPI_1",
		[11] = "QSPI_4",
		[12] = "UART",
		[13] = "USB(from internal transceiver)"
	};

static char *get_boot_string(int boot_mode){

	/* Refer to Table 32-23 Booting Messages in TRM */
	switch(boot_mode){
	case 1:
	case 2:
	case 3:
	case 5:
	case 6:
	case 7:
	case 9:
	case 10:
	case 11:
		return boot_modes[boot_mode];
		break;
	case 0x43:
		return boot_modes[12];
		break;
	case 0x45:
		return boot_modes[13];
		break;
	}
	return NULL;
}

static ssize_t get_next_valid_line(char **lineptr, size_t *line_len,
                                   FILE *fp) {
	ssize_t retval=0;
	int linefound=0;
	size_t inp_len=*line_len;
	char *ret_str;

	linefound=0;
	while((!feof(fp)) && (linefound==0) &&  (retval!=-1)){

		retval=0;

		*line_len=inp_len;
		memset(*lineptr,0x00,inp_len);
		ret_str = fgets(*lineptr, inp_len, fp);

		if((ret_str == NULL) || ((*lineptr)[0] == '#'))
			linefound=0;
		else
			linefound=1;
	}

	return retval;
}

#define LINE_LEN (1024)
static int get_boot_mode(int *peripheral_boot, int *boot_mode,
                         char *mlo_name){

	FILE *fp;
	int count;
	char line[LINE_LEN];
	char *lineptr;
	size_t line_len=LINE_LEN;
	ssize_t retval=0;

	fp = fopen(filename,"r");
	if(fp==NULL) {
		fprintf(fp_log,"Unable to open %s\n",filename);
		*peripheral_boot = 0;
		*boot_mode = DEF_BOOT_MODE;
		return 0;
	}
	fprintf(fp_log,"Opened %s\n",filename);
	fflush(fp_log);

	lineptr=&line[0];
	line_len=LINE_LEN;
	retval = get_next_valid_line(&lineptr,&line_len,fp);
	if(retval<0) {
		fprintf(fp_log,"Unable to get valid line from %s\n",filename);
		fflush(fp_log);
		return 1;
	}
	count = sscanf(line,"%d:%d\n",peripheral_boot,boot_mode);
	if(count!=2) {
		fprintf(fp_log,"Unable to read boot mode from %s\n",filename);
		*peripheral_boot = 0;
		*boot_mode = DEF_BOOT_MODE;
	} else {
		lineptr=&line[0];
		line_len=LINE_LEN;
		retval = get_next_valid_line(&lineptr,&line_len,fp);
		count = sscanf(line,"%s\n",mlo_name);
	}

	fflush(fp_log);

	fclose(fp);

	return 0;
}


/*
 * bootswitch -s <path to first stage bootloader>
 */
int main(int argc, char *argv[])
{
	libusb_device_handle *devh;
	int r;
	unsigned int data = 0xF0030006;
	int count = 0;
	int boot_mode = 0;
	int per_boot = 0;
	char *boot_str;
	FILE *fp_mlo;
	unsigned int mlo_len;
	char *mlo_buff;
	unsigned int data_left=0;
	char *ptr;
	size_t fread_cnt=0;
	unsigned int tmp_int;
	int c;
	int i = 0;

	memset(mlo_name,0x00,MLO_MAX_LEN);

	while ((c = getopt (argc, argv, "s:S:")) != -1) {
		switch (c) {
		case 's':
		case 'S':
			per_boot = 1;
			strncpy(mlo_name,optarg, MLO_MAX_LEN);
			printf("MLO name is %s\n",mlo_name);
			break;
		default:
			abort ();
		}
	}

	fp_log = fopen(log_filename, "a");
	if (fp_log == NULL) {
		fprintf(stderr,"Unable to open log file\n");
		fprintf(stderr,"Using stderr for logging\n");
		fp_log = stderr;
	}

	if (per_boot == 0)
		get_boot_mode(&per_boot, &boot_mode, mlo_name);
	fprintf(fp_log,"Per boot is %d\nBoot mode is %d\n",per_boot, boot_mode);
	fprintf(fp_log,"MLO name %s\n",mlo_name);

	r = libusb_init(NULL);
	if (r < 0) {
		fprintf(fp_log,"Unable to init libusb\n");
		fflush(fp_log);
		goto total_fail;
	}

	do {
		//Look for DRA74x (J6)
		devh = libusb_open_device_with_vid_pid(NULL,0x451,0xd013);

		//Look for DRA72x/DRA71x (J6 Eco/J6 Entry)
		if(devh==NULL)
			devh = libusb_open_device_with_vid_pid(NULL,0x451,0xd014);
		if(devh==NULL) {
			fprintf(fp_log,"Unable to find Vayu EVM\n");
			fflush(fp_log);
		} else {
			fprintf(fp_log,"Opened device\n");
		}
		i++;
		if (devh == NULL)
			sleep(1);
	} while ((i < MAX_RETRIES) && (devh == NULL));

	if(devh == NULL)
		goto cleanup;

	if(libusb_kernel_driver_active(devh,0)){
		fprintf(fp_log,"kernel driver active\n");
	}

	r = libusb_claim_interface(devh,0);

	if (0 != r) {
		fprintf(fp_log,"unable to claim imterface 0: code %d\n",r);
		fflush(fp_log);
		goto cleanup;
	} else {
		fprintf(fp_log,"claimed interface 0\n");
	}

	if (per_boot==0) {
		boot_str = get_boot_string(boot_mode);

		if(boot_str==NULL){
			boot_mode = DEF_BOOT_MODE;
			boot_str = get_boot_string(boot_mode);
		}
		fprintf(fp_log,"Setting boot to %s\n",boot_str);
		data = (data | ((boot_mode&0xFF) << 8));

		r= libusb_bulk_transfer(devh,
					 0x1,
					 (unsigned char *) &data, 4, &count, 1);

		if((r!=0) || (count!=4)) {
			fprintf(fp_log,"Setting boot mode failed\n");
		} else {
			fprintf(fp_log,"Setting boot mode successful\n");
			fflush(fp_log);
		}
	} else {

		fp_mlo = fopen(mlo_name,"rb");
		if(fp_mlo==NULL) {
			fprintf(fp_log,"Unable to open MLO %s\n",mlo_name);
			fprintf(fp_log,"Exiting\n");
			fflush(fp_log);
			goto release_libusb;
		}
		fseek(fp_mlo,0,SEEK_END);
		mlo_len = ftell(fp_mlo);
		fseek(fp_mlo,0,SEEK_SET);
		fprintf(fp_log,"MLO len is %d bytes:0x%08x\n",mlo_len,mlo_len);

		data = 0xF0030002;
		r = libusb_bulk_transfer(devh,
					 0x1,
					 (unsigned char *) &data, 4, &count, 1);
		if((r!=0) || (count!=4)) {
			fprintf(fp_log,"Setting peripheral boot mode failed\n");
			fflush(fp_log);
			goto release_libusb;
		} else {
			fprintf(fp_log,"Setting peripheral boot mode successful\n");
		}


		tmp_int = htole32(mlo_len);
		r = libusb_bulk_transfer(devh,
					 0x1,
					 (unsigned char *) &tmp_int, 4, &count, 0);
		if((r!=0) || (count!=4)) {
			fprintf(fp_log,"Setting MLO length failed\n");
			fflush(fp_log);
			goto release_libusb;
		} else {
			fprintf(fp_log,"Setting MLO length successful\n");
		}

		mlo_buff =(char *)malloc(mlo_len);

		fread_cnt = fread(mlo_buff,1,mlo_len,fp_mlo);
		if(fread_cnt != mlo_len) {
			fprintf(fp_log,"Could not read %d bytes from %s\n",
			        mlo_len, mlo_name);
			fflush(fp_log);
			exit(1);
		}

		data_left = mlo_len;
		ptr = mlo_buff;

		while((data_left>0) && (r==0)) {
			unsigned int tr_len=100*1024;

			if(tr_len>data_left)
				tr_len = data_left;

			r = libusb_bulk_transfer(devh,
						 0x1,
			                         (unsigned char *) ptr,
			                         tr_len, &count, 100);
			if((r!=0) || (count!=tr_len)) {
				fprintf(fp_log,"Transferring MLO failed\n");
				fprintf(fp_log,"Ret value %d; "
				        "transferred len %d/%d\n",
				        r,count,tr_len);
				fflush(fp_log);
			} else {
				data_left-=tr_len;
				ptr+=tr_len;
				fprintf(fp_log,"Transferring MLO successful %d/%d\n",data_left,mlo_len);
			}
		}
		free(mlo_buff);
		fclose(fp_mlo);
	}

 release_libusb:
	libusb_release_interface(devh,0);
	libusb_close(devh);

 cleanup:
	libusb_exit(NULL);
 total_fail:
	fflush(fp_log);
	fclose(fp_log);
	return 0;
}
