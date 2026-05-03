#include "TIME.h"
#include "oled.h"
#include "bsp_ina226.h"
#include <stdio.h>

u8 string_V[20], string_I[20], string_W[20], string_mAh[20], string_Wh[20], string_T[20];
u16 V_int = 0, V_dec = 0, I_int = 0, I_dec = 0, W_int = 0, W_dec = 0;

static u8 time_num = 0;
static u32 All_Sec = 0;
static u8 Hour = 0;
static u8 Min = 0;
static u8 Sec = 0;
static u32 mAh = 0;
static u32 Wh = 0;

void TIM2_Getsample_Int(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig( TIM2,TIM_IT_Update|TIM_IT_Trigger,ENABLE);//使能定时器2更新触发中断
 
	TIM_Cmd(TIM2, ENABLE);  //使能TIMx外设
 	
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure); 
}
//***************TIME2的中断*******************/
void TIM2_IRQHandler(void)
{
	
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET) //溢出中断
	{
		get_power(); 
    V_int = (uint16_t)(ina226_data.voltageVal / 1000);
    V_dec = (uint16_t)((ina226_data.voltageVal / 1000 - V_int) * 1000);
    I_int = (uint16_t)(ina226_data.Shunt_Current / 1000);
    I_dec = (uint16_t)((ina226_data.Shunt_Current / 1000 - I_int) * 1000);
    W_int = (uint16_t)ina226_data.Power_Val;
    W_dec = (uint16_t)((ina226_data.Power_Val - W_int) * 1000);
    
		
		sprintf((char *)string_V, "%02d.%03d V ", V_int, V_dec);
    sprintf((char *)string_I, "%02d.%03d A ", I_int, I_dec);
    sprintf((char *)string_W, "%02d.%03d W ", W_int, W_dec);
		
		// 在显示函数中，将 string_V 等变量强制转换为 (char *)
		OLED_ShowString(16, 4, (char *)string_V);
		OLED_ShowString(16, 28, (char *)string_I);
		OLED_ShowString(16, 52, (char *)string_W);
		
		//计时
		time_num++;
		if(ina226_data.Shunt_Current > 1)
		{
		if(time_num >= 5)
			{
				All_Sec++;
				Hour = All_Sec/(60*60);
				Min = (All_Sec%(60*60))/60;
				Sec = (All_Sec%(60*60))%60;
				time_num = 0;
			  
				mAh = ina226_data.Shunt_Current/3600 + mAh;
		    Wh += (ina226_data.voltageVal / 1000) * (ina226_data.Shunt_Current/1000)/3600;	
		    
			}
		}
		else
		{
			if(time_num >= 5)
				time_num = 0;
		}
		sprintf((char *)string_mAh, "%06d mAh ", mAh);
		sprintf((char *)string_Wh, "%06d Wh ", Wh);
		sprintf((char *)string_T, "%02d:%02d:%02d ", Hour,Min,Sec);	
    
			OLED_ShowString(24, 76,(char *) string_mAh);
			OLED_ShowString(24, 92,(char *) string_Wh);
			OLED_ShowString(32, 108,(char *) string_T);
	
}
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //清除中断标志位
}

