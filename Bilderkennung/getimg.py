import os
import cv2 as cv
from screentext import ScreenText
from fpstracker import FPSTracker
from CNN_Model import importall as RCNN
# Kamera öffnen (0 für die erste erkannte Kamera)
cap = cv.VideoCapture(0)

if not cap.isOpened():
    print("Error: capture not open")

fps_text = ScreenText()
fpstracker = FPSTracker()

current_dir = os.getcwd()
image_dir = os.path.join(current_dir, "trainingimages/")
imagecounter = 0

model = RCNN.initmodel()

while True:
    ret, frame = cap.read() #ret is a bool for checking output

    if not ret:
        print("Error: no frame")
        break
    
    #actually pre-process the image
    frame = cv.resize(frame, (640, 480))#make sure every frame has the same size, otherwise dependent on camera
    frame = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)#apply grayscale, color not needed for shape detection, reduces size
    
    imagecounter += 1
    filename = "trainingimg"+str(imagecounter)+".jpg"
    cv.imwrite(image_dir + filename, frame)
    #implement frames-per-second counter
    """ fps = fpstracker.updateFPS()
    fps_text.set_Text(f"FPS: {fps:.0f}")
    fps_text.set_Position((10,20))
    fps_text.set_Color((255,255,255))
    fps_frame = fps_text.showText(frame) """
    
    #implement MLM
    position = RCNN.checkimg(model, frame)
    if position[0] == 1:
        break
    #if cup is found, get cup coordinates
    
    #pass coordinate to Arduino
    
    #show pre-processed frame
    cv.imshow('Livebild', frame)
    
    if cv.waitKey(10) & 0xFF == ord('q'): 
        break

cap.release()
cv.destroyAllWindows()