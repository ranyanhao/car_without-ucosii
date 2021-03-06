/**
  ******************************************************************************
  * @file    rtc.c
  * @author  YANDLD
  * @version V2.5
  * @date    2014.3.26
  * @brief   www.beyondcore.net   http://upcmcu.taobao.com 
  ******************************************************************************
  */
  
 #include "rtc.h"
 #include "common.h"
 
 //!< Callback function slot
static RTC_CallBackType RTC_CallBackTable[1] = {NULL};
 
#define SECONDS_IN_A_DAY     (86400U)
#define SECONDS_IN_A_HOUR    (3600U)
#define SECONDS_IN_A_MIN     (60U)
#define DAYS_IN_A_YEAR       (365U)
#define DAYS_IN_A_LEAP_YEAR  (366U)
// Table of month length (in days) for the Un-leap-year
static const uint8_t ULY[] = {0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
// Table of month length (in days) for the Leap-year
static const uint8_t  LY[] = {0U, 31U, 29U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
// Number of days from begin of the non Leap-year
static const uint16_t MONTH_DAYS[] = {0U, 0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U};

/**
 * @brief  RTC模块快速初始化配置
 * @retval None
 */
void RTC_QuickInit(void)
{
    RTC_InitTypeDef RTC_InitStruct1;
    RTC_InitStruct1.oscLoad = kRTC_OScLoad_8PF;
    RTC_Init(&RTC_InitStruct1);
}

/**
 * @brief  由年月日计算出周数
 * @param  year  :年
 * @param  month :月
 * @param  days  :日
 * @retval 返回计算出来的周期数
 */
int RTC_GetWeekFromYMD(int year, int month, int days)
{  
    static int mdays[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30 };  
    int i, y = year - 1;  
    for (i=0; i<month; ++i) days += mdays[i];  
    if (month > 2) 
    { 
         /* Increment date if this is a leap year after February */
        if (((year%400) == 0) || ((year&3) == 0 && (year%100))) ++days;  
    }  
    return (y+y/4-y/100+y/400+days)%7;  
}
 
/**
 * @brief  由日期计算出秒
 * @param  RTC_DateTime_Type :RTC时钟数据结构体，详见rtc.h
 * @param  seconds           :就算出来的秒数据存储地址
 * @retval None
 */
static void RTC_DateTimeToSecond(const RTC_DateTime_Type * datetime, uint32_t * seconds)
{
    /* Compute number of days from 1970 till given year*/
    *seconds = (datetime->year - 1970U) * DAYS_IN_A_YEAR;
    /* Add leap year days */
    *seconds += ((datetime->year / 4) - (1970U / 4));
    /* Add number of days till given month*/
    *seconds += MONTH_DAYS[datetime->month];
    /* Add days in given month*/
    *seconds += datetime->day;
    /* For leap year if month less than or equal to Febraury, decrement day counter*/
    if ((!(datetime->year & 3U)) && (datetime->month <= 2U))
    {
        (*seconds)--;
    }

    *seconds = ((*seconds) * SECONDS_IN_A_DAY) + (datetime->hour * SECONDS_IN_A_HOUR) + 
               (datetime->minute * SECONDS_IN_A_MIN) + datetime->second;
    (*seconds)++;
}

/**
 * @brief  由秒计算出日期
 * @param  seconds           :输入的秒
 * @param  datetime  :计算出来的年月日等信息结构体
 * @retval None
 */
static void RTC_SecondToDateTime(const uint32_t * seconds, RTC_DateTime_Type * datetime)
{
    uint32_t x;
    uint32_t Seconds, Days, Days_in_year;
    const uint8_t *Days_in_month;
    /* Start from 1970-01-01*/
    Seconds = *seconds;
    /* days*/
    Days = Seconds / SECONDS_IN_A_DAY;
    /* seconds left*/
    Seconds = Seconds % SECONDS_IN_A_DAY;
    /* hours*/
    datetime->hour = Seconds / SECONDS_IN_A_HOUR;
    /* seconds left*/
    Seconds = Seconds % SECONDS_IN_A_HOUR;
    /* minutes*/
    datetime->minute = Seconds / SECONDS_IN_A_MIN;
    /* seconds*/
    datetime->second = Seconds % SECONDS_IN_A_MIN;
    /* year*/
    datetime->year = 1970;
    Days_in_year = DAYS_IN_A_YEAR;

    while (Days > Days_in_year)
    {
        Days -= Days_in_year;
        datetime->year++;
        if  (datetime->year & 3U)
        {
            Days_in_year = DAYS_IN_A_YEAR;
        }
        else
        {
            Days_in_year = DAYS_IN_A_LEAP_YEAR;    
        }
    }

    if  (datetime->year & 3U)
    {
        Days_in_month = ULY;
    }
    else
    {
        Days_in_month = LY;    
    }

    for (x=1U; x <= 12U; x++)
    {
        if (Days <= (*(Days_in_month + x)))
        {
            datetime->month = x;
            break;
        }
        else
        {
            Days -= (*(Days_in_month + x));
        }
    }
    datetime->day = Days;
}

/**
 * @brief  获得RTC的时间
 * @code
 *      //获得RTC的时间
 *      RTC_DateTime_Type ts;    //申请一个结构体
 *      RTC_GetDateTime(&ts);    //将日期存储到ts中
 * @endcode
 * @param  datetime  :返回出来的年月日等信息结构体
 * @retval None
 */
void RTC_GetDateTime(RTC_DateTime_Type * datetime)
{
    if(!datetime)
    {
        return;
    }
    uint32_t i = RTC->TSR;
    RTC_SecondToDateTime(&i, datetime);
}

uint32_t RTC_IsTimeValid(void)
{
    if(RTC->TSR)
    {
        return 0;
    }
    return 1;
}

/**
 * @brief  设置闹钟时间
 * @param  datetime  : 时间戳结构体
 * @retval None
 */
void RTC_SetAlarm(RTC_DateTime_Type * datetime)
{
    uint32_t i;
    if(!datetime)
    {
        return;
    }
    RTC_DateTimeToSecond(datetime, &i);
    RTC->TAR = i-1;
}

/**
 * @brief  设置RTC补偿寄存器
 * @param  compensationInterval
      Configures the compensation interval in seconds from 1 to 256 to control
    *  how frequently the TCR should adjust the number of 32.768 kHz cycles in
    *  each second. The value written should be one less than the number of
    *  seconds (for example, write zero to configure for a compensation interval
    *  of one second). This register is double buffered and writes do not take
    *  affect until the end of the current compensation interval.
 * @param  timeCompensation
      Configures the number of 32.768 kHz clock cycles in each second. This
    *  register is double buffered and writes do not take affect until the end
    *  of the current compensation interval.\n
    * \n
    *    80h Time prescaler register overflows every 32896 clock cycles.\n
    *    ... ...\n
    *    FFh Time prescaler register overflows every 32769 clock cycles.\n
    *    00h Time prescaler register overflows every 32768 clock cycles.\n
    *    01h Time prescaler register overflows every 32767 clock cycles.\n
    *    ... ...\n
    *    7Fh Time prescaler register overflows every 32641 clock cycles.\n
 * @retval None
 */
void RTC_SetCompensation(uint32_t compensationInterval, uint32_t timeCompensation)
{
    RTC->TCR &= ~RTC_TCR_CIR_MASK;
    RTC->TCR &= ~RTC_TCR_TCR_MASK;
    RTC->TCR |= RTC_TCR_CIR(compensationInterval);
    RTC->TCR |= RTC_TCR_TCR(timeCompensation);
}
    
/**
 * @brief  RTC模块初始化配置
 * @param  RTC_DateTime_Type :RTC工作模式配置，详见rtc.h
 * @retval None
 */
void RTC_Init(RTC_InitTypeDef * RTC_InitStruct)
{
    uint32_t i;
    SIM->SCGC6 |= SIM_SCGC6_RTC_MASK;
    // RTC->CR = 0;
    /* disable osc */
    RTC->CR &= ~RTC_CR_OSCE_MASK;
    /* OSC load config */
    switch(RTC_InitStruct->oscLoad)
    {
        case kRTC_OScLoad_2PF:
            RTC->CR |= RTC_CR_SC2P_MASK;
            break;
        case kRTC_OScLoad_4PF:
            RTC->CR |= RTC_CR_SC4P_MASK;
            break;
        case kRTC_OScLoad_8PF:
            RTC->CR |= RTC_CR_SC8P_MASK;
            break;
        case kRTC_OScLoad_16PF:
            RTC->CR |= RTC_CR_SC16P_MASK;
            break;
        default:
            break;
    }
    /* enable OSC */
    RTC->CR |= RTC_CR_OSCE_MASK;
	for(i=0;i<0x6000;i++) {};
    /* enable RTC */
    RTC->SR |= RTC_SR_TCE_MASK;
}

/**
 * @brief  获得TSR值
 * @retval TSR值 为0则说明无效
 */
uint32_t RTC_GetTSR(void)
{
    return RTC->TSR;
}

uint32_t RTC_GetTAR(void)
{
    return RTC->TAR;
}
/**
 * @brief  设置RTC的时间
 * @param  datetime  :时间戳结构
 * @retval None
 */
void RTC_SetDateTime(RTC_DateTime_Type * datetime)
{
    uint32_t i;
    if(!datetime)
    {
        return;
    }
    RTC_DateTimeToSecond(datetime, &i);
    LIB_TRACE("RTC_SetDateTime() - Reconfig:%d\r\n", i);
    RTC->SR &= ~RTC_SR_TCE_MASK;
    RTC->TSR = RTC_TSR_TSR(i);
    RTC->SR |= RTC_SR_TCE_MASK;
}

void RTC_SetTSR(uint32_t val)
{
    RTC->SR &= ~RTC_SR_TCE_MASK;
    RTC->TSR = RTC_TSR_TSR(val);
    RTC->SR |= RTC_SR_TCE_MASK;  
}

/**
 * @brief  设置RTC中断功能
 * @code
 *      //设置RTC开启闹钟中断
 *      RTC_ITDMAConfig(kRTC_IT_TimeAlarm); 
 * @endcode
 * @param config: 配置中断类型
 *         @arg kRTC_IT_TimeOverflow_Disable :关闭时间溢出中断
 *         @arg kRTC_IT_TimeAlarm_Disable    :关闭闹钟中断
 *         @arg kRTC_IT_TimeAlarm            :开启闹钟中断
 *         @arg kRTC_IT_TimeOverflow         :开启时间溢出中断
 * @retval None
 */
void RTC_ITDMAConfig(RTC_ITDMAConfig_Type config)
{
    switch(config)
    {
        case kRTC_IT_TimeOverflow_Disable:
            RTC->IER &= ~RTC_IER_TOIE_MASK;
            break;
        case kRTC_IT_TimeOverflow:
            RTC->IER |= RTC_IER_TOIE_MASK;
            NVIC_EnableIRQ(RTC_IRQn);
            break;
        case kRTC_IT_TimeAlarm_Disable:
            RTC->IER &= ~RTC_IER_TAIE_MASK;
            break;
        case kRTC_IT_TimeAlarm:
            RTC->IER |= RTC_IER_TAIE_MASK;
            NVIC_EnableIRQ(RTC_IRQn);
            break;
        default:
            break;
    }
}

/**
 * @brief  注册中断回调函数
 * @param AppCBFun: 回调函数指针入口
 * @retval None
 * @note 对于此函数的具体应用请查阅应用实例
 */
void RTC_CallbackInstall(RTC_CallBackType AppCBFun)
{
    if(AppCBFun != NULL)
    {
        RTC_CallBackTable[0] = AppCBFun;
    }
}
/**
 * @brief  中断处理函数入口
 * @param  RTC_IRQHandler :芯片的RTC中断函数入口
 * @note 函数内部用于中断事件处理
 */
void RTC_IRQHandler(void)
{
    if(RTC_CallBackTable[0])
    {
        RTC_CallBackTable[0]();
    }
    /* clear pending bit if not cleared */
    if((RTC->IER & RTC_IER_TOIE_MASK) && (RTC->SR & RTC_SR_TOF_MASK))
    {
        RTC->TSR = RTC->TSR;
    }
    if((RTC->IER & RTC_IER_TAIE_MASK) && (RTC->SR & RTC_SR_TAF_MASK))
    {
        RTC->TAR = RTC->TAR;
    }
}

