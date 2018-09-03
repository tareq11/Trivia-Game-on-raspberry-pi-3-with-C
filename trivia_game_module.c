/*
 * 
 * This module contain a small and simple Trivia game
 * in the init we read all the qustion from a file to a buffer
 * on Button press the module will print 1 qustion with 4 possible answer
 * the user choose an answer by pressing keys 1-4, if user choose the 
 * right answer the yellow led will turn on for 1 sec, if the answer is wrong the red led will turn on for
 * 1 sec.
 * 
 * the Timer callback is running all the time, when user press a key 1-4 (an answer) the keyboard
 * callback will turn on a flag telling timer callback to turn on a led, corresponding to the usre answer
 * 
 * * Authors:
* Tareq Arafat,<arafat.tareq6@gmail.com>
* Atif Abed alhalem, <cesc.atef@gmail.com>
*
* NOV 2017
 * */




#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/keyboard.h>
#include <linux/semaphore.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/moduleparam.h>
#include <linux/debugfs.h>
#include <linux/input.h>
#include <linux/ctype.h>
#include <linux/file.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/rcupdate.h>

#define DRIVER_AUTHOR "Tarek & Atif"
#define DRIVER_DESC "Trivia Game"

// define the timer activation in secound
#define SECONDS_COUNT 1
#define QUSTIONFILEPATH "/home/pi/Desktop/project/qustion.txt"

static unsigned int gpioLED6 = 6;
//static bool ledOn6 = 0;

static unsigned int gpioLED13 = 13;
//static bool ledOn13 = 0;

static unsigned int gpioButton = 16;
static unsigned int irqNumber;

/* setup timer struct for the timer block*/
struct timer_list myTimer; //the struct of the timer setup

static int qustionNumber = -1;//counting number asked, init to -1 for easier increment in button press callback

static int ledOn = false;//flag indicate for timerFun to turn on or off the led

static int answerRight = -1;//flag indicate if answer is right or wrong

static int answers[5] = { 2, 4, 4, 5, 2 };//answers for the qustion, answer is by keymap (2 is key "1")

static char buffer[2048] = { 0 };
static int index = 0;

/*
* Function to starts the timer and set time to activate
*/
static void my_set_timer(struct timer_list * mtimer)
{
	 mtimer->expires = jiffies + (HZ*SECONDS_COUNT);//setting up when the timer callback will be called
   add_timer (mtimer); /* setup the timer again */
}

void timerFun (unsigned long arg)
 {
	 /*
	 *if led is on then we need to turn them off
	 *if led is off turn on the led6 if answer was right
	 *or led 13 if answer was wrong
	 */
	 if(ledOn == true)
	 {
		 if(answerRight)//turn on yellow led - answer is right
		 {
			 gpio_set_value(gpioLED6,1);
			 gpio_set_value(gpioLED13,0);
			 ledOn = false;
		 }
		 else//turn on red led - answer is wrong
		 {
			 gpio_set_value(gpioLED6,0);
			 gpio_set_value(gpioLED13,1);
			 ledOn = false;
		 }
	 }
	 else
	 {
		 //turn off leds
		 gpio_set_value(gpioLED6,0);
		 gpio_set_value(gpioLED13,0);
		 ledOn = false;
	 }
  my_set_timer(&myTimer); //inserting timer again to timer_list to be called again, next call will turn off led
}

/*
* keyboard notifier
*/
// declaring semaphore lock for the keyboard
struct semaphore sem;

static const char* keymap[] = { "\0", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "_BACKSPACE_", "_TAB_",
                        "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "_ENTER_", "_CTRL_", "a", "s", "d", "f",
                        "g", "h", "j", "k", "l", ";", "'", "`", "_SHIFT_", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".",
                        "/", "_SHIFT_", "\0", "\0", " ", "_CAPSLOCK_", "_F1_", "_F2_", "_F3_", "_F4_", "_F5_", "_F6_", "_F7_",
                        "_F8_", "_F9_", "_F10_", "_NUMLOCK_", "_SCROLLLOCK_", "_HOME_", "_UP_", "_PGUP_", "-", "_LEFT_", "5",
                        "_RTARROW_", "+", "_END_", "_DOWN_", "_PGDN_", "_INS_", "_DEL_", "\0", "\0", "\0", "_F11_", "_F12_",
                        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "_ENTER_", "CTRL_", "/", "_PRTSCR_", "ALT", "\0", "_HOME_",
                        "_UP_", "_PGUP_", "_LEFT_", "_RIGHT_", "_END_", "_DOWN_", "_PGDN_", "_INSERT_", "_DEL_", "\0", "\0",
                        "\0", "\0", "\0", "\0", "\0", "_PAUSE_"};


/*
* notifier function - callback
* if user pressed a key , callback check if answer is right or wrong , changing flag for timer callback to turn on led
*/
int keylogger_notify(struct notifier_block *nblock, unsigned long code, void *_param)
{
        struct keyboard_notifier_param *param = _param;

		if (code == KBD_KEYCODE)
		{
			if(param->down)
			{
				//acquire lock to read from keyboard
				down(&sem);

				if(qustionNumber < 5  && qustionNumber != -1)
				{
					  if(keymap[param->value] == keymap[answers[qustionNumber]])
					  {
						answerRight = true;
						printk(KERN_INFO "Trivia: user answer is true for qustion %d, yellow led on\n",qustionNumber);
						ledOn = true;// tell timer to turn on led
					  }
					  else
					  {
						answerRight=false;
						ledOn = true;
						printk(KERN_INFO "Trivia: Worng Answer, red led is on\n");
					  }

				}
				else
				{
					printk(KERN_INFO "Trivia: No more qustion\n");
				}


				up(&sem);
			  }

        }
    return NOTIFY_OK;
}

