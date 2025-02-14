#ifndef PKFOC_H
#define PKFOC_H

#include "as5047p.h"
#include "pid_controller.h"
#include "stm32f4xx_hal.h"
#include <math.h>

// 添加常量定义
#define SQRT_3 1.732050807568877f

class PKFOC {
public:
  PKFOC(SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *htim,
        ADC_HandleTypeDef *hadc);

  // 配置函数
  void set_adc(ADC_HandleTypeDef *hadcx);
  void set_tim(TIM_HandleTypeDef *htimx, uint32_t channelu, uint32_t channelv,
               uint32_t channelw, uint32_t channelInJ);
  void set_endocer(AS5047P *driver);
  void set_target_I(float I_q, float I_d);
  void set_UqUd(float _Uq, float _Ud);
  void set_UqUdIqIdMAX(float _UqUdMax, float _IqIdMax);

  // 控制循环
  void get_angle();
  void ctrl_open_Loop();
  void ctrl_I_Loop();
  void ctrl_speed_Loop();
  void ctrl_position_Loop();
  PIDController Speed_pid;
  void set_speed(float speed) { Speed_pid.set_desireValue(speed); }
  void set_position(float position) { Position_pid.set_desireValue(position); }
  // PWM和ADC控制
  void update_I(uint8_t valid_num, uint32_t Uu, uint32_t Uv);
  void update_pwm(float tu, float tv, float tw);
  void halt_pwm();
  void start_pwm();
  void start_adc_sample();

  // FOC变换相关函数
  void cal_angle_sincos();
  void clark();
  void iclark();
  void park();
  void ipark();
  void generate_svpwm();

private:
  // 硬件相关
  AS5047P *as5047;
  TIM_HandleTypeDef *htim;
  ADC_HandleTypeDef *hadc;
  uint32_t TIM_CHANNEL_U, TIM_CHANNEL_V, TIM_CHANNEL_W, TIM_CHANNEL_InJ;

  // FOC参数
  float Pole_Pair;
  float Ts;
  float Udc;
  float Ud, Uq;
  float K;
  float mechanical_offset;
  float ADC_Uu_offset, ADC_Uv_offset, ADC_Uw_offset;
  float target_Iq, target_Id;

  // 电机状态
  float angle;
  float mechanical_angle;
  float Ia, Ib, Ic;
  float Ialpha, Ibeta;
  float Id, Iq;
  float Ualpha, Ubeta;
  float sinVal, cosVal;
  float pos_gap;
  float position;
  float speed;

  // 控制标志
  uint8_t motor_rotate_direct;

  // PID控制器
  PIDController I_pid_q;
  PIDController I_pid_d;

  PIDController Position_pid;
  float _tx_max, _tx_min;
  float _Ix_max, _Ix_min;
  float _Iqd_max, _Iqd_min;
  float _Ialphabeta_max, _Ialphabeta_min;
  // 限制参数
  float UqUd_Limit_Max;
  float IqId_Limit_Max;

  // 添加三相电压变量
  float Ua, Ub, Uc;
};

#endif /* PKFOC_H */
