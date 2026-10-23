
#include "../new_common.h"
#include "drv_i2c_local.h"

typedef struct {
    i2cDevice_t base;
    int luxChannel;
    int initialized;
} i2cDevice_BH1750_t;

static void BH1750_RunFrame(i2cDevice_t *dev)
{
    i2cDevice_BH1750_t *sensor = (i2cDevice_BH1750_t *)dev;
    byte data[2];

    if (DRV_I2C_Begin(dev->addr, dev->busType) != 0)
        return;

    // Start continuous high-resolution measurement.
    DRV_I2C_Write(0, 0x10);
    DRV_I2C_Close();

    if (!sensor->initialized) {
        sensor->initialized = 1;
        return;
    }

    if (DRV_I2C_Begin(dev->addr, dev->busType) != 0)
        return;

    DRV_I2C_ReadBytes(0, data, 2);
    DRV_I2C_Close();

    int raw = ((int)data[0] << 8) | data[1];
    int lux = (raw * 10) / 12;

    CHANNEL_Set(sensor->luxChannel, lux, 0);
}

static void BH1750_ChannelChange(i2cDevice_t *dev, int channel, int value)
{
    (void)dev;
    (void)channel;
    (void)value;
}

void DRV_I2C_BH1750_PreInit(void)
{
    // Driver registration will be added in the next step.
}
