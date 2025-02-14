#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

class PIDController {
public:
  void Init();
  void set_desireValue(float value);
  void set_PID(float P, float I, float D);
  void set_P(float P);
  void set_I(float I);
  void set_D(float D);
  void set_Iintegral_Limit(float limit);

  float cal_PID(float actual_value);
  float cal_PI(float actual_value);
  float cal_PI_AntiSaturation(float actual_value);

  // 为了让PKFOC类能访问PID参数
  friend class PKFOC;

private:
  float _P;
  float _I;
  float _D;
  float _I_temp;
  float _P_temp;

  float _desired_value;
  float _error;
  float _last_error;
  float _acc_integral;

  float I_integral_MaxLimit;
  float I_integral_MinLimit;
};

#endif /* PID_CONTROLLER_H */