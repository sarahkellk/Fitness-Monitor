# ENCE361 Fitness Monitor

## Setup

Note: This is instruction set assumes the correct TivaWare drivers have been installed. It is also possible to simply copy the source files into an existing project if a project has been setup already with similar properties. 

1. Clone the repository.
```
git clone https://eng-git.canterbury.ac.nz/ence361-2022/group19.git
```
2. Create a new project in CCS.
	* Select File>New>CCS project.
	* Click on “Target” window shown in the dialog box and start typing … “Tiva TM4C123GH6PM”.
	* For the “Connection”, select “Stellaris in -circuit Debugger Interface”.
	* Lastly, select “Empty Project” from the Project “Templates” window.
3. Copy source from the repository files into project root.
4. Set project properties.
	* Next, right-click the project name and select “Properties”.
	* From the Build pull down menu, select “Include Options”.
	* Use browse button to find the “TivaWare_C_Series_2.2.0.295” directory in C:\ti This will allow your CCS compiler to find the include files that CCS requires in order to compile your code.
	* From the same “Properties” dialog box, look down the list on the left side and select “Build | ARM linker” and then select “File search path”.
	* Click green ‘+’ button in the include library dialog box and select browse button. Find “C:\ti\TivaWare_C_Series-2.2.0.295\driverlib\ccs\Debug\driverlib.lib”.
	* Select Build>ARM Linker>Basic Options and increase the heap size to 512.
6. Build in CCS by clicking the debug or build button. Run by debugging then resuming program.

## Usage
Right Switch UP: Change to test mode.

Right Switch DOWN: Change to normal mode.

Button UP in normal mode: Cycles through units to display.

Button UP in test mode: Adds 100 steps, 0.09km to distance travelled.

Button DOWN in test mode: Decreases distance travelled by 500 steps, 0.45km.

Short Button DOWN in normal mode and Step Goal State: Set Current Goal to displayed New Goal.

Long Button DOWN in normal mode and not Step Goal State: Reset current distance travelled to zero.

Rotate Potentiometer Clockwise in normal mode and Step Goal State: Add 100 steps to New Goal.

Rotate Potentiometer Anticlockwise in normal mode and Step Goal State: Remove 100 steps from New Goal.

### Authors: Kenneth Huang, Sarah Kellock
