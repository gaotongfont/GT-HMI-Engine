/****************************************************************************************
 * @file       QR_Code.c
 * @author     MusongLin
 * @version    v1.0
 * @date       2016-01-05
 * @brief      //
 *---------------------------------------------------------------------------------------
 * @attention
 *  
 * 
 ****************************************************************************************/

#include "gt_qrcode_src.h"

#if GT_CFG_ENABLE_QRCODE == 1

#define FLASH_MODE_DEF  0		//01: flash 模式, 00: RAM模式

//unsigned char QRDataBuf[4072];//存储最终排置的点阵数据
////volatile 
//unsigned char QRCodeDataBuf[3706];//(QR Code)用于存放计算结果的数据码 
//volatile 
// unsigned char versionN=0;					//获取的二维码的版本号

static unsigned char versionN = 0;				//内部定义, QR版本号
static unsigned char* QRDataBuf; 		//自行定义, 二维码图形数组
static unsigned char* QRCodeDataBuf;	//自行定义

int gt_qrcode_init(unsigned char version , unsigned char* qrdata_buff , unsigned char* qrcodedata_buff)
{
	if(NULL == qrdata_buff || NULL == qrcodedata_buff)
		return 0;

	QRDataBuf = qrdata_buff ;
	QRCodeDataBuf = qrcodedata_buff ;
	versionN = version ;
	return 1;
}

/*----------------------------------------------------------------------------------------------- 
 *-----------------------------------------------------------------------------------------------
 *                      纠错码算法部分
 *-----------------------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------------------*/
//获取指数对应的整数 
const unsigned char Exp2Int[256]={    //ExponentOfAlpha2Integer                                                                                                         
  1,  2,  4,  8, 16, 32, 64,128, 29, 58,116,232,205,135, 19, 38, 76,152, 45, 90,//20
180,117,234,201,143,  3,  6, 12, 24, 48, 96,192,157, 39, 78,156, 37, 74,148, 53,
106,212,181,119,238,193,159, 35, 70,140,  5, 10, 20, 40, 80,160, 93,186,105,210,//60                                                               
185,111,222,161, 95,190, 97,194,153, 47, 94,188,101,202,137, 15, 30, 60,120,240,
253,231,211,187,107,214,177,127,254,225,223,163, 91,182,113,226,217,175, 67,134,//100
 17, 34, 68,136, 13, 26, 52,104,208,189,103,206,129, 31, 62,124,248,237,199,147,
 59,118,236,197,151, 51,102,204,133, 23, 46, 92,184,109,218,169, 79,158, 33, 66,//140
132, 21, 42, 84,168, 77,154, 41, 82,164, 85,170, 73,146, 57,114,228,213,183,115,
230,209,191, 99,198,145, 63,126,252,229,215,179,123,246,241,255,227,219,171, 75,//180
150, 49, 98,196,149, 55,110,220,165, 87,174, 65,130, 25, 50,100,200,141,  7, 14,
 28, 56,112,224,221,167, 83,166, 81,162, 89,178,121,242,249,239,195,155, 43, 86,//220
172, 69,138,  9, 18, 36, 72,144, 61,122,244,245,247,243,251,235,203,139, 11, 22,//240
 44, 88,176,125,250,233,207,131, 27, 54,108,216,173, 71,142,  1
};

//获取整数对应的指数 
const unsigned char Int2Exp[256]={
  0,  0,  1, 25,  2, 50, 26,198,  3,223, 51,238, 27,104,199, 75,  4,100,224, 14,//20
 52,141,239,129, 28,193,105,248,200,  8, 76,113,  5,138,101, 47,225, 36, 15, 33,//40
 53,147,142,218,240, 18,130, 69, 29,181,194,125,106, 39,249,185,201,154,  9,120,//60
 77,228,114,166,  6,191,139, 98,102,221, 48,253,226,152, 37,179, 16,145, 34,136,//80
 54,208,148,206,143,150,219,189,241,210, 19, 92,131, 56, 70, 64, 30, 66,182,163,//100
195, 72,126,110,107, 58, 40, 84,250,133,186, 61,202, 94,155,159, 10, 21,121, 43,//120
 78,212,229,172,115,243,167, 87,  7,112,192,247,140,128, 99, 13,103, 74,222,237,//140
 49,197,254, 24,227,165,153,119, 38,184,180,124, 17, 68,146,217, 35, 32,137, 46,//160
 55, 63,209, 91,149,188,207,205,144,135,151,178,220,252,190, 97,242, 86,211,171,//180
 20, 42, 93,158,132, 60, 57, 83, 71,109, 65,162, 31, 45, 67,216,183,123,164,118,//200
196, 23, 73,236,127, 12,111,246,108,161, 59, 82, 41,157, 85,170,251, 96,134,177,//220
187,204, 62, 90,203, 89, 95,176,156,169,160, 81, 11,245, 22,235,122,117, 44,215,//240
 79,174,213,233,230,231,173,232,116,214,244,234,168, 80, 88,175
};
/* Number of Blocks in Group 1 */
const	unsigned char NubOfGroup1[160]={1,1,1,1,1,1,1,1,1,1,2,2,1,2,2,4,1,2,2,2,2,4,4,4,2,4,2,4,2,2,4,4,2,3,4,4,2,4,6,6,
	4,1,4,3,2,6,4,7,4,8,8,12,3,4,11,11,5,5,5,11,5,7,15,3,1,10,1,2,5,9,17,2,3,3,17,9,3,3,15,15,4,17,17,19,2,17,7,
	34,4,4,11,16,6,6,11,30,8,8,7,22,10,19,28,33,8,22,8,12,3,3,4,11,7,21,1,19,5,19,15,23,13,2,42,23,17,10,10,19,17,
	14,29,11,13,14,44,59,12,12,39,22,6,6,46,2,17,29,49,24,4,13,48,42,20,40,43,10,19,18,34,20};
/*Number of Data Codewords in Each of Group 1's Blocks */
const	unsigned char DataNubOfGroup1[160]={19,16,13, 9,34,28,22,16,55,44,17,13,80,32,24, 9,108,43,15,11,68,27,19,15,78,
	31,14,13,97,38,18,14,116,36,16,12,68,43,19,15,81,50,22,12,92,36,20,14,107,37,20,11,115,40,16,12,87,41,24,12,98,
	45,19,15,107,46,22,14,120,43,22,14,113,44,21,13,107,41,24,15,116,42,22,16,111,46,24,13,121,47,24,15,117,45,24,
	16,106,47,24,15,114,46,22,16,122,45,23,15,117,45,24,15,116,45,23,15,115,47,24,15,115,46,24,15,115,46,24,15,115,
	46,24,15,115,46,24,16,121,47,24,15,121,47,24,15,122,46,24,15,122,46,24,15,117,47,24,15,118,47,24,15,};
