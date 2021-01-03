from tkinter import *
import serial
import threading

# Note: if the serial port that the board is connected to is not COM9 then change the port section on line 8
class Window(Frame):
    #open com port and assign to variable board
    board = serial.Serial(port='COM9', baudrate=9600)

    # initialize the class
    def __init__(self,master=None):
        Frame.__init__(self,master)
        self.master = master
        self.init_window()
        
    #initialize tkinter window with buttons
    def init_window(self):
        self.master.title("SYSC 3310 Project")
        self.pack(fill=BOTH,expand=1)
        nextButton = Button(self,text="Next",bg='red', fg='Black',height=20,width=30,font=("bold_text",10,"bold"),command=self.nextCommand)
        prevButton = Button(self,text="Prev",bg='green',fg='black',height=20,width=30,font=("bold_text",10,"bold"),command=self.prevCommand)
        prevButton.place(x=250,y=0)
        nextButton.place(x=0,y=0)
        
    # check for state update
    def updateState(self,text,board):
        #check if new message waiting
        if(board.inWaiting() >0):
            #read serial and convert to integer
            res = int(board.read())
            # cases to choose text 
            if res == 1:
                text.config(text="State: led1=OFF, led2=OFF",font=("bold_text",10,"bold"))
            elif res == 2:
                text.config(text="State: led1=OFF, led2=ON",font=("bold_text",10,"bold"))
            elif res == 3:
                text.config(text="State: led1=ON, led2=OFF",font=("bold_text",10,"bold"))        
            elif res == 4:
                text.config(text="State: led1=ON, led2=ON",font=("bold_text",10,"bold"))
        #rerun function every 100ms   
        self.after(100,self.updateState,text,board)
        
    #send 'P' to the serial port
    def prevCommand(self):
        self.board.write('P'.encode())
        
    #send 'N' to the serial port
    def nextCommand(self):
        self.board.write('N'.encode())
    
root = Tk()
#choose sizing of window
root.geometry("500x300")
app = Window(root)
#set resizable to false
root.resizable(0,0)
#default text until first serial message is sent
text = Label(root,text="Awaiting Serial First Notification...",font=("bold_text",10,"bold"))
text.pack()
#create and run a thread to continously check if there was a state update
thread1 = threading.Thread(target=app.updateState, args=(text,app.board))
thread1.start()
#run main loop for tkinter 
root.mainloop()