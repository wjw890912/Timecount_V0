






	  开发日志

 MODBUS RTU 主要针对了基于RS485的 M0上的产品需求而编写。
 此外已经顺利的在 ARM 公司的 M3内核CPU和M4内核CPU运行，
 其中M4内核CPU的MODBUS 是已经用在项目中去了。

1、移植MODBUS1.60到M051系列上。
1）、修改MODBUS协议栈的上层应用线圈和保持寄存器全部改为8个字节
/* -----------------------Slave Defines -------------------------------------*/
#define S_DISCRETE_INPUT_START        1
#define S_DISCRETE_INPUT_NDISCRETES   8
#define S_COIL_START                  1
#define S_COIL_NCOILS                 8
#define S_REG_INPUT_START             1
#define S_REG_INPUT_NREGS             8
#define S_REG_HOLDING_START           1
#define S_REG_HOLDING_NREGS           8
/* -----------------------Master Defines -------------------------------------*/
#define M_DISCRETE_INPUT_START        1
#define M_DISCRETE_INPUT_NDISCRETES   8
#define M_COIL_START                  1
#define M_COIL_NCOILS                 8
#define M_REG_INPUT_START             1
#define M_REG_INPUT_NREGS             8
#define M_REG_HOLDING_START           1
#define M_REG_HOLDING_NREGS           8
2）、修改定时器文件，开启20KHZ的定时器1.
3）、修改串口收发以及发送空中断和接收中断。
4）、预定波特率为9600  NONE校验 数据位8 停止1 
20160902

2、硬件上使用的是在比特电子设计的4路灯控继电器的PCB板。
1）RS485部分是自动的，那是那块电路是姚工原来的设计，有问题。
发送的时候接收可以收到几个字节。这样导致了MODBUS协议栈会上报一个错误的代码。
依次循环，会引发发送风暴。针对这个电路修改了代码的异常处理部分，是强制关闭错误MODBUS代码上报。
值得注意的是，在我自己的设计中应该去掉这个改动。因为我的那个电路是OK的。不修改也可以但是就没有了错误诊断的功能。
修改了
/* If the request was not sent to the broadcast address we
             * return a reply. */
            if( ucRcvAddress != MB_ADDRESS_BROADCAST )
            {
                if( eException != MB_EX_NONE )
                {
                    /* An exception occured. Build an error frame. */
                    usLength = 0;
                    ucMBFrame[usLength++] = ( UCHAR )( ucFunctionCode | MB_FUNC_ERROR );
                    ucMBFrame[usLength++] = eException;
					break;
					/*是为了适应姚工电路（他的发送接收可以收到，并且收不全），
					这个break是为了避免MODBUS接收到错误数据而上报错误状态码。
					我自己的自动收发电路无需修改。
                	*/
				}
                eStatus = peMBFrameSendCur( ucMBAddress, ucMBFrame, usLength );


上面的代码段中加入了一个break，是代码提前结束。:)


2）、使用MODBUS poll 进行读取HOLD寄存器值0-5 OK 测试OK 。
20160902



3、增加磁保持继电器的驱动和通过修改HOLD寄存器控制4路继电器的框架程序文件 realyctrl.c
使用的PCB是为比特开发的GLCS207-ER4GS T1 V1

 usSRegHoldBuf[0] 值是0表示第一路磁保持继电器关闭 1表示第1路继电器开
 usSRegHoldBuf[1] 值是0表示第一路磁保持继电器关闭 1表示第2路继电器打开
 usSRegHoldBuf[2] 值是0表示第一路磁保持继电器关闭 1表示第3路继电器打开
 usSRegHoldBuf[3] 值是0表示第一路磁保持继电器关闭 1表示第4路继电器打开

 20160907

 3、增加从机地址的配置，从硬件的薄码开关读取进来作为地址使用。
 增加2个宏分别支持2个不同的PCB板的驱动
 /*

#define GTLCS207_ER4GS_V1  使用PCB板为比特开发的 GTLCS207-ER4GS MB V1  的4路磁保持继电器驱动

*/
//#define GTLCS207_ER4GS_V1
#define GTRMS107_ER4AS_V10

 20160908

4、增加对按键参数的映射。分别放入寄存器如下。使得读取寄存器就可以获取外部开关量状态

		/*sample key data in idle*/
	usSRegHoldBuf[10] = P17;
	usSRegHoldBuf[11] = P16;
	usSRegHoldBuf[12] = P15;
	usSRegHoldBuf[13] = P14;
20160909


5、在MODBUS的基础上进行修改，把四路继电器的关闭，改用志红港这边的油锅计时器项目

要求：有一个数字电位器可以正反旋转，然后3位数码管数字来回的增加减少。停止2分钟后开始倒计时，倒计时为0时， 蜂鸣器报警即可。


		   P0端口对应数码管的数据总线
		   P00--a
		   p01--b
		   p02--c									
		   p03--d								   
		   p04--e									
		   p05--f
		   p06--g
		   p07--dp
		   P42--buzzer
		   p33--DP
		   p32--DN
		  