/* Number of Blocks in Group 2 */
const	unsigned char NubOfGroup2[160]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,4,1,0,2,2,2,0,2,4,4,2,1,2,2,0,
	4,4,8,2,2,6,4,0,1,4,4,1,5,5,5,1,5,7,7,1,3,2,13,5,1,15,17,1,4,1,19,4,11,4,16,5,13,5,10,4,0,6,6,7,0,16,0,5,14,14,
	14,4,14,16,2,4,13,22,13,2,4,6,4,4,3,26,28,10,23,31,31,7,7,37,26,10,10,25,25,3,29,1,28,0,23,35,35,1,21,19,46,6,
	23,7,1,7,26,14,41,14,34,10,64,4,14,10,46,18,32,14,32,4,7,22,67,6,31,34,61,}; 
/*Number of Data Codewords in Each of Group 2's Blocks */
//const	unsigned char DataNubOfGroup2[160]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,12,0,0,0,0,0,0,15,14,0,39,19,15,0,
//	37,17,13,69,44,20,16,0,51,23,13,93,37,21,15,0,38,21,12,116,41,17,13,88,42,25,13,99,46,20,16,108,47,23,15,121,
//	44,23,15,114,45,22,14,108,42,25,16,117,0,23,17,112,0,25,0,122,48,25,16,118,46,25,17,107,48,25,16,115,47,23,17,
//	123,46,24,16,118,46,25,16,117,46,24,16,116,48,25,16,116,47,25,16,0,47,25,16,116,47,25,16,116,47,25,17,122,48,
//	25,16,122,48,25,16,123,47,25,16,123,47,25,16,118,48,25,16,119,48,25,16};

//Generator polynomial for N error correction code words:
const unsigned char  ECCodeWords[259]={ 
		0, 87,229,146,149,238,102, 21,																//1_L,  7个纠错码 (8 byte) 
		0,251, 67, 46, 61,118, 70, 64, 94, 32, 45,										//1_M, 10个纠错码 (11byte) 
		0, 74,152,176,100, 86,100,106,104,130,218,206,140, 78,				//1_Q, 13个纠错码 (14byte) 
		0,  8,183, 61, 91,202, 37, 51, 58, 58,237,140,124,  5, 99,105,//3_L, 15个纠错码 (16byte) 
		0,120,104,107,109,102,161, 76,  3, 91,191,147,169,182,194,225,
	120,																														// 2_M ,16个纠错码 (17byte)
		0, 43,139,206, 78, 43,239,123,206,214,147, 24, 99,150, 39,243,
	163,136,																												// 1_H, 17个纠错码 (18byte)
		0,215,234,158, 94,184, 97,118,170, 79,187,152,148,252,179,  5,
 	 98, 96,153,																										// 3_Q ,18个纠错码 (19byte)
		0, 17, 60, 79, 50, 61,163, 26,187,202,180,221,225, 83,239,156,
	164,212,212,188,190,																						// 4_L ,20个纠错码 
		0,210,171,247,242, 93,230, 14,109,221, 53,200, 74,  8,172, 98,
	 80,219,134,160,105,165,231,																		// 2_Q ,22个纠错码
		0,229,121,135, 48,211,117,251,126,159,180,169,152,192,226,228,
	218,111,  0,117,232, 87, 96,227, 21,														// 5_M ,24个纠错码
		0,173,125,158,  2,103,182,118, 17,145,201,111, 28,165, 53,161,
	 21,245,142, 13,102, 48,227,153,145,218, 70,										// 3_M ,26个纠错码 
		0,168,223,200,104,224,234,108,180,110,190,195,147,205, 27,232,
	201, 21, 43,245, 87, 42,195,212,119,242, 37,  9,123,						// 2_H,28纠错码, (29byte)		
		0, 41,173,145,152,216, 31,179,182, 50, 48,110, 86,239, 96,222,
	125, 42,173,226,193,224,130,156, 37,251,216,238, 40,192,180,		//30纠错码, (31byte) 	 
};


