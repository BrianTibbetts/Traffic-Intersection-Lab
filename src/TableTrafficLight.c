// ***** 0. Documentation Section *****
/* Registers used for I/O include:
 *   PE0-PE2: Input from the road and pedestrian crossing sensors (3 sensor bits)
 *   PB0-PB5: Output to the 6 traffic lights (3 * 2 LED bits in Green -> Yellow -> Red and West -> South order from least to most significant bit)
 *	example bits: 100001 = West green, South red
 *   PF1, PF3: Output to the pedestrian crossing light which can be red or green (2 On-board LED bits)
 */

// ***** 1. Pre-processor Directives Section *****
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****
// Linked data structure
struct State {
  unsigned long OutTraffic;	// 6-bit pattern for traffic light LEDs
  unsigned long OutCross;	// 2-bit pattern for pedestrian LED
  unsigned long Time;		// Delay in 10ms units
  unsigned long Next[8];}; 	//next state for inputs 0-7
typedef const struct State STyp;

#define GoWest 		0
#define WaitWest	1
#define GoSouth		2
#define WaitSouth	3
#define GoCross 	4
#define HurryOnFlash1	5
#define HurryOffFlash1	6
#define HurryOnFlash2	7
#define HurryOffFlash2	8

// Finite state machine implemented using data from lab10_state_transition_table.ods
STyp FSM[9]={
  {0x21, 0x02, 100, {GoWest,GoWest,WaitWest,WaitWest,WaitWest,WaitWest,WaitWest,WaitWest}},
  {0x22, 0x02, 50, {GoSouth,GoSouth,GoSouth,GoSouth,GoCross,GoCross,GoSouth,GoSouth}},
  {0x0C, 0x02, 100, {GoSouth,WaitSouth,GoSouth,WaitSouth,WaitSouth,WaitSouth,WaitSouth,WaitSouth}},
  {0x14, 0x02, 50, {GoCross,GoWest,GoCross,GoWest,GoCross,GoCross,GoCross,GoCross}},
  {0x24, 0x08, 100, {GoCross,HurryOnFlash1,HurryOnFlash1,HurryOnFlash1,GoCross,HurryOnFlash1,HurryOnFlash1,HurryOnFlash1}},
  {0x24, 0x02, 50, {HurryOffFlash1,HurryOffFlash1,HurryOffFlash1,HurryOffFlash1,HurryOffFlash1,HurryOffFlash1,HurryOffFlash1,HurryOffFlash1}},
  {0x24, 0x00, 50, {HurryOnFlash2,HurryOnFlash2,HurryOnFlash2,HurryOnFlash2,HurryOnFlash2,HurryOnFlash2,HurryOnFlash2,HurryOnFlash2}},
  {0x24, 0x02, 50, {HurryOffFlash2,HurryOffFlash2,HurryOffFlash2,HurryOffFlash2,HurryOffFlash2,HurryOffFlash2,HurryOffFlash2,HurryOffFlash2}},
  {0x24, 0x00, 50, {GoWest,GoWest,GoSouth,GoWest,GoCross,GoWest,GoSouth,GoWest}}
};

unsigned long PresentState;	// index to the present state
unsigned long Input;		// input from the traffic and pedestrian sensors
// Debugging data collection
unsigned long Time[50];
unsigned long Data[50];

// FUNCTION PROTOTYPES: Each subroutine defined
void PLL_Init(void);
void SysTick_Init(void);
void SysTick_Wait(unsigned long delay);
void SysTick_Wait10ms(unsigned long delay);

