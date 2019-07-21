from matplotlib import pyplot as plt
from matplotlib import style,animation
import numpy as np
from numpy import trapz
from scipy.integrate import simps
import time


plt.ion()		# Start interactive mode
style.use('ggplot')
fig = plt.figure()	# Create a figure "fig"
ax1 = fig.add_subplot(111)	# Create a subplot "ax1"

ax2=ax1.twinx()					#use the same x axis
bbox_props = dict(boxstyle="square,pad=0.3", fc="green", ec="b", lw=1)		#define properties of the square box
         

file_name = input('Enter the "EXACT" CSV file name: ') #take the file_name from the user
strlen=len(file_name)		#get the length of file_name
if ((file_name[strlen-4 : strlen] != ".csv") and (file_name[strlen-4 : strlen] != ".CSV")):
	file_name += ".csv"	#if file name does not have .csv extension, upend it
time = file_name[-13:-4]
time = time[:2] + ':' + time[3:5] + ':' + time[6:8]
date = file_name[-24:-14]
resistance = int(file_name[:3])	#take resistance value from the file_name
resistance /= 1000		#resistance in kilo ohms
hours = np.arange(0,864000,1800)  # Half an hour in seconds for 10 days	       
hrNo = np.arange(0,240,0.5)     #define hour numbers for every half an hour
StartTime = "Date:- " + date + "\nStart Time:- " + time		# Define a statement for display of start time and date
        
def animate(i):
        try:
                Value = np.loadtxt(file_name,
                                unpack= True,
                                skiprows = 3,
                                delimiter = ',',usecols = 1)
        
                l = len(Value) #get the number of columns
                if(l <3):
                        return
                plotter(Value,l)
                fig.canvas.draw()

        except:
                Value = np.loadtxt(open(file_name,'rt').readlines()[:-3],skiprows=3,delimiter = ',',unpack = True,usecols = 1)
                TimeTaken = open(file_name,'r').readlines()[-2][:7]       # To fid Toatal time taken
                l = len(Value)
                mAh = plotter(Value,l)
                x = range(0,4*l-1,4)
                zz = "Capacity: " + str(mAh)[0 : 7] +"mAh\nNo. of data points:"+str(l)  #string to display capacity
                ax1.text((x[(3*l//5)] + (x[l-1]-x[0])*0.01), (Value[(l//6)] + 0.025), zz,
                         horizontalalignment="left", verticalalignment="bottom",
                         size=10, bbox=bbox_props)		#display the capacity i.e. area under I v/s t
                fig.canvas.draw()

                
# The FunAnimation function calls the "animate" function for every 200ms(default)                
ani = animation.FuncAnimation(fig,animate)

def axes_label():
        ax1.set_xlabel('Time (Hrs)')
        ax1.set_ylabel('Voltage (V)')
        ax1.tick_params(axis='y',labelcolor='blue')         #set tick parameter of y axis
        ax1.tick_params(axis='x',labelcolor='black')	#set tick parametr of x axis
        ax2.set_ylabel('Current (mA)')
        ax2.tick_params(axis='y',labelcolor='red')		#set tick parameter of secondary y axis
        plt.xticks(hours,hrNo)
        
# plotter function takes in the range type "Value" and "l" calculates and returns mAh value.
# It also clears the previous fig and redraws the figure with the updated values in the file
def plotter(Value,l):
        x=range(0,4*l-1,4)	#time varies from 0 to 2l-1 with a step size of 2sec
        I =[(i/resistance) for i in Value]  	#divided by the load resistance gives the current value (mA)
        mAh=trapz(I,dx=4) 	#area under the curve
        mAh /= 3600      #to convert time unit from seconds to hour
        
        ax1.clear()
        Vplot, =ax1.plot(x, Value,'b-',label='Voltage (V)')	#plot x v/s Value i.e. voltage

        ax2.clear()
        Cplot, =ax2.plot(x,I,'r--',label='Current (mA)')		#plot x v/s current
        
        
        axes_label()
        ax1.text((x[50] + (x[l-1] - x[0])*0.01), Value[l-3], StartTime,
                 horizontalalignment = "left", verticalalignment = "bottom",
                 size = 12, bbox = bbox_props)                  #Display Date and StartTime
        plt.legend(handles=[Vplot,Cplot])			#display legend
        fig.tight_layout()
        return mAh

        