/*编码部分 -----------------------------*/ 
//unsigned char bitdata[3200];
#define bitdata  QRDataBuf    			//用于临时存放转换的UTF-8编码
unsigned char ECDataBufTemp[2430];	//用于临时存放所有blocks组的纠错码计算结果
/* 纠错码部分 -----------------------------------------------------------------------------------------------------*/
//数组NubOfECCode用于存储版本和纠错等级对应的纠错码数[EC Codewords Per Block]
//参考链接(http://www.thonky.com/qr-code-tutorial/error-correction-table)
const unsigned char NubOfECCode[160]={  7,10,13,17, 10,16,22,28, 15,26,18,22, 20,18,26,16, 26,24,18,22, 18,16,24,28,
	/*version 7*/20,18,18,26, 24,22,22,26, 30,22,20,24, 18,26,24,28, 20,30,28,24, 24,22,26,28, 26,22,24,22,
	/*version14*/30,24,20,24, 22,24,30,24, 24,28,24,30, 28,28,28,28, 30,26,28,28, 28,26,26,26, 28,26,30,28,
	/*version21*/28,26,28,30, 28,28,30,24, 30,28,30,30, 30,28,30,30, 26,28,30,30, 28,28,28,30, 30,28,30,30,
	/*version28*/30,28,30,30, 30,28,30,30, 30,28,30,30, 30,28,30,30, 30,28,30,30, 30,28,30,30, 30,28,30,30,
	/*version35*/30,28,30,30, 30,28,30,30, 30,28,30,30, 30,28,30,30, 30,28,30,30, 30,28,30,30
};
//数组TotalNubOfDat用于存储版本和纠错等级对应的数据码字节数
const unsigned short TotalNubOfDat[160]={
	19,16,13, 9,        34,28,22,16,         55,44,34,26,         80,64,48,36,         108,86,62,46,       136,108,76,60,
	156,124, 88, 66,    194,154,110, 86,     232,182,132,100,     274,216,154,122,     324,254,180,140,    370,290,206,158, 
	428,334,244,180,    461,365,261,197,     523,415,295,223,     589,453,325,253,     647,507,367,283,    721,563,397,313,
	795,627,445,341,    861,669,485,385,     932,714,512,406,     1006,782,568,442,    1094,860,614,464,   1174,914,664,514,
	1276,1000,718,538,  1370,1062,754,596,   1468,1128,808,628,   1531,1193,871,661,   1631,1267,911,701,  1735,1373,985,745,
	1843,1455,1033,793, 1955,1541,1115,845,  2071,1631,1171,901,  2191,1725,1231,961,  2306,1812,1286,986, 2434,1914,1354,1054,
	2566,1992,1426,1096, 2702,2102,1502,1142, 2812,2216,1582,1222,2956,2334,1666,1276
};
/* 纠错码计算函数 ----------------------------------------------------------------------------------------*/	
//NubOfDat: 每一个blocks的对应的数据数;  ECBuf：block对应的数据; ECNub：计算结果的纠错码数 
void EC_Coding(unsigned short NubOfDat,/*unsigned char *ECBuf,*/unsigned char ECNub,
	unsigned short Nub,unsigned char *ECDataBuf)
{
	unsigned char ECCodeWordsBUF[31];
	unsigned char Integer1[123];
	unsigned short i=0;
	unsigned char AlphaTemp=0,temp=0;
	int j=0; 
	/*---------------------------------------------------------------*/
	if(ECNub== 7)temp=  0;
	else if(ECNub==10)temp=  8;//8;//8
	else if(ECNub==13)temp= 19;//8+11;//19
	else if(ECNub==15)temp= 33;//8+11+14;//33
	else if(ECNub==16)temp= 49;//8+11+14+16;//49
	else if(ECNub==17)temp= 66;//8+11+14+16+17;//66
	else if(ECNub==18)temp= 84;//8+11+14+16+17+18;//84
	else if(ECNub==20)temp=103;//8+11+14+16+17+18+19;//103
	else if(ECNub==22)temp=124;//8+11+14+16+17+18+19+21;//124
	else if(ECNub==24)temp=147;//8+11+14+16+17+18+19+21+23;//147
	else if(ECNub==26)temp=172;//8+11+14+16+17+18+19+21+23+25;//172
	else if(ECNub==28)temp=199;//8+11+14+16+17+18+19+21+23+25+27;//199 
	else if(ECNub==30)temp=228;//8+11+14+16+17+18+19+21+23+25+27+29;//228

//	memset(ECCodeWordsBUF,0,sizeof(ECCodeWordsBUF));
//	memset(Integer1,0,sizeof(Integer1));
#if FLASH_MODE_DEF
//	/* flash mode --------------------------------------*/
	r_dat_bat(1159+temp,ECNub+1,ECCodeWordsBUF);
#else
//	/* ram mode ----------------------------------------*/
	for(i=0;i<(ECNub+1);i++){
		ECCodeWordsBUF[i]=ECCodeWords[temp+i];
	}
#endif

	for(j=0;j<NubOfDat;j++){
		if(NubOfDat>ECNub){
			memset(Integer1,0,sizeof(Integer1));
			if(j<(NubOfDat-ECNub)){
					for(i=0;i<NubOfDat;i++){
						if(i<(ECNub+1)){
							AlphaTemp = (Int2Exp[ECDataBuf[0]] + ECCodeWordsBUF[i])%255;//步骤a指数的结果 
							Integer1[i] = Exp2Int[AlphaTemp]^ECDataBuf[i];}	//步骤b整数的结果
						else {
							Integer1[i]=0^ECDataBuf[i];											//步骤b整数的结果 
						}
					}
					/* 16-04-20--17:00 ##########################################*/
					if(Integer1[0]==0){
						for(i=0;i<(NubOfDat-1);i++)ECDataBuf[i]=Integer1[i+1]; 
						for(i=0;i<(j+1);i++)ECDataBuf[(NubOfDat-1)-i]=0;
					}
					else{
						for(i=0;i<NubOfDat;i++)ECDataBuf[i]=Integer1[i];j-=1;
					}
			}
			else{
				for(i=0;i<(ECNub+1);i++){
						AlphaTemp = (Int2Exp[ECDataBuf[0]]+ECCodeWordsBUF[i])%255;
						Integer1[i] = Exp2Int[AlphaTemp]^ECDataBuf[i];}
				
					/* 16-04-20--17:00 ##########################################*/
					if(Integer1[0]==0){
						for(i=0;i<ECNub;i++)ECDataBuf[i]=Integer1[i+1]; 
					}
					else{
						for(i=0;i<(ECNub);i++)ECDataBuf[i]=Integer1[i]; j-=1;
					}
			}
		}
		else{
				memset(Integer1,0,sizeof(Integer1));
				for(i=0;i<(ECNub+1);i++){
					AlphaTemp = (Int2Exp[ECDataBuf[0]]+ECCodeWordsBUF[i])%255;
					Integer1[i] = Exp2Int[AlphaTemp]^ECDataBuf[i];}
				/* 16-04-20--17:00 ##########################################*/
				if(Integer1[0]==0){
					for(i=0;i<ECNub;i++)ECDataBuf[i]=Integer1[i+1];
				}
				else{
					for(i=0;i<(ECNub);i++)ECDataBuf[i]=Integer1[i];
					j-=1;
				}
		}
	}
	/*------------------------------------------------------------*/
	for(i=0;i<ECNub;i++)
		ECDataBufTemp[i+Nub]=ECDataBuf[i];
}

/*-------------------------------------------------------------------------------------------------
 *  纠错码编码函数 
 * 参数 : version版本号，ECLevel纠错等级，NubOfDat数据字节数 
 *-------------------------------------------------------------------------------------------------*/	
