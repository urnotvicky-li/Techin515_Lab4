# Techin515_Lab4
Magic Wand:
<img src="![IMG_6036](https://github.com/user-attachments/assets/5cadf880-56f0-483c-86e3-f8b68a5a9f6a)" alt="Sample" width="400"/>

### Video 01 Link: https://drive.google.com/file/d/1muUuauE_8MSV_i0Ff8PW1j95K-GNkTY7/view?usp=sharing 
### Videl 02 Link: https://drive.google.com/file/d/1Q98iff1jx9BpyjaomakpaLEsOYEP4kXj/view?usp=sharing 

## Data collection process and results:

![My Diagram](Images/my-diagram.png)


## Edge Impulse model architecture and optimization:

## Performance analysis and metrics:



## Discussion Questions:
### Why should you use training data collected by multiple students rather than using your own collected data only? Think about the effectiveness and reliability of your wand.

If you only use your own data, the model might just learn your way of doing the gestures. But when I try to add data from other people, it gets better at recognizing the same gesture done in different ways, so it works more reliably for everyone.

### Discuss the effect of window size. Consider the number of samples generated, the number of neurons in your input layer of neural network, effectiveness when capturing slow-changing patterns

If the window’s too big, it can catch slower gestures better, but it also makes the model heavier and slower to train. If it’s too small, it might miss the full gesture. You kind of have to find a sweet spot where it’s just long enough to capture the motion, but not too much that it slows everything down.

One way to improve the model is to collect more gesture data from different people so it learns more variations. Another thing you can try is tweaking the model architecture—like adding a layer or changing the size—to see if it helps pick up patterns better.


### Challenges faced and solutions:
I ran into several issues like the horizontal header pins weren’t actually connected, so I had to resolder them for a reliable connection; the MPU6050 wouldn’t talk over I2C device until I corrected the SDA/SCL wiring and adjusted the pull-up resistors in code; the RGB LEDs failed to light properly until I fixed the PWM setup and reassigned the correct GPIO pins; and the 3D-printed enclosure was the wrong size, so I updated the CAD dimensions and reprinted it to ensure all components fit smoothly.

