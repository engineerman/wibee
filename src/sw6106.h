#include <Arduino.h>
#include "Wire.h"

#define SW6106_I2C_SLAVE_ADDR 0x3C

enum SW6115Register : uint8_t
{
    BG_CTRL = 0x01,        ///< 2.1. REG 0x01: BG Control
    KEY_EVENT_CTRL = 0x03, ///< 2.2. REG 0x03: Key Event Ctrl
    IQR_PEND_1 = 0x05,     ///< 2.3. REG 0x05: IRQ Pending1
    IQR_PEND_2 = 0x06,     ///< 2.4. REG 0x06: IRQ Pending2
    IQR_PEND_3 = 0x07,     ///< 2.5. REG 0x07: IRQ Pending3
    IQR_PEND_4 = 0x08,     ///< 2.6. REG 0x06: IRQ Pending4
    IQR_EN = 0x09,         ///< 2.7. REG 0x09: IRQ Enable
    MASKBITS_1 = 0x0A,     ///< 2.8. REG 0x0A: IRQ Maskbits1
    MASKBITS_2 = 0x0B,     ///< 2.9. REG 0x0B: IRQ Maskbits2
    MASKBITS_3 = 0x0C,     ///< 2.10. REG 0x0C: IRQ Maskbits3
    MASKBITS_4 = 0x0D,     ///< 2.11. REG 0x0D: IRQ Maskbits4
    STATUSS = 0x11,        ///< 2.12. REG 0x11: System Status
    DAC_BDATA = 0x13,      ///< 2.13. REG 0x13: Boost DAC Data
    ADC_VDATA_1 = 0x14,    ///< 2.14. REG 0x14: ADC Vbat Data
    ADC_VDATA_2 = 0x15,    ///< 2.15. REG 0x15: ADC Vbat/Vout Data
    ADC_VDATA_3 = 0x16,    ///< 2.16. REG 0x16: ADC Vout Data
    ADC_IDATA_1 = 0x17,    ///< 2.17. REG 0x17: ADC Ichg Data
    ADC_IDATA_2 = 0x18,    ///< 2.18. REG 0x18: ADC Ichg/Idischg Data
    ADC_IDATA_3 = 0x19,    ///< 2.19. REG 0x19: ADC Idischg Data
    ADC_TDATA_1 = 0x1A,    ///< 2.20. REG 0x1A: ADC Die Temp Data
    ADC_TDATA_2 = 0x1B,    ///< 2.21. REG 0x1B: ADC IC Temperature/NTC Data
    ADC_TDATA_3 = 0x1C,    ///< 2.22. REG 0x1C: ADC NTC Data
    CTRL_PWR = 0x22,       ///< 2.23. REG 0x22: Control Power
    INFO = 0x26,           ///< 2.24. REG 0x26: Version Info
    TYPEC_IND = 0x37,      ///< 2.25. REG 0x37: Typec Indication
    PLUGOUT_CONFIG = 0x38, ///< 2.26. REG 0x38: Plug Out Config
    CHRGR_CONFIG_1 = 0x3A, ///< 2.27. REG 0x3A: Charger Config1
    CHRGR_CONFIG_2 = 0x3B, ///< 2.28. REG 0x3B: Charger Config2
    BOOST_CONFIG = 0x3D,   ///< 2.29. REG 0x3D: Boost Config
    RDC_CONFIG = 0x48,     ///< 2.30. REG 0x48: RDC Config
    GAUGE_CONFIG = 0x49,   ///< 2.31. REG 0x49: Gauge Config
    RDC_VAL_1 = 0x4A,      ///< 2.32. REG 0x4A: Rdc Value by Compensation
    RDC_VAL_2 = 0x4B,      ///< 2.33. REG 0x4B: RDC Value Precompensation
    RDC_VAL_3 = 0x4C,      ///< 2.34. REG 0x4C: RDC Value Compensation Hibit
    OCV_1 = 0x4D,          ///< 2.35. REG 0x4D: OCV Current Percent
    OCV_2 = 0x4E,          ///< 2.36. REG 0x4E: OCV Useful Percent
    OCV_3 = 0x4F,          ///< 2.37. REG 0x4F: Final Percent
    LED_CONFIG_1 = 0x50,   ///< 2.38. REG 0x50: LED Percent Config1
    LED_CONFIG_2 = 0x51,   ///< 2.39. REG 0x51: LED Percent Config2
    LED_CONFIG_3 = 0x52,   ///< 2.40. REG 0x52: LED Percent Config3
    LED_CONFIG_4 = 0x53,   ///< 2.41. REG 0x53: LED Percent Config4
    PORT_CONFIG = 0x5E,    ///< 2.42. REG 0x5E: Port Config
                           // 2.43. REG 0x60~0x6F: OCV curve; No information provided

};