unsigned char ErrorCorrectionCoding(unsigned char version,unsigned char ECLevel,unsigned short NubOfDat)
{
	unsigned char ECDataTemp[123];
	unsigned char j=0,n=0;
	unsigned char VECNub=0,ECNub=0;
	unsigned short i=0,k=0,NubTemp=0;
	unsigned char NUBGroup1=0,NUBGroup2=0,DataNUBGroup1=0,DataNUBGroup2=0;
	VECNub=4*version-4+ECLevel;
#if FLASH_MODE_DEF
//	/* flash mode ----------------------------------------------*/
	r_dat_bat(0x00+VECNub,1,&NUBGroup1);		//Delay(0xf);
	r_dat_bat(160+VECNub,1,&DataNUBGroup1);	//Delay(0xf);
	r_dat_bat(320+VECNub,1,&NUBGroup2); 
	r_dat_bat(480+4*version-4+ECLevel,1,&ECNub);
#else
	/* ram mode --------------------------------------------------*/
	NUBGroup1=NubOfGroup1[VECNub];//板块1的组数 
	NUBGroup2=NubOfGroup2[VECNub];//板块2的组数 
	DataNUBGroup1=DataNubOfGroup1[VECNub];//板块1每个组的数据数 
	DataNUBGroup2=DataNUBGroup1+1;//DataNubOfGroup2[VECNub];//板块2每个组的数据数 
	ECNub = (unsigned char)NubOfECCode[4*version-4+ECLevel];//纠错码字节数 
#endif

	/*判断纠错组的数量 (if >1)------------------------------------*/
	if((NUBGroup1+NUBGroup2)!=1)
	{
			for(k=0;k<NUBGroup1;k++){
				memset(ECDataTemp,0,sizeof(ECDataTemp));
				for(i=0;i<DataNUBGroup1;i++)
					ECDataTemp[i]=QRCodeDataBuf[i+k*DataNUBGroup1];
				EC_Coding(DataNUBGroup1/*,ECDataBufTemp*/,ECNub,k*ECNub,ECDataTemp);
			}
			for(k=0;k<NUBGroup2;k++){
				memset(ECDataTemp,0,sizeof(ECDataTemp));
				for(i=0;i<DataNUBGroup2;i++)
					ECDataTemp[i]=QRCodeDataBuf[i+NUBGroup1*DataNUBGroup1+k*DataNUBGroup2];
				EC_Coding(DataNUBGroup2/*,ECDataBufTemp*/,ECNub,(k+NUBGroup1)*ECNub,ECDataTemp);
			}
			
			/* 穿插放置 (数据码部分)----------------------------------*/ 
			NubTemp = NUBGroup1*DataNUBGroup1+NUBGroup2*DataNUBGroup2;
			k=0;n=0;
//			NubTemp = TotalNubOfDat[4*version-4+ECLevel];//版本&纠错等级-对应的数据码字节数
			for(i=0;i<NubTemp;){
				if(i/(NUBGroup1+NUBGroup2)<DataNUBGroup1)
				for(j=0;j<NUBGroup1;j++){
					QRDataBuf[i++]=QRCodeDataBuf[k+j*DataNUBGroup1];
					if(j==(NUBGroup1-1))k++;
				}
				for(j=0;j<NUBGroup2;j++){
					QRDataBuf[i++]=QRCodeDataBuf[NUBGroup1*DataNUBGroup1+j*DataNUBGroup2+n];
					if(j==(NUBGroup2-1))n++;
				}
			}
			for(i=0;i<NubTemp;i++)
				QRCodeDataBuf[i]=QRDataBuf[i];
			/* 纠错码穿插放置 ----------------------------------------*/
			for(k=0,i=0;i<ECNub*(NUBGroup1+NUBGroup2);k++){
				for(j=0;j<(NUBGroup1+NUBGroup2);j++,i++)
					QRCodeDataBuf[NubTemp+i]=ECDataBufTemp[j*ECNub+k];
			}
			return 1;
	}
	/* 纠错码组数为1时 --------------------------------------------*/
//	memset(ECDataTemp,0,sizeof(ECDataTemp));
	for(i=0;i<NubOfDat;i++)
		ECDataTemp[i]=QRCodeDataBuf[i];
	EC_Coding(NubOfDat/*,QRCodeDataBuf*/,ECNub,NubOfDat,ECDataTemp);
	return 1;
}

/*-------------------------------------------------------------------------------------------------
 *  QRcode 编码函数
 * 参数 : CodeMode编码模式：Mode_Unicode/Mode_GBK/Mode_KSC5601/Mode_SHIFT_JIS
 * 参数 : ECLevel: 纠错等级
 *-------------------------------------------------------------------------------------------------*/	 
