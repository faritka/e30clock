#include <ClockTemperature.h>

/**
 * The temperatire from resistance lookup table for the NTC resistor B57164K0472J000 (4.7k, 5%)
 * https://www.tdk-electronics.tdk.com/web/designtool/ntc/
 */
const ClockNtc ClockTemperature::ntcLookupTable[] = {
    {-47, 235430, 193750, 277110},
    {-46, 219910, 181490, 258340},
    {-45, 205520, 170080, 240960},
    {-44, 192160, 159460, 224860},
    {-43, 179750, 149570, 209940},
    {-42, 168230, 140350, 196100},
    {-41, 157520, 131760, 183270},
    {-40, 147560, 123760, 171360},
    {-39, 138290, 116280, 160300},
    {-38, 129660, 109310, 150020},
    {-37, 121630, 102800, 140470},
    {-36, 114150, 96712,  131580},
    {-35, 107170, 91026,  123310},
    {-34, 100660, 85708,  115620},
    {-33, 94591,  80734,  108450},
    {-32, 88924,  76079,  101770},
    {-31, 83631,  71721,  95542},
    {-30, 78687,  67639,  89734},
    {-29, 73995,  63754,  84236},
    {-28, 69615,  60118,  79112},
    {-27, 65524,  56713,  74334},
    {-26, 61700,  53524,  69876},
    {-25, 58126,  50535,  65716},
    {-24, 54782,  47733,  61830},
    {-23, 51652,  45105,  58200},
    {-22, 48722,  42638,  54807},
    {-21, 45978,  40322,  51634},
    {-20, 43406,  38147,  48665},
    {-19, 41045,  36148,  45942},
    {-18, 38826,  34265,  43388},
    {-17, 36741,  32491,  40991},
    {-16, 34781,  30820,  38741},
    {-15, 32937,  29245,  36629},
    {-14, 31202,  27760,  34644},
    {-13, 29569,  26359,  32779},
    {-12, 28031,  25037,  31025},
    {-11, 26583,  23789,  29376},
    {-10, 25217,  22611,  27824},
    {-9,  23910,  21479,  26341},
    {-8,  22679,  20411,  24946},
    {-7,  21518,  19403,  23633},
    {-6,  20424,  18451,  22397},
    {-5,  19392,  17551,  21234},
    {-4,  18419,  16700,  20138},
    {-3,  17500,  15896,  19105},
    {-2,  16633,  15135,  18131},
    {-1,  15814,  14415,  17213},
    {0,   15040,  13733,  16347},
    {1,   14305,  13084,  15525},
    {2,   13610,  12470,  14749},
    {3,   12953,  11888,  14017},
    {4,   12331,  11337,  13325},
    {5,   11743,  10815,  12672},
    {6,   11187,  10319,  12055},
    {7,   10660,  9849.7, 11471},
    {8,   10161,  9404.1, 10919},
    {9,   9688.9, 8981.3, 10397},
    {10,  9241.1, 8579.9, 9902.4},
    {11,  8817.7, 8199.7, 9435.7},
    {12,  8416.1, 7838.6, 8993.7},
    {13,  8035.2, 7495.5, 8574.9},
    {14,  7673.6, 7169.2, 8177.9},
    {15,  7330.3, 6859.0, 7801.6},
    {16,  7004.4, 6564.0, 7444.7},
    {17,  6694.7, 6283.3, 7106.2},
    {18,  6400.5, 6016.1, 6784.9},
    {19,  6120.9, 5761.8, 6480.0},
    {20,  5855.0, 5519.7, 6190.4},
    {21,  5600.3, 5287.2, 5913.4},
    {22,  5358.1, 5065.8, 5650.3},
    {23,  5127.7, 4855.0, 5400.4},
    {24,  4908.6, 4654.1, 5163.0},
    {25,  4700.0, 4465.0, 4935.0},
    {26,  4496.2, 4263.2, 4729.2},
    {27,  4302.5, 4073.8, 4531.2},
    {28,  4118.4, 3894.1, 4342.6},
    {29,  3943.3, 3723.4, 4163.1},
    {30,  3776.7, 3561.2, 3992.2},
    {31,  3621.9, 3410.6, 3833.2},
    {32,  3474.4, 3267.2, 3681.5},
    {33,  3333.7, 3130.7, 3536.6},
    {34,  3199.4, 3000.6, 3398.2},
    {35,  3071.3, 2876.6, 3266.0},
    {36,  2949.0, 2758.4, 3139.6},
    {37,  2832.2, 2645.7, 3018.8},
    {38,  2720.7, 2538.2, 2903.3},
    {39,  2614.2, 2435.6, 2792.8},
    {40,  2512.4, 2337.7, 2687.2},
    {41,  2415.0, 2244.2, 2585.9},
    {42,  2322.0, 2154.9, 2489.0},
    {43,  2232.9, 2069.7, 2396.2},
    {44,  2147.8, 1988.2, 2307.4},
    {45,  2066.4, 1910.4, 2222.4},
    {46,  1988.5, 1836.1, 2140.9},
    {47,  1913.9, 1765.0, 2062.9},
    {48,  1842.6, 1697.1, 1988.1},
    {49,  1774.2, 1632.1, 1916.4},
    {50,  1708.8, 1570.0, 1847.6},
    {51,  1645.6, 1510.1, 1781.2},
    {52,  1585.2, 1452.8, 1717.5},
    {53,  1527.2, 1398.0, 1656.5},
    {54,  1471.8, 1345.6, 1597.9},
    {55,  1418.6, 1295.4, 1541.8},
    {56,  1367.6, 1247.4, 1487.9},
    {57,  1318.8, 1201.4, 1436.2},
    {58,  1271.9, 1157.3, 1386.5},
    {59,  1227.0, 1115.1, 1338.9},
    {60,  1183.9, 1074.7, 1293.1},
};

