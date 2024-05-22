#pragma once

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

long long get_timestamp();

int random_int();
int random_int(int min_value, int max_value, int random_value);
int random_int(int min_value, int max_value);
float random_float();
float random_float(float min_value, float max_value, float random_value);
float random_float(float min_value, float max_value);
float random_scale(float s);

float sum_array(float *a, int n);
float mag_array(float *a, int n);