unsigned short DataEncoding(unsigned char CodeMode,unsigned char ECLevel,unsigned char *str,
	long unsigned int length)
{
	unsigned char GBcode[2];
	unsigned long Address=0,BaseAddr=0;
	unsigned char  Unicode[2];
	unsigned short i=0,j=0,k=0,NubOfFormat=0;
	unsigned long  UnicodeTemp=0;
	unsigned short NubOfDat=16; //存储数据有效值总数 
//	memset(bitdata,0,sizeof(bitdata));
	/***************************************************************/	
	/* 将输入的字符数据转换成UTF-8 -------------------------------*/
	for(k=0;k<length;)
	{
#if 0
		if(CodeMode==Mode_Unicode)
		{
			/* unicode ---------------------------------------------S*/
			UnicodeTemp = ((unsigned long)str[k])<<8 |((unsigned long)str[k+1]);
			if(UnicodeTemp<=0x07F){
				bitdata[NubOfFormat]=(unsigned char)str[k+1];
				k+=2;NubOfFormat+=1;
			}
			else if(UnicodeTemp>=0x080 && UnicodeTemp<=0x07FF){ 
				bitdata[NubOfFormat]	=(unsigned char)(0xc0|((unsigned char)str[k+1]>>6)|(((unsigned char)str[k]&0x07)<<2));
				bitdata[NubOfFormat+1]=(unsigned char)(0x80|((unsigned char)str[k+1]&0x3f));
				k+=2;NubOfFormat+=2;
			}
			else if(UnicodeTemp>=0x0800 && UnicodeTemp<=0xFFFF){ 
				bitdata[NubOfFormat]	=(unsigned char)(0xe0|((unsigned char)str[k]>>4));
				bitdata[NubOfFormat+1]=(unsigned char)(0x80|(((unsigned char)str[k]&0x0f)<<2)|(((unsigned char)str[k+1]>>6)&0x03));
				bitdata[NubOfFormat+2]=(unsigned char)(0x80|((unsigned char)str[k+1]&0x3f));
				k+=2;NubOfFormat+=3;
			}
		}
		else if(CodeMode==Mode_GBK ||  CodeMode==Mode_KSC5601 || CodeMode==Mode_SHIFT_JIS)
		{
			//ASCII码部分 
			if(str[k]<=0x7f){ // if(str[k]>=0x20 & str[k]<=0x7f){
				bitdata[NubOfFormat]=str[k];
				k++;NubOfFormat++;
			}
			else if(CodeMode==Mode_GBK){ 				
				if(str[k]>=0x81 && str[k]<=0xFE && str[k+1]>=0x40 && str[k+1]<=0xFE){
					memset(Unicode,0,sizeof(Unicode));
					BaseAddr=26762UL;Address=0;
					GBcode[0]=str[k];
					GBcode[1]=str[k+1];
					if(str[k+1]>=0x40 && str[k+1]<=0x7E){ 
							Address =((GBcode[0]-0x81)*190+(GBcode[1]-0x40))*2+BaseAddr;  
					}
					else if(str[k+1]>=80 && str[k+1]<=0xFE){ 
							Address =((GBcode[0]-0x81)*190+(GBcode[1]-0x41))*2+BaseAddr;  
					}
					r_dat_bat(Address,2,Unicode);
					
					bitdata[NubOfFormat]  =  (unsigned char)(0xe0|((unsigned char)Unicode[0]>>4));
					bitdata[NubOfFormat+1]=  (unsigned char)(0x80|((unsigned char)(Unicode[0]&0x0f)<<2)|(((unsigned char)Unicode[1]>>6)&0x03));
					bitdata[NubOfFormat+2]=  (unsigned char)(0x80|((unsigned char)Unicode[1]&0x3f));
					k+=2;NubOfFormat+=3;
				}
				else {k+=2;continue;}
			}
			else if(CodeMode==Mode_KSC5601)
			{
				if(str[k]>=0xA1&&str[k]<=0xC8&&str[k+1]>=0xA1)
				{
					memset(Unicode,0,2);
					UnicodeTemp=0;
					BaseAddr=1418UL;Address=0;
					if(str[k]>=0xA1&&str[k]<0xB0&&str[k+1]>=0xA1)
						Address =((unsigned long)str[k]-0xA1)*188UL+((unsigned long)str[k+1]-0xA1)*2+ BaseAddr;
					else if(str[k]>=0xB0&&str[k]<=0xC8&&str[k+1]>=0xA1)
						Address =((unsigned long)str[k]-0xA1)*188UL+((unsigned long)str[k+1]-0xA1)*2-564UL+ BaseAddr;
					else {k+=2;continue;}
					r_dat_bat(Address,2,Unicode);
					UnicodeTemp = ((unsigned long)Unicode[0])<<8 |((unsigned long)Unicode[1]);
					if(UnicodeTemp>=0x080 && UnicodeTemp<=0x07FF){
						bitdata[NubOfFormat]=0xc0|(Unicode[1]>>6)|((Unicode[0]&0x07)<<2);
						bitdata[NubOfFormat+1]=0x80|(Unicode[1]&0x3f);
						k+=2;NubOfFormat+=2;
					}
					else if(UnicodeTemp>=0x0800 && UnicodeTemp<=0xFFFF){
						bitdata[NubOfFormat]=0xe0|(Unicode[0]>>4);
						bitdata[NubOfFormat+1]=0x80|((Unicode[0]&0x0f)<<2)|((Unicode[1]>>6)&0x03);
						bitdata[NubOfFormat+2]=0x80|(Unicode[1]&0x3f);
						k+=2;NubOfFormat+=3;
					}
					else return 0;
				}
			}
			else if(CodeMode==Mode_SHIFT_JIS)
			{
					/*---------------------------------------------------------------------------------------------*/
					memset(Unicode,0,2);
					UnicodeTemp=0;
					BaseAddr=8374UL;Address=0;
					if(str[k]>=0x81&&str[k]<=0x9F){
						if(str[k+1]>=0x40&&str[k+1]<=0x7E)
							Address =((unsigned long)str[k]-0x81)*376UL+((unsigned long)str[k+1]-0x40)*2+ BaseAddr;
						if(str[k+1]>=0x80&&str[k+1]<=0xFC)
							Address =((unsigned long)str[k]-0x81)*376UL+((unsigned long)str[k+1]-0x40)*2-2+ BaseAddr;
					}
					else if(str[k]>=0xE0&&str[k]<=0xEF){
						if(str[k+1]>=0x40&&str[k+1]<=0x7E)
							Address =((unsigned long)str[k]-0xE0)*376UL+((unsigned long)str[k+1]-0x40)*2+11656+ BaseAddr;
						if(str[k+1]>=0x80&&str[k+1]<=0xFC)
							Address =((unsigned long)str[k]-0xE0)*376UL+((unsigned long)str[k+1]-0x40)*2+11654+ BaseAddr;
					}
					else {k+=2;continue;}
					r_dat_bat(Address,2,Unicode);
					/*---------------------------------------------------------------------------------------------*/
					UnicodeTemp = ((unsigned long)Unicode[0])<<8 |((unsigned long)Unicode[1]);
					if(UnicodeTemp>=0x080 && UnicodeTemp<=0x07FF){
						bitdata[NubOfFormat]=0xc0|(Unicode[1]>>6)|((Unicode[0]&0x07)<<2);
						bitdata[NubOfFormat+1]=0x80|(Unicode[1]&0x3f);
						k+=2;NubOfFormat+=2;
					}
					else if(UnicodeTemp>=0x0800 && UnicodeTemp<=0xFFFF){
						bitdata[NubOfFormat]=0xe0|(Unicode[0]>>4);
						bitdata[NubOfFormat+1]=0x80|((Unicode[0]&0x0f)<<2)|((Unicode[1]>>6)&0x03);
						bitdata[NubOfFormat+2]=0x80|(Unicode[1]&0x3f);
						k+=2;NubOfFormat+=3;
					}
					else if(UnicodeTemp==0x0000){
						k+=2;continue;
					}
					else return 0;
			}
			else return 0;
		}	
#else
		if(CodeMode==Mode_Unicode)
		{
			/* unicode ---------------------------------------------S*/
			UnicodeTemp = str[k];
			if(UnicodeTemp<=0x07F){
				bitdata[NubOfFormat]=(unsigned char)str[k];
				k++;
				NubOfFormat+=1;
			}
			else
			{
				UnicodeTemp = ((unsigned long)str[k])<<8 |((unsigned long)str[k+1]);
				if(UnicodeTemp>=0x080 && UnicodeTemp<=0x07FF){ 
					bitdata[NubOfFormat]	=(unsigned char)(0xc0|((unsigned char)str[k+1]>>6)|(((unsigned char)str[k]&0x07)<<2));
					bitdata[NubOfFormat+1]=(unsigned char)(0x80|((unsigned char)str[k+1]&0x3f));
					k+=2;
					NubOfFormat+=2;
				}
				else if(UnicodeTemp>=0x0800 && UnicodeTemp<=0xFFFF){ 
					bitdata[NubOfFormat]	=(unsigned char)(0xe0|((unsigned char)str[k]>>4));
					bitdata[NubOfFormat+1]=(unsigned char)(0x80|(((unsigned char)str[k]&0x0f)<<2)|(((unsigned char)str[k+1]>>6)&0x03));
					bitdata[NubOfFormat+2]=(unsigned char)(0x80|((unsigned char)str[k+1]&0x3f));
					k+=2;
					NubOfFormat+=3;
				}
				else
				{
					return 0;
				}
			}
		}
		else
		{
			return 0;
		}
#endif
	}
	/*判断输入字符数据是否超出 该版本的最大容量 ------------------*/
	if((NubOfFormat==0)||(NubOfFormat+3)>TotalNubOfDat[156+ECLevel])return 0;//输入字符超过版本最大容量则return 0
	for(i=0;i<40;i++)
	{
			if((NubOfFormat+3)<TotalNubOfDat[i*4+ECLevel])
			{
				//versionN=i+1;
				break;
			}
	} 
	NubOfDat = TotalNubOfDat[4*versionN-4+ECLevel];
//	memset(QRCodeDataBuf,0,sizeof(QRCodeDataBuf));
	if(versionN<10){//10版本以下的字符计数指示符的位数是8bit
			/* 数据码部分 --------------------------------------------*/
			QRCodeDataBuf[0] = (0x40)|(NubOfFormat>>4);
			QRCodeDataBuf[1] = (NubOfFormat<<4)|(bitdata[0]>>4);
			for(i=0;i<(NubOfFormat-1);i++){
				QRCodeDataBuf[i+2] = (bitdata[i]<<4)|(bitdata[i+1]>>4);
			}
			/* 补零码 ------------------------------------------------*/
			QRCodeDataBuf[NubOfFormat+1]=bitdata[NubOfFormat-1]<<4;
			NubOfFormat+=2;
	}
	else {//10版本及以上的字符计数指示符的位数是16bit
			/* 数据码部分 --------------------------------------------*/
			QRCodeDataBuf[0] = (0x40)|(NubOfFormat>>12);
			QRCodeDataBuf[1] = (unsigned char)(NubOfFormat>>4);
			QRCodeDataBuf[2] = (NubOfFormat<<4)|(bitdata[0]>>4);
			for(i=0;i<(NubOfFormat-1);i++){
				QRCodeDataBuf[i+3] = (bitdata[i]<<4)|(bitdata[i+1]>>4);
			}
			/* 补零码 ------------------------------------------------*/
			QRCodeDataBuf[NubOfFormat+2] = bitdata[NubOfFormat-1]<<4;
			NubOfFormat+=3;
	}
	/* 补码部分 --------------------------------------------------*/
	for(j=NubOfFormat;j<(NubOfDat-1);j+=2,NubOfFormat+=2){
		QRCodeDataBuf[j]=236;
		QRCodeDataBuf[j+1]=17;
	}
	if(NubOfFormat!=NubOfDat)QRCodeDataBuf[NubOfDat-1]=236;

	return NubOfDat;
}


