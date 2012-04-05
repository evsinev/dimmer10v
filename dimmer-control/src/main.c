#define STM32F10X_LD_VL 1

#include "stm32f10x.h"

unsigned char flag=0;

volatile uint32_t *DCRDR = (uint32_t *) 0xe000edf8;
volatile uint32_t *DCRDR2 = (uint32_t *)0xe000edfc;
uint32_t theCounter = 0;
uint32_t theBuf[256];
volatile int theTicker = 0;
volatile int theCurrentValue = 100;


void TIM6_DAC_IRQHandler(void) {

  if(TIM6->SR & TIM_SR_UIF)   {
    TIM6->SR &= ~TIM_SR_UIF;     //сбросить флаг
    theTicker++;
    if(theTicker>100) {
        theTicker=0;
    }
    if(theCurrentValue>theTicker) {
        GPIOC->BSRR=GPIO_BSRR_BS9;  //засветил
        GPIOC->BSRR=GPIO_BSRR_BS13;  //засветил
    } else {
        GPIOC->BRR=GPIO_BRR_BR9;    //потушил
        GPIOC->BRR=GPIO_BRR_BR13;    //потушил
    }    
    //GPIOB->ODR ^= 0x0002;        //инвертровать выход
  }  
}

void setupLedTimer() {
//настройка таймера TIM6
  RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;    //подаем тактирование на TIM6
  TIM6->PSC	= 8-1;                //настроить делитель для формирования миллисекунд
  //TIM6->ARR     = 500;                   //2Hz - это 500 миллисекунд
  TIM6->ARR     = 1;                   //2Hz - это 500 миллисекунд
  TIM6->DIER    = TIM_DIER_UIE;        	 //разрешить событие от таймера
  //разрешить перезагрузку и включить таймер
  TIM6->CR1     = TIM_CR1_CEN | TIM_CR1_ARPE;
  //разрешить прерывания от таймера              
  NVIC_EnableIRQ (TIM6_DAC_IRQn);	
}


void InitClk()
{
//тактирование от внутреннего RC-генератора
  RCC->CR   |=  RCC_CR_HSION;            //Включить генератор HSI
  while((RCC->CR & RCC_CR_HSIRDY)==0) {} //Ожидание готовности HSI
  RCC->CFGR &= ~RCC_CFGR_SW;             //
  RCC->CFGR |=  RCC_CFGR_SW_HSI;         //Выбрать источником тактового сигнала HSI
}

void delay_nano(uint16_t value) {
  TIM2->ARR = value;                  // загрузить значение задержки
  TIM2->CNT = 0;
  TIM2->CR1 = TIM_CR1_CEN;	      //запустить таймер
  while((TIM2->SR & TIM_SR_UIF)==0){} //дождаться конца задержки
  TIM2->SR &= ~TIM_SR_UIF;	      //сбросить флаг
}

void setupTimer() {
 RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;   //подать тактирование на TIM2
 // TIM2->PSC     = 800 - 1;               //настроить делитель для формирования миллисекунд
 TIM2->PSC     = 800 - 1;               //настроить делитель для формирования миллисекунд
 TIM2->CR1     = TIM_CR1_OPM; 
}


void setupInterrupts() {
  RCC->APB2ENR|=RCC_APB2ENR_AFIOEN;    //включили тактирование альтернативных функций 

  AFIO->EXTICR[0]&=~AFIO_EXTICR1_EXTI0_PA; //EXTI0 на порт А

  EXTI->IMR|=EXTI_IMR_MR1;                           //На нулевой ножке
  EXTI->RTSR = 0 ;
  EXTI->FTSR = 0 ;
  
  EXTI->FTSR |= EXTI_FTSR_TR1;                          //  работает 100% по спаду, этот режим лучше 13568, 1100, 2200 
  // EXTI->RTSR |= EXTI_RTSR_TR1;                          // 9000 и 18000, 1100 и 2200 по восходящему

  NVIC_EnableIRQ (EXTI1_IRQn);          //включить прерывание

}

void setupInterruptsButton() {
  RCC->APB2ENR|=RCC_APB2ENR_AFIOEN;    //включили тактирование альтернативных функций 

  AFIO->EXTICR[0]&=~AFIO_EXTICR1_EXTI0_PA; //EXTI0 на порт А

  EXTI->IMR|=EXTI_IMR_MR0;                           //На нулевой ножке
  
  EXTI->FTSR |= EXTI_FTSR_TR0;                          //  работает 100% по спаду, этот режим лучше 13568, 1100, 2200 
   EXTI->RTSR |= EXTI_RTSR_TR0;                          // 9000 и 18000, 1100 и 2200 по восходящему

  NVIC_EnableIRQ (EXTI0_IRQn);          //включить прерывание

}



