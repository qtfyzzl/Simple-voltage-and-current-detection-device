#include "stm32f10x.h"//stm32f103c8t6
#include "delay.h"
#include "OLED.h"
#include "bsp_iic.h"
#include "bsp_ina226.h"
#include "TIME.h"
#include "OLEDSHOW.h"

// 定义显示用的变量
u32 disp_voltage; 
u32 disp_current;
u32 disp_power;

extern INA226 ina226_data; // 引用bsp_ina226.c里的结构体
extern const uint8_t BMP_ICON[];

int main(void)
{
    // 1. 底层初始化
    delay_init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    mx_iic_init();
    OLED_Init();
    mx_ina226_init();
    
    // 2. 界面初始化
		OLEDSHOW_Init();
		OLEDSHOW_ShowCN(2,5,0,1);
		OLEDSHOW_ShowCN(2,6,1,1);
		OLEDSHOW_ShowCN(3,5,2,1);
		OLEDSHOW_ShowCN(3,6,3,1);
		OLEDSHOW_ShowCN(3,7,4,1);
	
		OLEDSHOW_ShowBMP(0, 0, 64, 64, BMP_ICON,0);
		Delay_ms(3000);
	
    OLED_Clear();
    OLED_ShowString(1, 1, "Volt : --.-- V"); 
    OLED_ShowString(2, 1, "Curr : --.-- A"); 
    OLED_ShowString(3, 1, "Power: --.-- W"); 
    OLED_ShowString(4, 1, "----------------");

    while (1)
    {
        /* --- 模式选择：你可以注释掉其中一个 --- */

        // 【模式A：真实传感器模式】
        get_power(); // 内部会填充 ina226_data 结构体
        // voltageVal(mV) -> 12000mV 变为 1200 (用于显示12.00)
        disp_voltage = (u32)(ina226_data.voltageVal / 10.0f); 
        // Shunt_Current(mA) -> 500mA 变为 50 (用于显示00.50)
        disp_current = (u32)(ina226_data.Shunt_Current / 10.0f);
        // Power_Val(W) -> 6.0W 变为 600 (用于显示06.00)
        disp_power   = (u32)(ina226_data.Power_Val * 100.0f);

        
        // 【模式B：模拟测试模式 - 如果传感器没接好，用这个看屏幕】
//        static float test_v = 0.0f;
//        test_v += 0.05f; if(test_v > 12.0f) test_v = 0.0f;
//        disp_voltage = (u32)(test_v * 100);
//        disp_current = 50;  // 0.50A
//        disp_power = (disp_voltage * disp_current) / 100;
        

        // --- 刷新显示 ---
        
        // 刷新电压
        OLED_ShowNum(1, 8, disp_voltage / 100, 2); 
        OLED_ShowString(1, 10, ".");
        OLED_ShowNum(1, 11, disp_voltage % 100, 2);

        // 刷新电流
        OLED_ShowNum(2, 8, disp_current / 100, 2);
        OLED_ShowString(2, 10, ".");
        OLED_ShowNum(2, 11, disp_current % 100, 2);

        // 刷新功率
        OLED_ShowNum(3, 8, disp_power / 100, 2);
        OLED_ShowString(3, 10, ".");
        OLED_ShowNum(3, 11, disp_power % 100, 2);

        delay_ms(200); 
    }
}