/*------------------------------------------------------------------------------------------------------
 *  二维码获取函数
 *-----------------------------------------------------------------------------------------------------*/
unsigned char QRBinMode(unsigned char version,unsigned char ECLevel,unsigned char MaskNub) ;
//
unsigned char gt_qrcode_get(unsigned char CodeMode,unsigned char ECLevel,unsigned char MaskNub,
	unsigned char *str,long unsigned int length)
{
	unsigned short NubOfDat=16;
	// 
	if(NULL == QRDataBuf || NULL == QRCodeDataBuf)
		return 0;

	if( ECLevel>3 ||MaskNub>7)return 0;
	/* 数据码处理部分 ------------------------------------*/
	NubOfDat = DataEncoding(CodeMode,ECLevel,str,length);
	if(NubOfDat==0)return 0;
	/* 纠错码计算部分 ------------------------------------*/
	ErrorCorrectionCoding(versionN,ECLevel,NubOfDat);
	/* 数据排置处理部分 ----------------------------------*/ 
	QRBinMode(versionN,ECLevel,MaskNub);
	return 1;
}


/*-----------------------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------------------
 *                          排置处理及掩膜部分
 *-----------------------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------------------*/

static unsigned short const QRDataByte[40]={ //存储QR code 字节数 
	26,  44,  70, 100, 134, 172, 196, 242, 292, 346, 404, 466, 532, 581, 655, 733,
 815, 901, 991,1085,1156,1258,1364,1474,1588,1706,1828,1921,2051,2185,2323,2465,
2611,2761,2876,3034,3196,3362,3532,3706
};
/* List of all Format Information Strings 纠错等级和掩码图案的选择对应的15Bit格式信息码 */
static unsigned short const ListOfFI[32]={ //格式信息
	0x77C4,0x72F3,0x7DAA,0x789D,0x662F,0x6318,0x6C41,0x6976,	//ECC Level  'L'-0/1/2/3/4/5/6/7 
	0x5412,0x5125,0x5E7C,0x5B4B,0x45F9,0x40CE,0x4F97,0x4AA0,	//ECC Level  'M'-0/1/2/3/4/5/6/7 
	0x355F,0x3068,0x3F31,0x3A06,0x24B4,0x2183,0x2EDA,0x2BED,	//ECC Level  'Q'-0/1/2/3/4/5/6/7 
	0x1689,0x13BE,0x1CE7,0x19D0,0x0762,0x0255,0x0D0C,0x083B		//ECC Level  'H'-0/1/2/3/4/5/6/7 
};
static unsigned long const ListOfFI7[34]={//版本信息 version 7-40
	0x7C94 ,0x85BC ,0x9A99 ,0xA4D3 ,0xBBF6 ,0xC762 ,0xD847 ,0xE60D,
	0xF928 ,0x10B78,0x1145D,0x12A17,0x13532,0x149A6,0x15683,0x168C9,
	0x177EC,0x18EC4,0x191E1,0x1AFAB,0x1B08E,0x1CC1A,0x1D33F,0x1ED75,
	0x1F250,0x209D5,0x216F0,0x228BA,0x2379F,0x24B0B,0x2542E,0x26A64,
	0x27541,0x28C69
};

/* 对齐图形坐标 --------------------------------------------------------*/
const unsigned char ModuleXY[39][7]={ //对齐图形坐标 
 {6,18, 0, 0, 0, 0, 0},{6,22, 0, 0, 0, 0, 0},{6,26, 0, 0, 0, 0, 0},{6,30, 0, 0, 0, 0, 0},{6,34, 0, 0, 0, 0, 0},
 {6,22,38, 0, 0, 0, 0},{6,24,42, 0, 0, 0, 0},{6,26,46, 0, 0, 0, 0},{6,28,50, 0, 0, 0, 0},{6,30,54, 0, 0, 0, 0},
 {6,32,58, 0, 0, 0, 0},{6,34,62, 0, 0, 0, 0},{6,26,46,66, 0, 0, 0},{6,26,48,70, 0, 0, 0},{6,26,50,74, 0, 0, 0},
 {6,30,54,78, 0, 0, 0},{6,30,56,82, 0, 0, 0},{6,30,58,86, 0, 0, 0},{6,34,62,90, 0, 0, 0},{6,28,50,72,94, 0, 0},
 {6,26,50,74,98, 0, 0},{6,30,54,78,102,0, 0},{6,28,54,80,106,0, 0},{6,32,58,84,110,0, 0},{6,30,58,86,114,0, 0},
 {6,34,62,90,118,0, 0},{6,26,50,74,98,122,0},{6,30,54,78,102,126,0},{6,26,52,78,104,130, 0},{6,30,56,82,108,134, 0},
 {6,34,60,86,112,138, 0},{6,30,58,86,114,142, 0},{6,34,62,90,118,146, 0},{6,30,54,78,102,126,150},{6,24,50,76,102,128,154},
 {6,28,54,80,106,132,158},{6,32,58,84,110,136,162},{6,26,54,82,110,138,166},{6,30,58,86,114,142,170},
};

