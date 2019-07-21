#include <stdio.h>
#include <sys/time.h>

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "pigpio.h"

#define ADXL343_I2C_ADDR_PRIMARY	0x53
#define ADXL343_BW_RATE 			0x2C
#define ADXL343_DATA_FORMAT			0x31
#define ADXL343_FIFO_CTL			0x38
#define ADXL343_POWER_CTL			0x2D
#define ADXL343_DATAX0				0x32
#define ADXL343_DATAY0				0x34
#define ADXL343_DATAZ0				0x36


static int handle;
static FILE *fp;
static int32_t micros_old ;


int32_t timeStamp()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	int32_t time_in_micros = now.tv_usec;
	return time_in_micros;
}

void initSensor()
{
	/* BW_RATE : 0x0f = 1600Hz */
	i2cWriteByteData(handle, ADXL343_BW_RATE, 		0x0f );
	i2cWriteByteData(handle, ADXL343_DATA_FORMAT, 	0x00 );
	i2cWriteByteData(handle, ADXL343_FIFO_CTL, 		0x00 );
	i2cWriteByteData(handle, ADXL343_POWER_CTL, 	0x08 );
}

void writeCSV(int x_dat, int y_dat, int z_dat )
{
	int32_t micros;
	micros = gpioTick();
	// micros = timeStamp();
	
	fprintf(fp, "%10u: x:y:z ,%6d,%6d,%6d\n",micros-micros_old,x_dat,y_dat,z_dat );
	micros_old = micros;
}

void readAllSensor()
{
	int x_dat, y_dat, z_dat;
	char buf[8];

	while(1)
	{
//		x_dat = i2cReadWordData(handle, ADXL343_DATAX0);
//		y_dat = i2cReadWordData(handle, ADXL343_DATAY0);
//		z_dat = i2cReadWordData(handle, ADXL343_DATAZ0);	
		i2cReadI2CBlockData(handle, ADXL343_DATAX0, buf, 6);
		x_dat=(int)(buf[0]*256 + buf[1]);
		y_dat=(int)(buf[2]*256 + buf[3]);
		z_dat=(int)(buf[4]*256 + buf[5]);
		
		writeCSV( x_dat, y_dat, z_dat );
	}
}


void readSensor()
{
	int x_dat, y_dat, z_dat;
	char buf[8];
	i2cReadI2CBlockData(handle, ADXL343_DATAX0, buf, 6);
	x_dat=(int)(buf[0]*256 +buf[1]);
	y_dat=(int)(buf[2]*256 +buf[3]);
	z_dat=(int)(buf[4]*256 +buf[5]);
	writeCSV( x_dat, y_dat, z_dat );
	return;
}

void timer_handler(int signum)
{
	readSensor();
}

int main(int argc, char *argv[])
{
	// gpioCfgDMAchannels(7,6);
	// gpioCfgBufferSize(1000);
	
	if (gpioInitialise() < 0) return 1;
	handle = i2cOpen(1, ADXL343_I2C_ADDR_PRIMARY, 0);
	fp = fopen("pig_adxl343.csv","w");

	initSensor();

	// readAllSensor();

	// Timer
    struct sigaction act, oldact;
    timer_t tid;
    struct itimerspec itval;
    memset(&act, 0, sizeof(struct sigaction));
    memset(&oldact, 0, sizeof(struct sigaction));
    act.sa_handler = timer_handler;
    act.sa_flags = SA_RESTART;
    if(sigaction(SIGALRM, &act, &oldact) < 0) {
        return -1;
    }
	itval.it_value.tv_sec = 1;
    itval.it_value.tv_nsec = 0;
    itval.it_interval.tv_sec = 0;
    itval.it_interval.tv_nsec = 500*1000;
    if(timer_create(CLOCK_REALTIME, NULL, &tid) < 0) {
        return -1;
    }
 
    if(timer_settime(tid, 0, &itval, NULL) < 0) {
        return -1;
    }
 
    while(1) {
        pause();
    }
 
    timer_delete(tid);
    sigaction(SIGALRM, &oldact, NULL);

	fclose(fp);
	i2cClose(handle);
	gpioTerminate();
}