void setupGpio() {
  RCC->APB2ENR|=RCC_APB2ENR_IOPCEN;      //подали тактирование на порт С

  GPIOC->CRH|=(GPIO_CRH_MODE8_1|GPIO_CRH_MODE9_1|GPIO_CRH_MODE13_1);     //настроил РС9, РС8 как выводы с ограниченой скоростью 2МГц
  GPIOC->CRH&=~(GPIO_CRH_MODE13_0|GPIO_CRH_MODE8_0|GPIO_CRH_MODE9_0|GPIO_CRH_CNF9|GPIO_CRH_CNF8|GPIO_CRH_CNF13);
}

int main()
{
    int i;
    
 InitClk();
 setupTimer();
 setupInterrupts();
 setupGpio();  
 setupLedTimer();
 setupInterruptsButton();
 
 delay_nano(1); // правильно сбросим таймер
    
    *DCRDR = &theBuf;
    for(i=0; i<50; i++) {
        theBuf[i] = 0 ;
    }    
    //theBuf[0] = EXTI->FTSR;
   
   GPIOC->BRR=GPIO_BRR_BR8;
    
  while(1)
  {
    
    //delay_nano(50000);

    //delay_nano(50000);
        
    //if(flag)
    //{
      //GPIOC->BSRR=GPIO_BSRR_BS9;  //засветил
    //  GPIOC->BRR=GPIO_BRR_BR8;    //потушил
    //}
    //else
    //{
      //GPIOC->BRR=GPIO_BRR_BR9;    //потушил
    //  GPIOC->BSRR=GPIO_BSRR_BS8;  //засветил
    //}
  }
}

int theBitIndex = 0;
uint32_t theValue = 0;
uint32_t thePreviousButtonCode = 0;


void onButton(uint32_t aCode) {
    if(aCode==0xbe407788 || aCode==0xbe40bf40) {
        theCurrentValue = theCurrentValue > 9 ? 10 : theCurrentValue+1;        
    } else {
        theCurrentValue = theCurrentValue < 1 ? 0 : theCurrentValue-1;        
    }

    thePreviousButtonCode = aCode;
    //theBuf[0] = theCurrentValue;
    theBuf[2] = aCode;
     
    flag = aCode==0xbe407788;
}

void onTsop(uint16_t aTime) {
    int signal ;
    
    if(aTime>900 && aTime<950) return;
    if(aTime>110 && aTime < 120 && thePreviousButtonCode!=0) {
        onButton(thePreviousButtonCode);
        return;
    }
    
    if(aTime>100) {
        theBitIndex = 32;
        theValue = 0;
        
    } else {
        if(aTime<16) {
            signal = 1;
        } else {
            signal = 0;
        }
        
        if(theBitIndex==16) {
            theValue |= 0 << theBitIndex;
            theBitIndex--;
        }
        
        
        theValue |= signal << theBitIndex;
        theBitIndex--;
    } 
    
    if(theBitIndex <=0) {
        //theBuf[0] = theValue;
        onButton(theValue);
        theBitIndex=32;
    }    
    
    
    //flag = signal;
    
    
}

void EXTI0_IRQHandler(void) {
    int i;
  EXTI->PR|=0x01;
  
    theCounter=0;
    for(i=0; i<50; i++) {
        theBuf[i] = 0 ;
    }
    
  theCurrentValue = theCurrentValue - 1;
    if(theCurrentValue<1) {
        theCurrentValue = 100;
    }    
}

void EXTI1_IRQHandler(void) {
  EXTI->PR|=0x01;

  theCurrentValue = theCurrentValue - 1;
    if(theCurrentValue<1) {
        theCurrentValue = 100;
    }    
    
  return ;
    
    //flag^=1;
  onTsop(TIM2->CNT);
    
  if(theCounter<200) { 
    theCounter++;
    theBuf[theCounter] = TIM2->CNT;
  }
    
  TIM2->SR &= ~TIM_SR_UIF;	      //сбросить флаг
  
  TIM2->ARR = 0xffff-1;                  //загрузить значение задержки
  TIM2->CNT = 0;
  TIM2->CR1 |= TIM_CR1_CEN;	      //запустить таймер
  // while((TIM2->SR & TIM_SR_UIF)==0){} //дождаться конца задержки
  
}

