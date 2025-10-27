## RUINS WALL PANEL 
Also known as the Hieroglyphics Wall, contains an elaborate matrix of 35 responsive contact points. Where, when one of the contact (hieroglyphic letter) is depressed, it will illuminate and begin to fade out for a few seconds.
Finding the correct 7 letters in a predetermined order, will solve the puzzle. 

## Design
The wall panel containing the 35 contact points is within a 5 feet wide by 4 feet long area; this allows for 7 columns and 5 rows of pressable contacts. Each one of those contact areas are 6 inches wide by 8 inches long, leaving 3 inches gap between each column and approximately 1 5/16 inches gap between each row. For the separation of the electronics, 3/4 inch spacers were created from 2"x4" lumber and placed onto the drywall forming the 1 5/16 inches of gap between the rows. A piece of plywood, of at least 5 feet by 4 feet containing the cut out of the rectangular sections of the 35 contact points, is then placed and fasten onto the spacers using threaded inserts. Two sets of LED strips run along the width (5 feet) of each row. Two small  pieces of copper strips, separated by a small gap, is placed at each intersection of the columns and rows forming our electronic matrix.     

## Matrix
As mentioned above, our matrix is formed from the columns and rows with the pieces of copper strips in their intersections, within the 6"x8" rectangular contact area. For the 7 columns, a piece of thecopper strip is electronical connected together using wires and solder for the 5 rows. Thus, creating 7 conductor contacts for the 7 columns instead of 35. The same was done for the rows, except in between the wires and the remaining copper strips are schottky diodes (having a 0.2 voltage drop). This formed 5 conductor contacts for the rows. This allows for the reading of 35 contacts points while only using 12 wires.

## Controller
For this setup we are using the Arduino Mega. It was suitable for this because large amount of MCU memory space is needed to store the data to control all the LEDs in the Hieroglyphic Wall. In the case of other Arduinos, like Nano, external flash memory with higher capacity would have to be used. 

## Power Supply 
We are using a 5V power supply unit for this setup. The power supply is rated for a maximum of 5A, which more than enough to power the Arduino and the LED strips.

## Algorithm 
1. Scan for inputs from the matrix.
- Set column 1 HIGH.
- Read state of row 1. 
- If HIGH, send the corresponding position given the column and row.
- Otherwise, continue scanning for HIGH state in the following rows up to the last row.
- Move onto column 2 and repeat the same steps above.
2. If there weren't any inputs, leave the subroutine. 
3. Otherwise, using the input position: 
4. Turn on the set of LEDs corresponding to the input position. 
5. Store the input position in a queue for fade effect.
6. If the input position corresponds to the correct value in the correct order 
(order is stored in array), store the input in global array to capture the sequence of 
correct inputs. 
7. Otherwise, clear the global array to restart the sequence capture.
8. Check to see if the index of the array capturing the sequence of correct inputs,
equal to the number of correct values to be found. 
- If so, you have won the game. Otherwise continue. 
9. * Fade the corresponding LEDs from the stored input in the fade queue.
10. Update queue. (First in, first out. Pop input value with LEDs that has faded to black.)
11. Loop back to 1. 

## Connections

