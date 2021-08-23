/*

* Replica of UK based pelican crossing.

* Using state machines and interrupts.

*/

const int red_led = 13;
const int amber_led = 12;
const int green_led = 11;
const int pedestrian_red = 3;
const int pedestrian_green = 2;
const int button = 4;

typedef enum {RED, RED_AMBER, GREEN, AMBER} states;
states current_state = GREEN;
const int times[] = {10, 4, 10, 4};
int time_in_current_state = 0;

const int MAX_TASKS = 10;

typedef struct {
  int counter;
  int period;
  void (*function)(void);
} TaskType;

typedef struct {
  TaskType tasks[MAX_TASKS];
  int number_of_tasks;
} TaskList;

TaskList tlist;

int add_task(int period, int initial, void(*fun)(void))
{
  if (tlist.number_of_tasks == MAX_TASKS) return 0;
  tlist.tasks[tlist.number_of_tasks].period = period;
  tlist.tasks[tlist.number_of_tasks].counter = initial;
  tlist.tasks[tlist.number_of_tasks].function = fun;
  tlist.number_of_tasks++;
  return tlist.number_of_tasks;
}

void check_button(void)
{
  Serial.println(digitalRead(button));
  if (digitalRead(button) == LOW) 
  {
    current_state = AMBER;
    time_in_current_state = 0;
  }
}

void set_lights(int r, int a, int g)
{
  digitalWrite(red_led, r);
  digitalWrite(amber_led, a);
  digitalWrite(green_led, g);
}

void set_pedestrian(int safe)
{
  digitalWrite(pedestrian_green, safe ? HIGH : LOW);
  digitalWrite(pedestrian_red, safe ? LOW : HIGH);
}

void update_lights()
{
  
  bool change = false;
  // Handle the timing to switch between states.
  time_in_current_state++;
  if (time_in_current_state == times[current_state])
  {
    change = true;
    time_in_current_state = 0;
  }
  
  switch (current_state)
  {
      case RED:
        set_lights(HIGH, LOW, LOW);
        set_pedestrian(true);
        if (change) current_state = RED_AMBER;
        break;
      case RED_AMBER:
    	set_lights(HIGH, HIGH, LOW);
        set_pedestrian(false);
        if (change) current_state = GREEN;
        break;
      case GREEN:
    	set_lights(LOW, LOW, HIGH);
        set_pedestrian(false);
        // On a pelican crossing, the traffic lights don't
        // change from green unless the button has been pressed.
        break;
      case AMBER:
    	set_lights(LOW, HIGH, LOW);
        set_pedestrian(false);
        if (change) current_state = RED;
        break;
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(red_led, OUTPUT);
  pinMode(amber_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(pedestrian_red, OUTPUT);
  pinMode(pedestrian_green, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  tlist.number_of_tasks = 0;
  if (add_task(1000, 0, update_lights))
  {
    Serial.println("Task 1 added successfully");
  }
  if (add_task(100, 10, check_button))
  {
    Serial.println("Task 2 added successfully");
  }
    
  noInterrupts();           
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;   
  OCR1A = 16000;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);   
  interrupts();   
}

void loop()
{
  //Nothing here, instead using state-machine.
}

ISR(TIMER1_COMPA_vect)         
{
  for (int i = 0; i < tlist.number_of_tasks; i++)
  {
    if (tlist.tasks[i].counter == 0)
    {
      tlist.tasks[i].function();
      tlist.tasks[i].counter = tlist.tasks[i].period;
    }
    tlist.tasks[i].counter--;
  }
}

