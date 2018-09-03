# advanced-os-course-project-tareq-atif
external hardware:
1-red LED -tow wires one to  GPIO for the LED is 13 and and the second put Ground 
2-green LED -tow wires one to GPIO for the LED is 6 and the second Ground
3-button -three wires one for GPIO for the LED is 16 and second put on 3v-power and third on Ground 

our project have to view question from file of the game and the modules in the kernel, we excute the module  that responsible to copy messages from the text file to the kernel according of the turning on the matched gpio LED.

 This module contain a small and simple Trivia game (we can add more question to the game with changing the The rest of the corresponding elements)
 * in the init we read all the qustion from a file to a buffer
 * on Button press the module will print 1 qustion with 4 possible answer
 * the user choose an answer by pressing keys 1-4, if user choose the 
 * right answer the yellow led will turn on for 1 sec, if the answer is wrong the red led will turn on for
  1 sec.
 * the Timer callback is running all the time, when user press a key 1-4 (an answer) the keyboard
 * callback will turn on a flag telling timer callback to turn on a led, corresponding to the usre answer
 
**we used the necessary code commands and instruction to implement module  :


 