byte readSW6106Reg(byte regaddress)
{
    byte error;
    byte regval[1];
    Wire.beginTransmission(SW6106_I2C_SLAVE_ADDR);
    Wire.write(regaddress);
    error = Wire.endTransmission(false);
    if (error)
        Serial.printf("endTransmission: %u\n", error);
    error = Wire.requestFrom(SW6106_I2C_SLAVE_ADDR, 1);
    Wire.readBytes(regval, 1);

    // printf("reg %d  =  0x%x\n",regaddress,regval[0]);
    return regval[0];
}

float readSW6106Vout()
{
    uint16_t r15 = readSW6106Reg(ADC_VDATA_2);
    uint16_t r16 = readSW6106Reg(ADC_VDATA_3);

    return ((((r15 >> 4) << 8) | r16) & 0xFFF) * 4;
    // Vout= ((Reg0x15[7:4]<<8)+ Reg0x16[7:0]) *4 mV
}

float readSW6106BattVoltage()
{
    uint16_t r14 = readSW6106Reg(ADC_VDATA_1);
    uint16_t r15 = readSW6106Reg(ADC_VDATA_2);

    //  printf("r15 shifted 0x%x\n",(((r15 << 8) | r14) & 0xFFF));

    return (((r15 << 8) | r14) & 0xFFF) * 1.2;
}

float readSW6106Ichrg()
{
    uint16_t r17 = readSW6106Reg(ADC_IDATA_1);
    uint16_t r18 = readSW6106Reg(ADC_IDATA_2);

    return (((r18 << 8) | r17) & 0xFFF) * 25 / 7;
    // ICharge = ((Reg0x18[3:0]<<8)+ Reg0x17[7:0])*25/7 mA
}

float readSW6106Idischrg()
{

    uint16_t r18 = readSW6106Reg(ADC_IDATA_2);
    uint16_t r19 = readSW6106Reg(ADC_IDATA_3);

    return ((((r18 >> 4) << 8) | r19) & 0xFFF) * 25 / 7;
    // IDischarge = ((Reg0x18[7:4]<<8)+ Reg0x19[7:0])*25/7 mA
}

float readSW6106Tic()
{

    uint16_t r20 = readSW6106Reg(ADC_TDATA_1);
    uint16_t r21 = readSW6106Reg(ADC_TDATA_2);

    return ((((r21 << 8) | r20) & 0xFFF) - 1851) * 1 / 6.82;
    // TDie= ((Reg0x1B[3:0]<<8)+ Reg0x1A[7:0] - 1851)*1/6.82 ℃
}

float readSW6106Vntc()
{
    uint16_t r21 = readSW6106Reg(ADC_TDATA_2);
    uint16_t r22 = readSW6106Reg(ADC_TDATA_3);

    return ((((r21 >> 4) << 8) | r22) & 0xFFF) * 1.1;
    // VNTC = ((Reg0x1B[7:4]<<8)+ Reg0x1C[7:0])*1.1mV;
}