static struct notifier_block keylogger_nb =
{
    .notifier_call = keylogger_notify
};

/*
 * This function open a file ,qustion file, then copy(read) all content to buffer
 * 
 * */
void read(char* filename)
{
	struct file *filp;
    mm_segment_t old_fs = get_fs();
    set_fs(get_ds());
    

	/* Open the file */
    filp = filp_open(filename, O_RDWR | O_CREAT | O_APPEND, 0444);
    if (IS_ERR(filp)) {
        printk("open file error\n");
        return;
    }
	
    /* read from file all the qustion and answers */

	vfs_read(filp, buffer, 2048 , &filp->f_pos);


	/*close file*/
	filp_close(filp, NULL);
    /* Restore kernel memory setting */
    set_fs(old_fs);
	printk(KERN_INFO "Trivia: read file to buffer end\n");
	return;
	
	
}

/*
* irq setup for Button
*/

// function to prevent mechanical bouncing of the button
static unsigned char debounce_button(void)
{
	static unsigned long old_jiffies = 0; //last time we entered the function and return was true
	unsigned long diff = jiffies - old_jiffies; // time delta (in jiffies) before now and last time return was true

	if(diff < 20){ //the delta may be different for mechanical prefs of the button.
		return 0;
	}
	 old_jiffies = jiffies; //uptadte the time
	return 1;
}

/*
* every time gpio Button pressed this function will be called.
* Copy from buffer 1 qustion and print it on dmesg log
*/
static irq_handler_t irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	//preventing button debounce
	if(debounce_button() == 0){
		return (irq_handler_t) IRQ_HANDLED; //inform the system that we handled the interrupt
	}
	printk(KERN_INFO "Button pressed\n");
	char qustion[128];
	int i=0,j=0,k=0;
	
	qustionNumber++;//indicate wich qustion number game is on
	
	//indicate to user no more qustion to be asked
	if(qustionNumber >= 5)
	{
		printk(KERN_INFO "No more qustion");
		return (irq_handler_t) IRQ_HANDLED;
	}
	
	//copy the qustion from buffer and print it to dmesg log
	for(j=0;j<5;j++)
	{
		while(buffer[index] != '\n')
		{
			qustion[i++] = buffer[index++];
		}
		qustion[i] = '\0';
		i=0;
		index++;
		printk(KERN_INFO "%s\n",qustion);
		
		
		//clearing last input from buffer
		for(k=0;k<128;k++)
		{
			qustion[k] = 0;
		}
		qustion[127] = '\0';
	}

    printk(KERN_INFO "Qustion printed, end Button callback\n");
  
  return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}


/*
* init - initialize all the module resource , registering to keyboard event starting timer
* and copying the qustion from buffer.
*/
static int __init trivia_init(void)
{
	  int result = 0;
	  //keybot notifier init
	  register_keyboard_notifier(&keylogger_nb);
	  printk(KERN_INFO "Trivia: Registering the keylogger module with the keyboard notifier list\n");
	  sema_init(&sem, 1);



	  //led and button init
	  gpio_request(gpioLED6,"yellow");
	  gpio_direction_output(gpioLED6, 0);

	  gpio_request(gpioLED13,"red");
	  gpio_direction_output(gpioLED13, 0);
		printk(KERN_INFO "Trivia: gpio leds added.\n");

	  gpio_request(gpioButton, "button");       // Set up the gpioButton
	  gpio_direction_input(gpioButton);        // Set the button GPIO to be an input
	  gpio_set_debounce(gpioButton, 200);      // Debounce the button with a delay of 200ms

	  irqNumber = gpio_to_irq(gpioButton);
	  printk(KERN_INFO "Trivia: GPIO_TEST: The button is mapped to IRQ: %d\n", irqNumber);

	   // This next call requests an interrupt line
	   result = request_irq(irqNumber,             // The interrupt number requested
							(irq_handler_t) irq_handler, // The pointer to the handler function below
							IRQF_TRIGGER_RISING,   // Interrupt on rising edge (button press, not release)
							"rsbp3_gpio_handler",    // Used in /proc/interrupts to identify the owner
							NULL);                 // The *dev_id for shared interrupt lines, NULL is okay

	   printk(KERN_INFO "Trivia: Request irq Done\n");
		
	   gpio_set_value(gpioLED6,0);
	   gpio_set_value(gpioLED13,0);

	    //timer init
	   init_timer(&myTimer);
	   myTimer.function = timerFun;//telling the control block what function to activate
	   myTimer.data = 0;
	   my_set_timer(&myTimer);
		
	   //read all the qustion from the file to a buffer
	   read(QUSTIONFILEPATH);

	  printk(KERN_INFO "Trivia: init finished\n");
	  return 0;

}

/*
* free all the resouces of the module and removing the module from the kernel
*/
static void __exit trivia_exit(void)
{
	printk(KERN_INFO "Trivia: exit started");

	unregister_keyboard_notifier(&keylogger_nb);
    printk(KERN_INFO "Trivia: Unregistered the keylogger module \n");

    //free gpio
	gpio_set_value(gpioLED6,0);
	gpio_set_value(gpioLED13,0);


	gpio_free(gpioLED6);
	gpio_free(gpioLED13);
	printk(KERN_INFO "Trivia: removed leds");

	free_irq(irqNumber,NULL);
	gpio_free(gpioButton);
	printk(KERN_INFO "Trivia: removed Button");

	//free timer
	if(!del_timer(&myTimer))
	{
		printk(KERN_INFO "Trivia: Couldn't remove timer");
	}
	else
	{
		printk(KERN_INFO "Trivia: Timer removed");
	}

	printk(KERN_INFO "Trivia: unloaded and finished\n");


}

module_init(trivia_init);
module_exit(trivia_exit);

MODULE_LICENSE("GPL");
