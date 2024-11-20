#include "zephyr/device.h"
#include "zephyr/drivers/sensor.h"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/timing/timing.h>
#include "adxl345.h"

LOG_MODULE_REGISTER(Acelerômetro);

#define PERIODO_VELOCIDADE K_MSEC(1)
#define PERIODO_VELOCIDADE_SEGUNDOS ((double)1/(double)1000)

static double last_accelerations[3];
static double velocidade[3];
static const struct device *accel = DEVICE_DT_GET(DT_NODELABEL(adxl345));
static struct k_timer timer_accel;

static void timer_callback(struct k_timer *timer){
    double readings[3], acc_delta;
    adxl345_read_acceleration(readings);

    for(int i = 0; i < 3; i++){
        acc_delta = readings[i] - last_accelerations[i];
        velocidade[i] += acc_delta * PERIODO_VELOCIDADE_SEGUNDOS;
        last_accelerations[i] = readings[i];
    }

    k_timer_start(&timer_accel, PERIODO_VELOCIDADE, K_NO_WAIT);
}

void adxl345_init(){
    adxl345_read_acceleration(last_accelerations);

    for(int i = 0; i < 3; i++){
        velocidade[i] = 0;
    }

    k_timer_init(&timer_accel, timer_callback, NULL);
    k_timer_start(&timer_accel, PERIODO_VELOCIDADE, K_NO_WAIT);
}

int adxl345_read_acceleration(double readings[3]){
    if(!device_is_ready(accel)){
        LOG_ERR("O dispositivo não está pronto.");
        return -1;
    }

    struct sensor_value val[3];
    // antes de ler os dados do sensor é preciso dar fetch
    if(sensor_sample_fetch(accel)){
        LOG_ERR("Erro ao ler a aceleração.");
        return -2;
    }
    if(sensor_channel_get(accel, SENSOR_CHAN_ACCEL_XYZ, val)){
        LOG_ERR("Erro ao ler a aceleração.");
        return -3;
    }
    
    readings[0] = val[0].val1 + ((double)val[0].val2)/1000000;
    readings[1] = val[1].val1 + ((double)val[1].val2)/1000000;
    readings[2] = val[2].val1 + ((double)val[2].val2)/1000000;

    return 0;
}

void adxl345_read_speed(double readings[3]){
    readings[0] = velocidade[0];
    readings[1] = velocidade[1];
    readings[2] = velocidade[2];
}
