#include "foc_core.h"
#include "arm_math.h"
#include "common_inc.h"
#include "math.h"
#include <cstdint>

#define ARR_10Khz 4199

PKFOC::PKFOC(SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *htim,
             ADC_HandleTypeDef *hadc)
    : htim(htim), hadc(hadc) {
  // 初始化基本参数
  Pole_Pair = 11;
  Ts = 1;
  Udc = 12.0f;
  Ud = 0.0;
  Uq = 1.5f;

  K = SQRT_3 * Ts / Udc;
  mechanical_offset = 333.7734f; // up 87.5698f down 333.7734f

  // ADC偏移值
  ADC_Uu_offset = 2048;
  ADC_Uv_offset = 2048;
  ADC_Uw_offset = 2048;

  // 目标电流
  target_Iq = 0.0f;
  target_Id = 0.0f;

  // 电机方向和标志位
  motor_rotate_direct = DISABLE;

  // 初始化PID控制器
  I_pid_q.Init();
  I_pid_d.Init();
  Speed_pid.Init();
  Position_pid.Init();

  // 设置PID参数
  I_pid_q.set_PID(3.0, 0.015, 0);
  I_pid_q.set_Iintegral_Limit(200.0f);

  I_pid_d.set_PID(3, 0.015, 0);
  I_pid_d.set_Iintegral_Limit(200.0f);

  Speed_pid.set_PID(-0.065f, -0.007f, 0);
  Speed_pid.set_Iintegral_Limit(200.0f);
  Speed_pid.set_desireValue(0.5);

  Position_pid.set_PID(20, 0.0020, 0.0);
  Position_pid.set_Iintegral_Limit(200.0f);
  Position_pid.set_desireValue(160);

  Aim_pid.set_PID(20, 0.0020, 0.0);
  Aim_pid.set_Iintegral_Limit(200.0f);
  Aim_pid.set_desireValue(0);

  _tx_max = 1;
  _tx_min = 0;
  _Ix_max = 0.22f;
  _Ix_min = -0.22f;
  _Iqd_max = 0.3f;
  _Iqd_min = -0.3f;
  _Ialphabeta_max = 0.22f;
  _Ialphabeta_min = -0.22f;

  // 位置相关
  pos_gap = 0;
  position = 0;

  aimerr = 0;
  // 限制参数
  UqUd_Limit_Max = 6.0f;
  IqId_Limit_Max = 3.0f;
}

void PKFOC::set_tim(TIM_HandleTypeDef *htimx, uint32_t channelu,
                    uint32_t channelv, uint32_t channelw, uint32_t channelInJ) {
  htim = htimx;
  TIM_CHANNEL_U = channelu;
  TIM_CHANNEL_V = channelv;
  TIM_CHANNEL_W = channelw;
  TIM_CHANNEL_InJ = channelInJ;
}

void PKFOC::set_adc(ADC_HandleTypeDef *hadcx) { hadc = hadcx; }

void PKFOC::set_endocer(AS5047P *driver) { as5047 = driver; }

void PKFOC::set_UqUd(float _Uq, float _Ud) {
  Uq = _Uq;
  Ud = _Ud;
}

void PKFOC::set_target_I(float I_q, float I_d) {
  target_Iq = I_q;
  target_Id = I_d;
  I_pid_q.set_desireValue(target_Iq);
  I_pid_d.set_desireValue(target_Id);
}

void PKFOC::set_UqUdIqIdMAX(float _UqUdMax, float _IqIdMax) {
  UqUd_Limit_Max = _UqUdMax;
  IqId_Limit_Max = _IqIdMax;
}

void PKFOC::get_angle() {
  float angle_d = as5047->readAngle(true);
  angle = angle * 0.3f + 0.7f * angle_d;
  pos_gap = as5047->readPositionGap();
  if (angle_d < mechanical_offset) {
    angle_d = mechanical_offset - angle_d;
  } else {
    angle_d = mechanical_offset + (360.0f - angle_d);
  }

  mechanical_angle =
      (motor_rotate_direct == ENABLE) ? (360 - angle_d) : angle_d;
}

/** 控制循环 **/
void PKFOC::ctrl_open_Loop() {
  clark();
  park();
  ipark();
  generate_svpwm();
}

