#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//dot matrix header
#include <QMainWindow>
#include <QMessageBox>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <server.h>

#define FPGA_DOT_DEVICE "/dev/fpga_dot"

//add dip switch header
#include <QTimer>
#include <sys/ioctl.h>
#include <signal.h>


#define FPGA_DIP_SWITCH_DEVICE "/dev/fpga_dip_switch"

//add push switch
#include <sys/io.h>

#define MAX_BUTTON 9
#define FPGA_PUSH_SWITCH_DEVICE "/dev/fpga_push_switch"

//add led header
#define LED_DEVICE "/dev/fpga_led"

//add step motor header
#define MOTOR_ATTRIBUTE_ERROR_RANGE 4
#define FPGA_STEP_MOTOR_DEVICE "/dev/fpga_step_motor"

//add LCD
#define MAX_BUFF 32
#define LINE_BUFF 16
#define FPGA_TEXT_LCD_DEVICE        "/dev/fpga_text_lcd"

//add FND
#define MAX_DIGIT 4
#define FND_DEVICE "/dev/fpga_fnd"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void itoa(int value, char* str, int base);
    void strreverse(char* begin, char* end);

private slots:
    //dot matrix
    void on_Up_BT_clicked();

    void on_Down_BT_clicked();

    void on_Close_BT_clicked();

    //add dip switch
    void update();
    void update_1();

    //add push switch
    void on_pwdSet_BT_clicked();

    void on_Active_BT_clicked();

    /* server */
      void slot_connect();


private:
    int isRun;

    //dot matrix
    Ui::MainWindow *ui;
    int amount;
    int dev_dot;
    int str_size;
    int set_num;
    int pwdSet_Flag;
    int pwd[4];

    //led
    int dev_led;
    unsigned char data;
    unsigned char retval;

    QMessageBox msgBox;

    //dip switch
    int dev_dip;
    unsigned char dip_sw_buff;

    //push switch
    int j;
    int dev_push;
    char passwd[5];
    char set_passwd[5];
    int buff_size;
    unsigned char push_sw_buff[MAX_BUTTON];

    //LCD
    int dev_lcd;
    int lcd_str_size, str_size2;
    int chk_size;
    char string[32];

    //Step Motor
    int dev_step;
    int motor_action;
    int motor_direction;
    int motor_speed;
    unsigned char motor_state[3];

    //FND
    int dev_fnd;
    unsigned char fnd_data[4];
    unsigned char fnd_retval;
    int k;
    int fnd_str_size;

    int total_time;
    int current_time;

    QTimer *total_timer;

    int mode[4] = {0,};



    Server *server;
     QString status;
     QString remainTime;
     QString amountLaundry;
     QString washingMode;
     QString rinseMode;
     QString dehydrationMode;
     QString DryMode;
};

#endif // MAINWINDOW_H
