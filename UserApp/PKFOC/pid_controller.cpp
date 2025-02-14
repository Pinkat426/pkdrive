#include "pid_controller.h"

void PIDController::Init() {
  _P = 0.0f;
  _I = 0.0f;
  _D = 0.0f;
  _I_temp = 0.0f;

  _desired_value = 0.0f;
  _error = 0.0f;
  _last_error = 0.0f;
  _acc_integral = 0.0f;
}

void PIDController::set_desireValue(float value) { _desired_value = value; }

void PIDController::set_PID(float P, float I, float D) {
  _P = P;
  _I = I;
  _I_temp = I;
  _D = D;
}

void PIDController::set_P(float P) {
  _P = P;
  _P_temp = P;
}

void PIDController::set_I(float I) {
  _I = I;
  _I_temp = I;
}

void PIDController::set_D(float D) { _D = D; }

void PIDController::set_Iintegral_Limit(float limit) {
  I_integral_MaxLimit = limit;
  I_integral_MinLimit = -limit;
}

float PIDController::cal_PID(float actual_value) {
  float value;
  _last_error = _error;
  _error = _desired_value - actual_value;
  _acc_integral += _error;

  if (_acc_integral > I_integral_MaxLimit)
    _acc_integral = I_integral_MaxLimit;
  else if (_acc_integral < I_integral_MinLimit)
    _acc_integral = I_integral_MinLimit;

  value = _P * _error + _I * _acc_integral + _D * (_error - _last_error);
  return value;
}

float PIDController::cal_PI(float actual_value) {
  float value;
  _last_error = _error;
  _error = _desired_value - actual_value;
  
  _acc_integral += _error;

  if (_acc_integral > I_integral_MaxLimit)
    _acc_integral = I_integral_MaxLimit;
  else if (_acc_integral < I_integral_MinLimit)
    _acc_integral = I_integral_MinLimit;

  value = _P * _error + _I * _acc_integral;
  return value;
}

float PIDController::cal_PI_AntiSaturation(float actual_value) {
  float value;
  _last_error = _error;
  _error = _desired_value - actual_value;

  if (_error < 0.2f) {
    _acc_integral += _error;
  } else {
    _acc_integral += _error * 0.5f;
  }

  if (_acc_integral > I_integral_MaxLimit)
    _acc_integral = I_integral_MaxLimit;
  else if (_acc_integral < I_integral_MinLimit)
    _acc_integral = I_integral_MinLimit;

  value = _P * _error + _I * _acc_integral;
  return value;
}