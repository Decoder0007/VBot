import os
import time
from tkinter import Tk, filedialog
from winreg import *
from Reg import *

defaultpath = ""
ans = ""

def Confirm(item, data):
    while True:
        ans = input("Set "+str(item)+" to "+str(data)+"? (y/n)")
        
        if ans == "y" or ans == "Y":
            return True
        elif ans == "n" or ans == "N":
            return False
        else:
            print("Please enter Y or N")
            time.sleep(3)
            os.system("CLS")

def PathSelector():
    root = Tk() # pointing root to Tk() to use it as Tk() in program.
    root.withdraw() # Hides small tkinter window.
    root.attributes('-topmost', True) # Opened windows will be active. above all windows despite of selection.
    path = filedialog.askdirectory() # Returns opened path as str
    return path

def GetInstallPath():
    global installpath
    while True:
        print("___VBot setup___\n")

        print("First we need to get our Geometry Dash folder")
        print("Please select an option")
        print("A. Use default directory")
        print("B. Use custom directory")
            
        ans = input("Enter A or B:  ")

        ###
        if ans == "A" or ans == "a":
            installpath = "C:\Program Files (x86)\Steam\steamapps\common\Geometry Dash"
            if Confirm("path", installpath):
                break
            else:
                os.system("CLS")
        ###
        elif ans == "B" or ans == "b":
            installpath = PathSelector()
            if Confirm("path", installpath):
                break
            else:
                os.system("CLS")
        else:
            print("Please enter A or B")
            time.sleep(3)
        os.system("CLS")
        ###
    
    print("Default path set as:  "+installpath)
    time.sleep(3)
    os.system("CLS")

def GetSteamPath():
    global steampath
    while True:
        steampath = ""
        print("___VBot setup___\n")
        
        print("Now we need to get our steam folder")
        print("Please select an option")
        print("A. Use default directory")
        print("B. Auto-Detect directory")
        print("C. Use custom directory")
            
        ans = input("Enter A, B or C:  ")

        ###
        if ans == "A" or ans == "a":
            steampath = "C:\Program Files (x86)\Steam\\"
            if Confirm("path", steampath):
                break
            else:
                os.system("CLS")
        ###
        elif ans == "B" or ans == "b":
            registry = ConnectRegistry(None, HKEY_LOCAL_MACHINE)
            rawKeyA = OpenKey(registry, "SOFTWARE\WOW6432Node\Valve\Steam")

            try:
                i = 0
                while 1:
                    keyname, value, type = EnumValue(rawKeyA, i)
                    if keyname == "InstallPath":
                        steampath = value
                        break
                    i += 1

            except WindowsError:
                print("END")

                CloseKey(rawKeyA)
                    
            if Confirm("path", steampath):
                break
            else:
                os.system("CLS")
        ###
        elif ans == "C" or ans == "c":
            steampath = PathSelector()
            if Confirm("path", steampath):
                break
            else:
                os.system("CLS")
        else:
            print("Please enter A, B or C")
            time.sleep(3)
            os.system("CLS")
        ###

    print("Steam path set as:  "+steampath)
    time.sleep(3)
    os.system("CLS")
    
GetInstallPath()
GetSteamPath()

print(installpath)
print(steampath)
while True:
    time.sleep(9999)
