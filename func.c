#include<stdio.h>
#include<unistd.h>
#include<stdint.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<poll.h>

//ピン番号
#define PIN_PWM "P8_13"

//有効化の順序によって変化する番号
#define pwm_no 15

//ocp.□
#define ocp 3

//bone_capemgr.□
#define bone_capemgr 9

//周期
#define period 15000000

//最小デューティー比
#define min_duty 750000

//最大デューティー比
#define max_duty 3600000

//gpioの有効化関数
void gpio_export(int n){
    int fd;
    char buf[40];

    sprintf(buf, "%d", n);
    //低水準ストリームで開く
    fd = open("/sys/class/gpio/export",O_WRONLY);  //O_WRONLYはアクセスモード
    write(fd,buf,strlen(buf));
    close(fd);
}
//gpioの有効化解除関数
void gpio_unexport(int n){
    int fd;
    char buf[40];

    sprintf(buf, "%d", n);
    //低水準ストリームで開く
    fd = open("/sys/class/gpio/unexport",O_WRONLY);  //O_WRONLYはアクセスモード
    write(fd,buf,strlen(buf));
    close(fd);
}
//gpioの設定ファイルを開く関数
int gpio_open(int n, char *file, int flag){
    int fd;
    char buf[40];

    sprintf(buf,"/sys/class/gpio/gpio%d/%s", n, file);

    fd = open(buf, flag);
    return fd;
}
//指定されたパスから整数値を読み込む
int read_int(char path[]){
    int value;
    FILE *fp = fopen(path,"rb");
    fscanf(fp,"%d",&value);
    fclose(fp);
    return value;
}
//指定されたパスに整数値を書き込む
void write_int(char path[], int value){
    FILE *fp = fopen(path,"wb");
    fprintf(fp,"%d",value);
    fclose(fp);
}
//指定されたパスに文字列を書き込む
void write_string(char path[], char string[]){
    FILE *fp = fopen(path,"w");
    fprintf(fp,string);
    fclose(fp);
}
//PWM初期化関数
void init_pwm(int gpio){
    int i,fd;
    FILE *fp;

    //自由文字列
    char string1[50];
    char string2[50];

    //各文字列の定義
    char path_slots[50];
    char path_value[50];
    char path_polarity[50];
    char path_period[50];
    char path_duty[50];
    char path_run[50];

    gpio_export(gpio);
    fd=gpio_open(gpio, "direction", O_WRONLY);
    write(fd, "out", 3);
    close(fd);

    sprintf(path_value, "/sys/class/gpio/gpio%d/value", gpio);
    fp=fopen(path_value,"w");
    fprintf(fp, "%d", 0);
    fclose(fp);

    sprintf(path_slots, "/sys/devices/bone_capemgr.%d/slots", bone_capemgr);

    sprintf(string1, "am33xx_pwm");
    sprintf(string2, "bone_pwm_%s", PIN_PWM);
    write_string(path_slots, string1);
    write_string(path_slots, string2);

    //各パスを結びつける
    sprintf(path_polarity, "/sys/devices/ocp.%d/pwm_test_%s.%d/polarity", ocp ,PIN_PWM, pwm_no);
    sprintf(path_period, "/sys/devices/ocp.%d/pwm_test_%s.%d/period", ocp, PIN_PWM, pwm_no);
    sprintf(path_duty, "/sys/devices/ocp.%d/pwm_test_%s.%d/duty", ocp, PIN_PWM, pwm_no);
    sprintf(path_run, "/sys/devices/ocp.%d/pwm_test_%s.%d/run", ocp, PIN_PWM, pwm_no);

    //安全のためPWM出力を停止する
    write_int(path_run, 0);

    //周期を入力(1000000[ns])
    write_int(path_period, period);

    //極性を設定
    write_int(path_polarity, 0);

    //デューティー比を設定
    write_int(path_duty, min_duty);

    //PWM出力開始
    write_int(path_run, 1);
}
//PWM終了関数
void close_pwm(int gpio){
    FILE *fp;
    char path_duty[50];
    char path_run[50];
    
    sprintf(path_duty, "/sys/devices/ocp.%d/pwm_test_%s.%d/duty", ocp, PIN_PWM, pwm_no);
    write_int(path_duty, 0);

    sprintf(path_run, "/sys/devices/ocp.%d/pwm_test_%s.%d/run", ocp, PIN_PWM, pwm_no);
    write_int(path_run, 0);

    gpio_unexport(gpio);
}
//サーボモータ用出力関数
void servo_test(gpio){
    int angle;
    char path_value[50];
    char path_duty[50];

    sprintf(path_value, "/sys/class/gpio/gpio%d/value", gpio);
    write_int(path_value, 1);

    while(1){
        printf("angleを入力\n");
        scanf("%d",&angle);

        if(angle==-1){break;}

        sprintf(path_duty, "/sys/devices/ocp.%d/pwm_test_%s.%d/duty", ocp, PIN_PWM, pwm_no);
        write_int(path_duty, min_duty + angle * (max_duty - min_duty) / 180);
        usleep(200);
    }
}
