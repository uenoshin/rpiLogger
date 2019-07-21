#include <stdio.h>
#include <sys/time.h>

#include "pigpiod_if2.h"

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
static int pi;
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
	i2c_write_byte_data(pi, handle, ADXL343_BW_RATE, 	0x0f );
	i2c_write_byte_data(pi, handle, ADXL343_DATA_FORMAT,0x00 );
	i2c_write_byte_data(pi, handle, ADXL343_FIFO_CTL, 	0x00 );
	i2c_write_byte_data(pi, handle, ADXL343_POWER_CTL, 	0x08 );
}

void writeCSV(int x_dat, int y_dat, int z_dat )
{
	int32_t micros;
	
	// get_current_tick を使うと発信源の時刻情報を取れるのでいいが
	// 100マイクロ秒ぐらい遅くなる
	
	micros = get_current_tick(pi);
	// micros = timeStamp();
	fprintf(fp, "%10u: x:y:z ,%6d,%6d,%6d\n",micros-micros_old,x_dat,y_dat,z_dat );
	micros_old = micros;
}

void readSensor()
{
	int x_dat, y_dat, z_dat;

	while(1)
	{
		x_dat = i2c_read_word_data(pi, handle, ADXL343_DATAX0);
		y_dat = i2c_read_word_data(pi, handle, ADXL343_DATAY0);
		z_dat = i2c_read_word_data(pi, handle, ADXL343_DATAZ0);	
		// printf("xyz:,%d,%d,%d\n",x_dat,y_dat,z_dat);

		writeCSV( x_dat, y_dat, z_dat );
	}
}

int main(int argc, char *argv[])
{
	pi = pigpio_start(NULL,NULL);
// printf("pigpio_start =%d\n",pi);
	if ( pi < 0 ) return -1;
	
	handle = i2c_open(pi, 1, ADXL343_I2C_ADDR_PRIMARY, 0);
	fp = fopen("pigd_adxl343.csv","w");
// printf("i2c_open =%d\n",handle);

	initSensor();
	readSensor();

	fclose(fp);
	i2c_close(pi, handle);

	pigpio_stop(pi);
}