// ***** 3. Subroutines Section *****
int main(void){ 
  volatile unsigned long delay;
  unsigned long i,last,now;
  PLL_Init();
  SysTick_Init();
  SYSCTL_RCGC2_R |= 0x32; 	// 1) enable clock to F, E, and B GPIO ports (110010 = 0x32)
  delay = SYSCTL_RCGC2_R;	// 2) allow time for clock to start
  
  GPIO_PORTE_AMSEL_R &= ~0x07; // 3) disable analog function on PE2-0
  GPIO_PORTE_PCTL_R &= ~0x00000FFF; // 4) enable regular GPIO
  GPIO_PORTE_DIR_R &= ~0x07;   // 5) inputs on PE2-0
  GPIO_PORTE_AFSEL_R &= ~0x07; // 6) regular function on PE2-0
  GPIO_PORTE_DEN_R |= 0x07;    // 7) enable digital on PE2-0
  
  GPIO_PORTB_AMSEL_R &= ~0x3F; // 3) disable analog function on PB5-0
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // 4) enable regular GPIO
  GPIO_PORTB_DIR_R |= 0x3F;    // 5) outputs on PB5-0
  GPIO_PORTB_AFSEL_R &= ~0x3F; // 6) regular function on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;    // 7) enable digital on PB5-0
  
  GPIO_PORTF_LOCK_R = 0x4C4F434B; // unlock GPIO Port F
  GPIO_PORTF_CR_R |= 0x0A;        // allow changes to PF3 and PF1
  GPIO_PORTF_AMSEL_R &= ~0x0A;    // Clear the PF3 and PF1 bits in Port F AMSEL to disable analog
  GPIO_PORTF_PCTL_R &= ~0x0A;     // Clear the PF3 and PF1 bit fields in Port F PCTL to configure as GPIO
  GPIO_PORTF_DIR_R |= 0x0A;        // Set the Port F direction register so PF3 and PF1 are both outputs
  GPIO_PORTF_AFSEL_R &= ~0x0A;    // Clear the PF3 and PF1 bits in Port F AFSEL to disable alternate functions
  GPIO_PORTF_DEN_R |= 0x0A;       // Set the PF3 and PF1 bits in Port F DEN to enable digital I/O
  /* Altering GPIO_PORTF_PUR_R is unnecessary as none of the TivaC switches are used.
   * The breadboard will have a 10 KOhm resistor on a switch which controls the TivaC LED. */
  
  PresentState = GoWest;
  i = 0;
  while(1){
     // 1) Output to the 6 breadboard LEDs and the TivaC LED.
     GPIO_PORTB_DATA_R = FSM[PresentState].OutTraffic;
     GPIO_PORTF_DATA_R = FSM[PresentState].OutCross;
     // 2) Dwell in the present state.
     SysTick_Wait10ms(FSM[PresentState].Time);
     // 3) Read the 3 switches as inputs.
     Input = GPIO_PORTE_DATA_R&0x07;
     if(i < 50){
        now = NVIC_ST_CURRENT_R;
      	Time[i] = (last-now)&0x00FFFFFF;  // 24-bit time difference
     	Data[i] = Input;
     	last = now;
     	i++;
     }
     
     // 4) Progress to the next state based on which switches were pressed.
     PresentState = FSM[PresentState].Next[Input];
  }
}

void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}

// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay){
  NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}
// 800000*12.5ns equals 10ms
void SysTick_Wait10ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(800000);  // wait 10ms
  }
}

// Activates the LM4F/TM4C with a 16 MHz crystal to run at 80 MHz 
void PLL_Init(void){
  // 0) Use RCC2
  SYSCTL_RCC2_R |=  0x80000000;  // USERCC2
  // 1) bypass PLL while initializing
  SYSCTL_RCC2_R |=  0x00000800;  // BYPASS2, PLL bypass
  // 2) select the crystal value and oscillator source
  SYSCTL_RCC_R = (SYSCTL_RCC_R &~0x000007C0)   // clear XTAL field, bits 10-6
                 + 0x00000540;   // 10101, configure for 16 MHz crystal
  SYSCTL_RCC2_R &= ~0x00000070;  // configure for main oscillator source
  // 3) activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~0x00002000;
  // 4) set the desired system divider
  SYSCTL_RCC2_R |= 0x40000000;   // use 400 MHz PLL
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~ 0x1FC00000)  // clear system clock divider
                  + (4<<22);      // configure for 80 MHz clock
  // 5) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&0x00000040)==0){};  // wait for PLLRIS bit
  // 6) enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~0x00000800;
}