void PKFOC::ctrl_I_Loop() {
  get_angle();
  cal_angle_sincos();
  clark();
  park();

  // 电流环PI控制
  Uq = I_pid_q.cal_PI_AntiSaturation(Iq);
  Ud = I_pid_d.cal_PI_AntiSaturation(Id);

  // 限制电压输出
  Uq = (Uq > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Uq < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Uq;
  Ud = (Ud > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Ud < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Ud;

  ipark();
  generate_svpwm();
}

void PKFOC::ctrl_speed_Loop() {

  get_angle();
  cal_angle_sincos();
  clark();
  park();

  // 速度环控制
  float speed_rad = as5047->readSpeed(); // 获取当前速度
  float desire = Speed_pid.cal_PI(speed_rad);
  I_pid_q.set_desireValue(desire); // 速度环输出作为电流环的目标值

  // 电流环控制
  Uq = I_pid_q.cal_PI_AntiSaturation(Iq);
  Ud = I_pid_d.cal_PI_AntiSaturation(Id);

  // 限制电压输出
  Uq = (Uq > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Uq < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Uq;
  Ud = (Ud > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Ud < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Ud;
  PRINT(foc, "%.3f,%.3f", angle, speed_rad);
  ipark();
  generate_svpwm();
}
void PKFOC::my_sped_control() {
  static uint8_t run_times = 3;
  run_times++;
  if (run_times == 4) {
    get_angle();
    cal_angle_sincos();
    run_times = 0;
  }

  float speed_rad = as5047->readSpeed(); // 获取当前速度
  float desire = Speed_pid.cal_PI(speed_rad);
  I_pid_q.set_desireValue(desire); // 速度环输出作为电流环的目标值
  // 电流环控制
  Uq = I_pid_q.cal_PI_AntiSaturation(Iq);
  Ud = I_pid_d.cal_PI_AntiSaturation(Id);

  // 限制电压输出
  Uq = (Uq > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Uq < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Uq;
  Ud = (Ud > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Ud < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Ud;
  // PRINT(foc, "%.3f,%.3f", angle, speed_rad);
  ipark();
  generate_svpwm();
}

void PKFOC::ctrl_position_Loop() {
  get_angle();
  cal_angle_sincos();
  clark();
  park();

  if (as5047->as5047_read_speed_update() == 1) {
    position += pos_gap;
    float position_error = Position_pid.cal_PI(position);
    Speed_pid.set_desireValue(position_error * 0.005f);
    float speed_rad = as5047->readSpeed();
    float desireI = Speed_pid.cal_PI(speed_rad);
    I_pid_q.set_desireValue(desireI); // 设置期望力矩
  }

  // 电流环控制
  Uq = I_pid_q.cal_PI_AntiSaturation(Iq);
  Ud = I_pid_d.cal_PI_AntiSaturation(Id);
  // PRINT(foc, "%.3f,%.3f", angle, position);
  //   限制电压输出
  Uq = (Uq > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Uq < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Uq;
  Ud = (Ud > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Ud < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Ud;

  ipark();
  generate_svpwm();
}
volatile uint8_t run_timesk = 3;
void PKFOC::my_pos_control() {

  run_timesk++;
  if (run_timesk == 4) {
    get_angle();
    cal_angle_sincos();
    run_timesk = 0;
  }

  clark();
  park();

  if (as5047->as5047_read_speed_update() == 1) {
    position += pos_gap;
    float position_error = Position_pid.cal_PI(position);
    Speed_pid.set_desireValue(position_error * 0.005f);
    float speed_rad = as5047->readSpeed();
    float desireI = Speed_pid.cal_PI(speed_rad);
    I_pid_q.set_desireValue(desireI); // 设置期望力矩
  }

  // 电流环控制
  Uq = I_pid_q.cal_PI_AntiSaturation(Iq);
  Ud = I_pid_d.cal_PI_AntiSaturation(Id);

  //   限制电压输出
  Uq = (Uq > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Uq < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Uq;
  Ud = (Ud > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Ud < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Ud;

  ipark();
  generate_svpwm();
}

void PKFOC::my_aim_control() {
  run_timesk++;
  if (run_timesk == 4) {
    get_angle();
    cal_angle_sincos();
    run_timesk = 0;
    // PRINT(foc, "%.3f", angle);
  }

  clark();
  park();

  if (as5047->as5047_read_speed_update() == 1) {
    position += pos_gap;
    float position_error = Position_pid.cal_PI(position);
    Speed_pid.set_desireValue(position_error * 0.005f);
    float speed_rad = as5047->readSpeed();
    float desireI = Speed_pid.cal_PI(speed_rad);
    I_pid_q.set_desireValue(desireI); // 设置期望力矩
  }

  // 电流环控制
  Uq = I_pid_q.cal_PI_AntiSaturation(Iq);
  Ud = I_pid_d.cal_PI_AntiSaturation(Id);

  //   限制电压输出
  Uq = (Uq > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Uq < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Uq;
  Ud = (Ud > UqUd_Limit_Max)    ? UqUd_Limit_Max
       : (Ud < -UqUd_Limit_Max) ? -UqUd_Limit_Max
                                : Ud;

  ipark();
  generate_svpwm();
}

/** ADC与PWM相关函数 **/
void PKFOC::update_I(uint8_t valid_num, uint32_t Uu, uint32_t Uv) {
  // 将ADC值转换为电压
  float Uu_ = ((float)((int)Uu - (int)ADC_Uu_offset)) * 0.0016f; // 0.001611328f
  float Uv_ = ((float)((int)Uv - (int)ADC_Uv_offset)) * 0.0016f;

  // 计算相电流，使用低通滤波
  Ia = Ia * 0.2f + 2.2857f * Uu_; // 2.2857142856f
  Ib = Ib * 0.2f + 2.2857f * Uv_;
  Ic = Ic * 0.2f + 0.8f * (-(Ia + Ib));
}

void PKFOC::update_pwm(float tu, float tv, float tw) {
  // 限制PWM占空比在0-0.8之间
  tu = (tu > 0.8f) ? 0.8f : (tu < 0.0f) ? 0.0f : tu;
  tv = (tv > 0.8f) ? 0.8f : (tv < 0.0f) ? 0.0f : tv;
  tw = (tw > 0.8f) ? 0.8f : (tw < 0.0f) ? 0.0f : tw;

  // 更新PWM比较值
  htim->Instance->CCR1 = (uint16_t)(tu * ARR_10Khz);
  htim->Instance->CCR2 = (uint16_t)(tv * ARR_10Khz);
  htim->Instance->CCR3 = (uint16_t)(tw * ARR_10Khz);
}

void PKFOC::halt_pwm() {
  __HAL_TIM_SetCompare(htim, TIM_CHANNEL_U, 0);
  __HAL_TIM_SetCompare(htim, TIM_CHANNEL_V, 0);
  __HAL_TIM_SetCompare(htim, TIM_CHANNEL_W, 0);
}

void PKFOC::start_pwm() {
  // 启动PWM输出
  HAL_TIM_PWM_Start(htim, TIM_CHANNEL_U);
  HAL_TIM_PWM_Start(htim, TIM_CHANNEL_V);
  HAL_TIM_PWM_Start(htim, TIM_CHANNEL_W);

  // 启动互补PWM输出
  HAL_TIMEx_PWMN_Start(htim, TIM_CHANNEL_U);
  HAL_TIMEx_PWMN_Start(htim, TIM_CHANNEL_V);
  HAL_TIMEx_PWMN_Start(htim, TIM_CHANNEL_W);

  // 初始化为0占空比
  __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_U, 0);
  __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_V, 0);
  __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_W, 0);

  // 使能定时器更新中断
  __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
}

void PKFOC::start_adc_sample() {
  __HAL_ADC_ENABLE_IT(hadc, ADC_IT_JEOC);
  HAL_ADCEx_InjectedStart(hadc);
  HAL_TIM_PWM_Start(htim, TIM_CHANNEL_InJ);
}

/** FOC变换相关函数 **/

void PKFOC::cal_angle_sincos() {
  float electrical_angle = mechanical_angle * Pole_Pair;
  while (electrical_angle > 360)
    electrical_angle -= 360.0f;
  electrical_angle *= 0.0174f; // 0.0174532922222f

  cosVal = arm_cos_f32(electrical_angle);
  sinVal = arm_sin_f32(electrical_angle);
}

void PKFOC::clark() {
  Ialpha = Ia;
  Ibeta = 0.577f * (Ia + 2 * Ib); // 0.5773502691896f
}

void PKFOC::iclark() {
  Ua = Ualpha;
  Ub = -Ualpha / 2 + 0.866f * Ubeta; // 0.8660254037f
  Uc = -Ualpha / 2 - 0.866f * Ubeta;
}

void PKFOC::park() {
  Iq = Iq * 0.3f + 0.7f * (-Ialpha * sinVal + Ibeta * cosVal);
  Id = Id * 0.3f + 0.7f * (Ialpha * cosVal + Ibeta * sinVal);
}

void PKFOC::ipark() {
  Ualpha = Ud * cosVal - Uq * sinVal;
  Ubeta = Uq * cosVal + Ud * sinVal;
}

void PKFOC::generate_svpwm() {
  float U1, U2, U3;
  float T0 = 0, T1, T2, T3, T4, T5, T6, T7;
  float Ta, Tb, Tc;
  float sqrt_u = SQRT_3 / 2 * Ualpha;
  float Ubeta_div2 = Ubeta / 2;

  // 计算三相电压
  U1 = Ubeta;
  U2 = -sqrt_u - Ubeta_div2;
  U3 = sqrt_u - Ubeta_div2;

  // 扇区判断
  uint8_t A = (U1 > 0) ? 1 : 0;
  uint8_t B = (U2 > 0) ? 1 : 0;
  uint8_t C = (U3 > 0) ? 1 : 0;
  uint8_t N = (A << 2) + (B << 1) + C;

  // 确定扇区
  uint8_t sector = (N == 5)   ? 1
                   : (N == 4) ? 2
                   : (N == 6) ? 3
                   : (N == 2) ? 4
                   : (N == 3) ? 5
                              : 6;

  // 根据扇区计算矢量作用时间
  switch (sector) {
  case 1:
    T4 = K * U3;
    T6 = K * U1;
    if (Ts < T4 + T6) {
      float k = Ts / (T4 + T6);
      T4 *= k;
      T6 *= k;
    }
    T7 = (Ts - T4 - T6) / 2;
    Ta = T4 + T6 + T0 + T7;
    Tb = T6 + T0 + T7;
    Tc = T0 + T7;
    break;

  case 2:
    T2 = -K * U3;
    T6 = -K * U2;
    if (Ts < T2 + T6) {
      float k = Ts / (T2 + T6);
      T2 *= k;
      T6 *= k;
    }
    T7 = (Ts - T2 - T6) / 2;
    Ta = T6 + T0 + T7;
    Tb = T2 + T6 + T0 + T7;
    Tc = T0 + T7;
    break;

  case 3:
    T2 = K * U1;
    T3 = K * U2;
    if (Ts < T2 + T3) {
      float k = Ts / (T2 + T3);
      T2 *= k;
      T3 *= k;
    }
    T7 = (Ts - T2 - T3) / 2;
    Ta = T0 + T7;
    Tb = T2 + T3 + T0 + T7;
    Tc = T3 + T0 + T7;
    break;

  case 4:
    T1 = -K * U1;
    T3 = -K * U3;
    if (Ts < T1 + T3) {
      float k = Ts / (T1 + T3);
      T1 *= k;
      T3 *= k;
    }
    T7 = (Ts - T1 - T3) / 2;
    Ta = T0 + T7;
    Tb = T3 + T0 + T7;
    Tc = T1 + T3 + T0 + T7;
    break;

  case 5:
    T1 = K * U2;
    T5 = K * U3;
    if (Ts < T1 + T5) {
      float k = Ts / (T1 + T5);
      T1 *= k;
      T5 *= k;
    }
    T7 = (Ts - T1 - T5) / 2;
    Ta = T5 + T0 + T7;
    Tb = T0 + T7;
    Tc = T1 + T5 + T0 + T7;
    break;

  case 6:
    T4 = -K * U2;
    T5 = -K * U1;
    if (Ts < T4 + T5) {
      float k = Ts / (T4 + T5);
      T4 *= k;
      T5 *= k;
    }
    T7 = (Ts - T4 - T5) / 2;
    Ta = T4 + T5 + T0 + T7;
    Tb = T0 + T7;
    Tc = T5 + T0 + T7;
    break;
  }

  // 更新PWM占空比
  update_pwm(Ta, Tb, Tc);
}