unsigned char QRDataMasking(unsigned char version,unsigned char MaskNub,unsigned char *ModelXY);
void PositionDetectionPatterns(unsigned char version);
unsigned char FormatInformation(unsigned char version,unsigned char n);
unsigned char QRBinMode(unsigned char version,unsigned char ECLevel,unsigned char MaskNub) 
{
	unsigned char i=0;
	unsigned char fomatVersion=0;//存储格式信息数组的第几个
	unsigned char ModuleXYValue[7]={0};
	if(version==1)memset(ModuleXYValue,0,sizeof(ModuleXYValue));
	else{
		#if FLASH_MODE_DEF
		/* flash MODE ---------------------------*/
		r_dat_bat(886+(version-2)*7,7,ModuleXYValue);//获取对齐图形点坐标
		#else
		/* RAM MODE -----------------------------*/
		for(i=0;i<7;i++)
		{
			ModuleXYValue[i]=ModuleXY[version-2][i];
		}
		#endif
	}

	memset(QRDataBuf,0,sizeof(QRDataBuf));
	fomatVersion=ECLevel*8+MaskNub;
	PositionDetectionPatterns(version);						//加入定位点
	FormatInformation(version,fomatVersion);			//加入版本信息及格式信息
	QRDataMasking(version,MaskNub,ModuleXYValue); //加入最终数据及进行掩模处理

	return 1;
}

/* position detection patterns(加入定位图形) --------------------------------------------------*/
void PositionDetectionPatterns(unsigned char version)
{
	int x=6,y=0;
	unsigned char port=0;	
	unsigned char PositionDetectionData[7]={0x7f,0x41,0x5d,0x5d,0x5d,0x41,0x7f};
	unsigned char w=0,i=0,hbyte=0;
	w = 4*(version-1)+21;
	hbyte = (w+7)/8;
	
	/* Upper left ------------------------------------*/
	for(i=0;i<7;i++){
		QRDataBuf[i]=PositionDetectionData[i]; 
	}
	/* upper right -----------------------------------*/
	for(i=0;i<7;i++){
		QRDataBuf[w-7+i]=PositionDetectionData[i];
	}
	/* left bottom. ----------------------------------*/
	for(i=0;i<7;i++){
		QRDataBuf[(hbyte-1)*w+i] |= PositionDetectionData[i]>>(hbyte*8-w-1);
		QRDataBuf[(hbyte-2)*w+i] |= PositionDetectionData[i]<<(9-(hbyte*8-w));
	}
	
	/* Timing Pattern --------------------------------*/
	x=6;y=w;
	while(y--){
		if((y>=8 && y<=(w-9)&& (y-8)%2==0)){
			port=(unsigned char)y;
			QRDataBuf[w*(y/8)+x]|=0x01<<(port%8);
		}
	}
	y=6;x=w;
	while(x--){
		if((x>=8 && x<=(w-9)&& (x-8)%2==0)){
			port=(unsigned char)y;
			QRDataBuf[w*(y/8)+x]|=0x01<<(port%8);
		}
	}
	
}

/* ADD format information(加入格式信息) -------------------------------------------------------------*/
unsigned char FormatInformation(unsigned char version,unsigned char n)
{
	unsigned char i=0,t=0,QRWidth=0,hbyte=0; 
	unsigned long j=0x0001;
//	unsigned char DataBuf[3];
	unsigned short FormatInformationV=0;
	unsigned long VersionInformationV=0;
	QRWidth =4*version+17;
	hbyte = (QRWidth+7)/8;
	if(version>6)
	{
		#if FLASH_MODE_DEF
		/* flash读数据 */
		memset(DataBuf,0,sizeof(DataBuf));
		r_dat_bat(704+(version-7)*3,3,DataBuf);
		VersionInformationV=DataBuf[0]<<16 |DataBuf[1]<<8|DataBuf[2];
		#else
		/* RAM获取数据 */
		VersionInformationV = ListOfFI7[version-7];
		#endif
		
		for(t=0;t<6;t++)
		 for(i=0;i<3;i++){
			if(VersionInformationV&j){
				QRDataBuf[QRWidth-11+i]|=(0x01<<t);
				QRDataBuf[QRWidth*((QRWidth-11+i)/8)+t]|=(0x01<<((QRWidth-11+i)%8));
			}j<<=1;
		}
	}
	/* format information ----------------------------*/
	#if FLASH_MODE_DEF
	/* flash读数据 */
	memset(DataBuf,0,sizeof(DataBuf));
	r_dat_bat(640+n*2,2,DataBuf);
	FormatInformationV=(unsigned short)DataBuf[0]<<8 |DataBuf[1];
	#else
	/* RAM获取数据 */
	FormatInformationV=ListOfFI[n];
	#endif

	/* Upper left ------------------------------------*/
	QRDataBuf[8] |= (unsigned char)FormatInformationV & 0x3F;
	QRDataBuf[8] |= (FormatInformationV <<1)& 0x80;
	for(i=0;i<6;i++){QRDataBuf[QRWidth+i] |= (FormatInformationV>>(14-i))&0x01;}
	QRDataBuf[QRWidth+7] |= (FormatInformationV>>8)&0x01;
	QRDataBuf[QRWidth+8] |= (FormatInformationV>>7)&0x01;
	/* upper right -----------------------------------*/
	for(i=0;i<8;i++){QRDataBuf[QRWidth*2-i-1] |= (FormatInformationV>>i)&0x01;}
	/* left bottom. ----------------------------------*/
	QRDataBuf[(hbyte-1)*QRWidth+8] |= (FormatInformationV>>(8+(hbyte*8-QRWidth-1)));
	QRDataBuf[(hbyte-2)*QRWidth+8] |= (FormatInformationV>>((hbyte*8-QRWidth-1)));
	
	QRDataBuf[(hbyte-2)*QRWidth+8] |= 0x01<<(8-(hbyte*8-QRWidth));
	return 1;
}


/*----------------------------------------------------------------------------------------------------
 * 检测坐标是否处于有效范围（AlignmentPatterns & position detection patterns ------------------------
 * 参数：x,y显示坐标;ModelXY:对齐图形坐标; 
 * 阐述：
 *----------------------------------------------------------------------------------------------------*/
