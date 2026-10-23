
#include "../new_common.h"
#include "drv_i2c_local.h"

typedef struct {
    i2cDevice_t base;
    int luxChannel;
    int initialized;
} i2cDevice_BH1750_t;


static void BH1750_AddDeviceInternal(int busType, int address, int targetChannel);

static commandResult_t BH1750_AddDevice(
    const void *context,
    const char *cmd,
    const char *args,
    int cmdFlags
);

static void BH1750_RunFrame(i2cDevice_t *dev)
{
    i2cDevice_BH1750_t *sensor = (i2cDevice_BH1750_t *)dev;
    byte data[2];

    // First frame: start continuous high-resolution measurement.
    if (!sensor->initialized) {
        if (DRV_I2C_Begin(dev->addr, dev->busType) != 0)
            return;

        DRV_I2C_Write(0, 0x10);
        DRV_I2C_Close();

        sensor->initialized = 1;
        return;
    }

    // Subsequent frames: read the latest measurement.
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
    //cmddetail:{"name":"addI2CDevice_BH1750","args":"[Soft/I2C1/I2C2] [address] [targetChannel]",
    //cmddetail:"descr":"Adds a BH1750 ambient light sensor",
    //cmddetail:"fn":"BH1750_AddDevice","file":"i2c/drv_i2c_bh1750.c","requires":"",
    //cmddetail:"examples":"addI2CDevice_BH1750 Soft 35 1"}
    CMD_RegisterCommand("addI2CDevice_BH1750", BH1750_AddDevice, NULL);
}



static void BH1750_AddDeviceInternal(int busType, int address, int targetChannel)
{
    i2cDevice_BH1750_t *sensor =
        (i2cDevice_BH1750_t *)calloc(1, sizeof(i2cDevice_BH1750_t));

    if (!sensor)
        return;

    sensor->base.addr = address;
    sensor->base.busType = busType;
    sensor->base.type = I2CDEV_UNKNOWN;
    sensor->base.next = 0;
    sensor->base.runFrame = BH1750_RunFrame;
    sensor->base.channelChange = BH1750_ChannelChange;
    sensor->luxChannel = targetChannel;
    sensor->initialized = 0;

    DRV_I2C_AddNextDevice((i2cDevice_t *)sensor);
}

static commandResult_t BH1750_AddDevice(const void *context,
    const char *cmd, const char *args, int cmdFlags)
{
    const char *busName;
    int address;
    int targetChannel;
    i2cBusType_t busType;

    Tokenizer_TokenizeString(args, 0);
    busName = Tokenizer_GetArg(0);
    address = Tokenizer_GetArgInteger(1);
    targetChannel = Tokenizer_GetArgInteger(2);

    busType = DRV_I2C_ParseBusType(busName);

    if (busType == I2C_BUS_ERROR || address < 0x03 || address > 0x77)
        return CMD_RES_BAD_ARGUMENT;

    if (DRV_I2C_FindDevice(busType, address))
        return CMD_RES_BAD_ARGUMENT;

    BH1750_AddDeviceInternal(busType, address, targetChannel);
    return CMD_RES_OK;
}