ClockTemperature::ClockTemperature()
{
    channelId = DT_IO_CHANNELS_INPUT_BY_IDX(DT_PATH(bmw_thermometer), 0);
    enablePin = GPIO_DT_SPEC_GET(DT_PATH(bmw_thermometer), enable_gpios);

    printk("ADC channelId: %d\n", channelId);

    struct adc_channel_cfg channelCfg = {
        .gain = ADC_GAIN_1,
        .reference = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_MAX,
        /* channel ID will be overwritten below */
        .channel_id = channelId,
        .differential = 0
    };

    adcDevice = DEVICE_DT_GET(DT_PHANDLE(DT_PATH(bmw_thermometer), io_channels));

    if (!device_is_ready(adcDevice)) {
        printk("ADC device not found\n");
        return;
    }

    k_mutex_init(&mutexAdc);

    printk("ADC is ready\n");

    adc_channel_setup(adcDevice, &channelCfg);
}


int ClockTemperature::getTemperature()
{
    int temperature = -50;
    uint16_t sampleBuffer;

    printk("ADC getTemperature. ChannelId: %d\n", channelId);

    struct adc_sequence sequence = {
        /* individual channels will be added below */
        .channels    = 0,
        .buffer      = &sampleBuffer,
        /* buffer size in bytes, not number of samples */
        .buffer_size = sizeof(sampleBuffer),
        .resolution  = 12,
        .calibrate   = false,
    };

    sequence.channels |= BIT(channelId);

    if (k_mutex_lock(&mutexAdc, K_MSEC(300)) == 0) {
        /* Configure the enable pin as active */
        gpio_pin_configure_dt(&enablePin, GPIO_OUTPUT_ACTIVE);

        int err = adc_read(adcDevice, &sequence);

        gpio_pin_configure_dt(&enablePin, GPIO_OUTPUT_INACTIVE);
        
        k_mutex_unlock(&mutexAdc);

        if (err != 0) {
            printk("ADC reading failed with error %d.\n", err);
            return -200;
        }
    }

    printk("ADC reading:");
    int32_t measuredVoltage = sampleBuffer;

    printk(" %d", measuredVoltage);
    if (adcVref > 0) {
        adc_raw_to_millivolts(adcVref, ADC_GAIN_1, 12, &measuredVoltage);
        printk(" = %d mV  \n", measuredVoltage);
    }

    uint32_t ntcResistance = (measuredVoltage * dividerResistor) / (adcVref - measuredVoltage);

    printk("Resistance: %d\n", ntcResistance);

    //the difference between Rnom and the measured ntcResistance
    //we will select the Rnom with the smallest difference
    float resistanceNomDiff = 0.0;

    for (uint16_t i = 0; i < sizeof(ntcLookupTable)/sizeof(ntcLookupTable[0]); i++) {

        printk("Temp: %d, Rnom: %zu, Rmin: %zu, Rmax: %zu\n", ntcLookupTable[i].temperature,
            (uint32_t)ntcLookupTable[i].resistanceNom, (uint32_t)ntcLookupTable[i].resistanceMin, (uint32_t)ntcLookupTable[i].resistanceMax);

        //find the temperature for the ntcResistance in the Rmin and Rmax range
        //check if the next temperature has the Rnom that is closer to ntcResistance
        if ((ntcResistance >= ntcLookupTable[i].resistanceMin) && (ntcResistance <= ntcLookupTable[i].resistanceMax)) {
            float resistanceNomDiffTmp = abs(ntcLookupTable[i].resistanceNom - ntcResistance);
            if (!resistanceNomDiff || (resistanceNomDiffTmp < resistanceNomDiff)) {
                temperature = ntcLookupTable[i].temperature;
                resistanceNomDiff = resistanceNomDiffTmp;
                printk("Found temperature: %d\n", temperature);
                printk("ResistanceNomDiff: %zd\n", (int32_t)resistanceNomDiff);
            }
        } else if (resistanceNomDiff) {
            break;
        }
    }

    printk("Measured temperature: %d\n", temperature);

    return temperature;
}


