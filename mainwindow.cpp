#include "fpga_dot_font.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <math.h>
int flag_dip[4];
int dip[4];
int flag_push[9];
int push[9];
int n = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isRun = 0;

    total_time = 0;
    current_time=0;
    //dot matrix
    dev_dot = open(FPGA_DOT_DEVICE,O_RDWR);
    if (dev_dot<0){
        printf("Device open error : %s\n",FPGA_DOT_DEVICE);
        msgBox.setText("Device Open Error!");
        msgBox.setInformativeText("Check Module & Device node");
        msgBox.exec();
        exit(1);
    }

    amount=0;
    str_size = sizeof(fpga_progress[amount]);
    write(dev_dot,fpga_progress[amount],str_size);

    //add dip switch
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),this,SLOT(update()));
    timer->start(100);

    dev_dip = open(FPGA_DIP_SWITCH_DEVICE,O_RDWR);
    if(dev_dip<0){
        printf("Device Open Error : %s\n",FPGA_DIP_SWITCH_DEVICE);
        msgBox.setText("Device Open Error!");
        msgBox.setInformativeText("Check Module & Device node");
        msgBox.exec();
        exit(1);
    }

    for (int i=0; i<4; i++) {
        flag_dip[i] = 1;
    }

    //add led
    dev_led = open(LED_DEVICE,O_RDWR);
    if(dev_led<0){
        printf("Device open error:%s\n",LED_DEVICE);
        msgBox.setText("Device Open Error!");
        msgBox.setInformativeText("Check Module & DEVICE node");
        msgBox.exec();
        exit(1);
    }
    data = 0x0F;
    write(dev_led, &data, 1);

    //add push
    dev_push = open(FPGA_PUSH_SWITCH_DEVICE,O_RDWR);
    if(dev_push<0){
        printf("Device open error:%s\n",FPGA_PUSH_SWITCH_DEVICE);
        msgBox.setText("Device Open Error!");
        msgBox.setInformativeText("Check Module & DEVICE node");
        msgBox.exec();
        exit(1);
    }
    buff_size=sizeof(push_sw_buff);

    //add step motor
    dev_step = open(FPGA_STEP_MOTOR_DEVICE,O_RDWR);
    if(dev_step<0){
        printf("Device open error:%s\n",FPGA_STEP_MOTOR_DEVICE);
        msgBox.setText("Device Open Error!");
        msgBox.setInformativeText("Check Module & DEVICE node");
        msgBox.exec();
        exit(1);
    }
    memset(motor_state,0,sizeof(motor_state));
    motor_action=0;
    motor_direction=0;
    motor_speed=250;
    motor_state[0]=(unsigned char)motor_action;
    motor_state[1]=(unsigned char)motor_direction;
    motor_state[2]=(unsigned char)motor_speed;
    write(dev_step,motor_state,3);


    //add LCD
    dev_lcd = open(FPGA_TEXT_LCD_DEVICE, O_RDWR);
    if (dev_lcd < 0) {
        printf("Device open error:%s\n", FPGA_TEXT_LCD_DEVICE);
        msgBox.setText("Device Open Error!");
        msgBox.setInformativeText("Check Module & DEVICE node");
        msgBox.exec();
        exit(1);
    }
    memset(string, ' ', 32);
    strcpy(string, "Please enter PW       ");
    write(dev_lcd, string, MAX_BUFF);

    memset(passwd, 0, 4);

    //add FND
    dev_fnd = open(FND_DEVICE,O_RDWR);
    if (dev_fnd<0){
        printf("Device open error : %s\n",FND_DEVICE);
        msgBox.setText("Device Open Error!");
        msgBox.setInformativeText("Check Module & Device node");
        msgBox.exec();
        exit(1);
    }

    fnd_data[0] = '0';
    fnd_data[1] = '0';
    fnd_data[2] = '0';
    fnd_data[3] = '0';

    write(dev_fnd, &fnd_data , 4);

    status = QString("e");
    remainTime = QString("0");
    amountLaundry = QString("1");
    washingMode = QString("t");
    rinseMode = QString("t");
    dehydrationMode = QString("t");
    DryMode = QString("t");

    //server
    server = new Server();

    connect(server, SIGNAL(connect_signal()),this, SLOT(slot_connect()));

    server->listen(QHostAddress::Any, 35000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Active_BT_clicked()
{
    for(int i = 0; i<4; i++){
        if(mode[i] == 0) {
            total_time += 10;
        }
        else
            total_time += 20;
    }

    if(amount == 0)
        total_time += 10;
    else if(amount == 1)
        total_time += 20;
    else
        total_time += 30;

    fnd_data[0] = '0';
    fnd_data[3] = total_time % 10 - 0x30;
    fnd_data[2] = total_time / 10 - 0x30;
    fnd_data[1] = total_time / 100 - 0x30;


    current_time = total_time;

    write(dev_fnd, &fnd_data , 4);

    ui->FND_Value->setText(QString("%1").arg(total_time));


    memset(motor_state,0,sizeof(motor_state));
    motor_action=1;
    motor_direction = 0;
    motor_speed = 250;
    motor_state[0]=(unsigned char)motor_action;
    motor_state[1]=(unsigned char)motor_direction;
    motor_state[2]=(unsigned char)motor_speed;
    write(dev_step,motor_state,3);

    total_timer = new QTimer(this);
    connect(total_timer, SIGNAL(timeout()),this,SLOT(update_1()));
    total_timer->start(500);

    // server

    status = QString("w");
    remainTime = QString("%1").arg(current_time);
    amountLaundry = QString("%1").arg(amount + 1);

    QString message = QString("%1,%2,%3,%4,%5,%6,%7,")
            .arg(status)
            .arg(remainTime)
            .arg(amountLaundry)
            .arg(washingMode)
            .arg(rinseMode)
            .arg(dehydrationMode)
            .arg(DryMode);

    server->sendMessage(message);
    //server end

    isRun = 1;
    ui->Active_BT->setChecked(false);
}

void MainWindow::on_Up_BT_clicked()
{
    amount++;
    if (amount == 1) {
        ui->progressBar->setValue(2);
    }
    else if (amount == 2){
        ui->progressBar->setValue(3);
    }
    else if (amount == 3) {
        ui->progressBar->setValue(1);
    }

    if(amount>2)
        amount=0;

    set_num = amount;


    str_size = sizeof(fpga_progress[set_num]);
    write(dev_dot,fpga_progress[set_num],str_size);

}

void MainWindow::on_Down_BT_clicked()
{
    amount--;
    if (amount == -1) {
        ui->progressBar->setValue(3);
    }
    else if (amount == 0) {
        ui->progressBar->setValue(1);
    }
    else if (amount == 1) {
        ui->progressBar->setValue(2);
    }
    if(amount<0)
        amount=2;

    set_num = amount;

    str_size = sizeof(fpga_progress[set_num]);
    write(dev_dot,fpga_progress[set_num],str_size);
}
char data[4];
void MainWindow::update_1()
{

    current_time -= 1;

    fnd_data[0] = '0';
    fnd_data[3] = current_time % 10 - 0x30;
    fnd_data[2] = current_time / 10 - 0x30;
    fnd_data[1] = current_time / 100 - 0x30;


    ui->FND_Value->setText(QString("%1").arg(current_time));
    write(dev_fnd, &fnd_data , 4);

    // server
    int cal_time = total_time /4 ;

    memset(string, ' ', 32);
    if((total_time - current_time) <= cal_time) {
        status = QString("w");
        strcpy(string, "Washing");
        string[strlen("Washing")] = ' ';
        ui->lcd_value->setText(QString("Washing"));
    } else if((total_time - current_time) <= cal_time * 2) {
        status = QString("r");
        strcpy(string, "Rinse");
        ui->lcd_value->setText("Rinse");
    } else if((total_time - current_time) <= cal_time * 3) {
        status = QString("h");
        strcpy(string, "Dehydration");
        ui->lcd_value->setText("Dehydration");
    } else if((total_time - current_time) <= cal_time * 4) {
        status = QString("d");
        strcpy(string, "Dry");
        ui->lcd_value->setText("Dry");
    } else if((total_time - current_time) == 0) {
        status = QString("e");
        strcpy(string, "End");
        ui->lcd_value->setText("End");
    }
    write(dev_lcd,string,MAX_BUFF);


    remainTime = QString("%1").arg(current_time);
    amountLaundry = QString("%1").arg(amount + 1);

    QString message = QString("%1,%2,%3,%4,%5,%6,%7,")
            .arg(status)
            .arg(remainTime)
            .arg(amountLaundry)
            .arg(washingMode)
            .arg(rinseMode)
            .arg(dehydrationMode)
            .arg(DryMode);

    server->sendMessage(message);
    //server end

    if (current_time == 0) {
        ui->lcd_value->setText("Please enter PW");

        memset(string, ' ', 32);
        strcpy(string, "Please enter PW       ");
        write(dev_lcd, string, MAX_BUFF);

        memset(motor_state,0,sizeof(motor_state));
        motor_action=0;
        motor_direction = 0;
        motor_speed = 250;
        motor_state[0]=(unsigned char)motor_action;
        motor_state[1]=(unsigned char)motor_direction;
        motor_state[2]=(unsigned char)motor_speed;
        write(dev_step,motor_state,3);

        QString message = QString("e,%1,%2,%3,%4,%5,%6,")
                .arg(remainTime)
                .arg(amountLaundry)
                .arg(washingMode)
                .arg(rinseMode)
                .arg(dehydrationMode)
                .arg(DryMode);

        server->sendMessage(message);
        isRun = 2;
        total_timer->stop();

        current_time = 0;
        total_time = 0;


    }

}

//add dip swtich
void MainWindow::update()
{
    if (isRun == 0 || isRun == 2) {
    read(dev_dip,&dip_sw_buff,1);

    for (int i=0;i<4;i++) {
        dip[i] = dip_sw_buff%2;
        dip_sw_buff = dip_sw_buff/2;
    }

    for (int i=0; i<4; i++) {
            if (dip[i] == 1) {
                data |= (1<<(3-i));
                data &= ~(1<<((3-i)+4));
                switch(i) {
                case 0:
                    ui->checkBox->setChecked(0);
                    ui->checkBox_2->setChecked(1);
                    washingMode = QString ("t");
                    mode[0] = 0;
                    break;
                case 1:
                    ui->checkBox_3->setChecked(0);
                    ui->checkBox_4->setChecked(1);
                    rinseMode = QString("t");
                    mode[1] = 0;
                    break;
                case 2:
                    ui->checkBox_5->setChecked(0);
                    ui->checkBox_6->setChecked(1);
                    dehydrationMode = QString("t");
                    mode[2] = 0;
                    break;
                case 3:
                    ui->checkBox_7->setChecked(0);
                    ui->checkBox_8->setChecked(1);
                    DryMode = QString("t");
                    mode[3] = 0;
                    break;
                }
            }
            else if (dip[i] == 0) {
                data &= ~(1<<(3-i));
                data |= (1<<((3-i)+4));
                switch(i) {
                case 0:
                    ui->checkBox->setChecked(1);
                    ui->checkBox_2->setChecked(0);
                    washingMode = QString ("s");
                    mode[0] = 1;
                    break;
                case 1:
                    ui->checkBox_3->setChecked(1);
                    ui->checkBox_4->setChecked(0);
                    rinseMode = QString("s");
                    mode[1] = 1;
                    break;
                case 2:
                    ui->checkBox_5->setChecked(1);
                    ui->checkBox_6->setChecked(0);
                    dehydrationMode = QString("s");
                    mode[2] = 1;
                    break;
                case 3:
                    ui->checkBox_7->setChecked(1);
                    ui->checkBox_8->setChecked(0);
                    DryMode = QString("s");
                    mode[3] = 1;
                    break;
                }
            write(dev_led, &data, 1);
        }
    }
    for (int i=0; i<4; i++) {
        flag_dip[i] = dip[i];
    }


    //add push switch
    read(dev_push, &push_sw_buff,buff_size);
    for(j=0;j<MAX_BUTTON;j++)
    {
        if(push_sw_buff[j]==1)
        {
            switch(j)
            {
            case 0:
                ui->pushButton_1->setChecked(1);
                passwd[n++] = '1';
                break;
            case 1:
                ui->pushButton_2->setChecked(1);
                passwd[n++] = '2';
                break;
            case 2:
                ui->pushButton_3->setChecked(1);
                passwd[n++] = '3';
                break;
            case 3:
                ui->pushButton_4->setChecked(1);
                passwd[n++] = '4';
                break;
            case 4:
                ui->pushButton_5->setChecked(1);
                passwd[n++] = '5';
                break;
            case 5:
                ui->pushButton_6->setChecked(1);
                passwd[n++] = '6';
                break;
            case 6:
                ui->pushButton_7->setChecked(1);
                passwd[n++] = '7';
                break;
            case 7:
                ui->pushButton_8->setChecked(1);
                passwd[n++] = '8';
                break;
            case 8:
                ui->pushButton_9->setChecked(1);
                passwd[n++] = '9';
                break;
            }
            strcat(string, "*");
            write(dev_lcd, string, MAX_BUFF);
        }
        else
        {
            switch(j)
            {
            case 0:
                ui->pushButton_1->setChecked(0);
                break;
            case 1:
                ui->pushButton_2->setChecked(0);
                break;
            case 2:
                ui->pushButton_3->setChecked(0);
                break;
            case 3:
                ui->pushButton_4->setChecked(0);
                break;
            case 4:
                ui->pushButton_5->setChecked(0);
                break;
            case 5:
                ui->pushButton_6->setChecked(0);
                break;
            case 6:
                ui->pushButton_7->setChecked(0);
                break;
            case 7:
                ui->pushButton_8->setChecked(0);
                break;
            case 8:
                ui->pushButton_9->setChecked(0);
                break;
            }
        }
    }
    for (int i=0; i<9; i++) {
        push_sw_buff[i] = push[i];
    }
    passwd[4] = '\0';

    if (n == 4) {
        if (!isRun) {
            n = 0;
            ui->lcd_value->setText("Passwd set!!");
            strcpy(set_passwd, passwd);
            memset(string, ' ', 32);
            strcpy(string, "Please enter PW       ");
            write(dev_lcd, string, MAX_BUFF);
        }
        else if (isRun == 2) {
            if (!strcmp(passwd, set_passwd)) {
                ui->lcd_value->setText("[Open] Passwd Correct!! ");
                strcpy(set_passwd, passwd);
                memset(string, ' ', 32);
                strcpy(string, "Passwd Correct!!       ");
                write(dev_lcd, string, MAX_BUFF);
                n = 0;
                isRun = 0;
            }
            else {
                n = 0;
                ui->lcd_value->setText("[ERROR] Passwd Wrong... ");
                strcpy(set_passwd, passwd);
                memset(string, ' ', 32);
                strcpy(string, "Passwd Wrong...       ");
                write(dev_lcd, string, MAX_BUFF);
            }
        }
    }
    }
}

/* server */
void MainWindow::slot_connect()
{
    if(total_time != 0) {
    int cal_time = total_time /4 ;

    if((total_time - current_time) <= cal_time) {
        status = QString("w");
    } else if((total_time - current_time) <= cal_time * 2) {
        status = QString("r");
    } else if((total_time - current_time) <= cal_time * 3) {
        status = QString("h");
    } else if((total_time - current_time) <= cal_time * 4) {
        status = QString("d");
    } else if((total_time - current_time) == 0) {
        status = QString("e");
    }
    } else {
        status = QString("e");
    }

    remainTime = QString("%1").arg(current_time);
    amountLaundry = QString("%1").arg(amount + 1);

    QString message = QString("%1,%2,%3,%4,%5,%6,%7,")
            .arg(status)
            .arg(remainTime)
            .arg(amountLaundry)
            .arg(washingMode)
            .arg(rinseMode)
            .arg(dehydrationMode)
            .arg(DryMode);

    server->sendMessage(message);
}

void MainWindow::on_Close_BT_clicked()
{
    int off = 0;
    write(dev_led, &off, 1);

    write(dev_dot,fpga_progress[3],str_size);

    char data_0[4] = {'0', '0', '0', '0'};
    write(dev_fnd, &data_0, 4);

    memset(string, ' ', 32);
    write(dev_lcd, string, MAX_BUFF);

    memset(motor_state,0,sizeof(motor_state));
    motor_action=0;
    motor_direction = 0;
    motor_speed = 250;
    motor_state[0]=(unsigned char)motor_action;
    motor_state[1]=(unsigned char)motor_direction;
    motor_state[2]=(unsigned char)motor_speed;
    write(dev_step,motor_state,3);

    this->close();
}

//add push switch
void MainWindow::on_pwdSet_BT_clicked()
{
    pwdSet_Flag = 1;
}

