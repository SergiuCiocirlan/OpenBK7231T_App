
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
    // Register the BH1750 on the software I2C bus at address 0x23.
    if (DRV_I2C_FindDeviceExt(I2C_BUS_SOFT, 0x23, I2CDEV_UNKNOWN))
        return;

    i2cDevice_BH1750_t *sensor =
        (i2cDevice_BH1750_t *)calloc(1, sizeof(i2cDevice_BH1750_t));

    if (!sensor)
        return;

    sensor->base.busType = I2C_BUS_SOFT;
    sensor->base.addr = 0x23;
    sensor->base.type = I2CDEV_UNKNOWN;
    sensor->base.runFrame = BH1750_RunFrame;
    sensor->base.channelChange = BH1750_ChannelChange;

    sensor->luxChannel = -1;
    sensor->initialized = 0;

    DRV_I2C_AddNextDevice((i2cDevice_t *)sensor);
}