unsigned char CheckModelXY(unsigned char version,unsigned char *ModelXY,int x,int y)
{
	unsigned char i=0,j=0,Nub=0;
	unsigned char w=0;
	unsigned char port=0;
	w = 4*(version-1)+21;
	 
	for(i=0;i<7;i++){
		if(ModelXY[i]!=0)Nub++;
		else break;
	}
	if(Nub==2){
		if(x>=(ModelXY[1]-2) && x<=(ModelXY[1]+2) && y>=(ModelXY[1]-2) && y<=(ModelXY[1]+2)){
			if(x==(ModelXY[1]-2)||x==(ModelXY[1]+2)||y==(ModelXY[1]-2)||y==(ModelXY[1]+2)){
				port=(unsigned char)y;
				QRDataBuf[w*(y/8)+x]|=0x01<<(port%8);
			} 
			if(x==ModelXY[1] && y==ModelXY[1]){
				port=(unsigned char)y;
				QRDataBuf[w*(y/8)+x]|=0x01<<(port%8);
			}	
			return 1;
		}
	}
	if(Nub>=3){
		for(j=0;j<Nub;j++){
			for(i=0;i<Nub;i++){
				if(j==0){
					if(i>0&&i<(Nub-1)){
						if(x>=(ModelXY[j]-2) && x<=(ModelXY[j]+2) && y>=(ModelXY[i]-2) && y<=(ModelXY[i]+2))
						{
							if(x==(ModelXY[j]-2)||x==(ModelXY[j]+2)||y==(ModelXY[i]-2)||y==(ModelXY[i]+2)){
								port=(unsigned char)y;
								QRDataBuf[w*(y/8)+x]|=0x01<<(port%8);
							}
							return 1;
						}
					}
				}
				else if(j==(Nub-1)){
					if(i>0){
						if(x>=(ModelXY[j]-2) && x<=(ModelXY[j]+2) && y>=(ModelXY[i]-2) && y<=(ModelXY[i]+2)){
							if(x==(ModelXY[j]-2)||x==(ModelXY[j]+2)||y==(ModelXY[i]-2)||y==(ModelXY[i]+2)){
								port=(unsigned char)y;
								QRDataBuf[w*(y/8)+x]|=0x01<<(port%8);
							}
							if(x==ModelXY[j] && y==ModelXY[i]){
								port=(unsigned char)y;
								QRDataBuf[w*(y/8)+x]|=0x01<<(port%8);
							}
						return 1;}
					}
				}
				else{
					if(x>=(ModelXY[j]-2) && x<=(ModelXY[j]+2) && y>=(ModelXY[i]-2) && y<=(ModelXY[i]+2))
					{
						if(x==(ModelXY[j]-2)||x==(ModelXY[j]+2)||y==(ModelXY[i]-2)||y==(ModelXY[i]+2)){
								port=(unsigned char)y;
								QRDataBuf[w*(y/8)+x]|=0x01<<(port%8);
						}
						if(x==ModelXY[j] && y==ModelXY[i]){
							port=(unsigned char)y;
							QRDataBuf[w*(y/8)+x]|=0x01<<(port%8);
						}
						return 1;
					}
				}
			}
		}
	}
	if((x<=8 && y<=8)||(x>=(w-8)&&y<=8)||(x<=8&&y>=(w-8)))return 1;
	if(x==6 || y==6)return 1;
	if(version>=7){
		if(x<=(w-9)&& x>=(w-11)&& y>=0 && y<=5)return 1;
		if(x<=5 && x>=0 && y<=(w-9) && y>=(w-11))return 1;
	}
	return 0;
}

/*------------------------------------------------------------------------------------------
	判断坐标属于哪个数据字节中
	条件：坐标(X,Y).版本号=w*h
	判断：
	(w*(y/8)+x)=所属数据字节位数
-------------------------------------------------------------------------------------------*/
/* 二维码掩模处理及排置处理(填充最终编码)------------------------------------------------*/
unsigned char QRDataMasking(unsigned char version,unsigned char MaskNub,unsigned char *ModelXY)
{
	unsigned char bit=0;
	unsigned char QRWidth=0;  //QRWidth存储版本对应的点阵大小
	unsigned long Nub=0;
	unsigned char i=0;//,BUF[2]={0,0};
	unsigned short bitIndex = 0;
	unsigned short QRDataByteNub=0;//存储版本对应的编码数据字节数
	int x,y,xx,direction = - 1;
	unsigned char port=0;
	QRWidth = 4*version+17;
	#if FLASH_MODE_DEF
	/* flash mode --------------------------------------*/
	r_dat_bat(806+(version-1)*2,2,BUF);
	QRDataByteNub = (unsigned short)BUF[0]<<8|BUF[1];
	#else
	/* ram mode ----------------------------------------*/
	QRDataByteNub = QRDataByte[version-1];
	#endif
	
	// Start from the right bottom cell.
	x = QRWidth - 1;
  y = QRWidth - 1;
	while (x >= 0 && x<0x10000)
	{
				// Skip the vertical timing pattern.
				if (x == 6)x -= 1;
				while (y >= 0 && y < QRWidth)
				{
					for ( i = 0; i < 2; ++i)
					{
						 xx = x - i;
						// Skip the cell if it's not empty.  
						if(CheckModelXY(version,ModelXY,xx,y)!=0)continue;
						if(Nub < (QRDataByteNub*8))
						{
							++Nub;
							if(QRCodeDataBuf[bitIndex]&0x80)bit=0x01;//bit=QRSET;
							else bit=0x00;//bit=QRRESET;
							QRCodeDataBuf[bitIndex]<<=1;
							if(Nub!=0 & Nub%8==0)bitIndex+=1;
						}
						else{
							bit=0x00;
						}
						switch(MaskNub)
						{
							case 0:if((xx+y)%2==0)bit^=0x1;//000
							break;
							case 1:if(y%2==0)bit^=0x1;//001
							break;
							case 2:if(xx%3==0)bit^=0x1;//010
							break;
							case 3:if((xx+y)%3==0)bit^=0x1;//011
							break;
							case 4:if((xx/3+y/2)%2==0)bit^=0x1;//100
							break;
							case 5:if(((xx*y)%2+(xx*y)%3)==0)bit^=0x1;//101
							break;
							case 6:if(((xx*y)%2+(xx*y)%3)%2==0)bit^=0x1;//110
							break;
							case 7:if(((xx*y)%3+(xx+y)%2)%2==0)bit^=0x1;//111
							break;
						}
						port=(unsigned char)y;
						QRDataBuf[QRWidth*(y/8)+xx]|=bit<<(port%8);
					}
					y += direction;
				}
				direction = - direction; // Reverse the direction.
				y += direction;
				x -= 2; // Move to the left.
			}
	return 1;
}

#endif
/* */

