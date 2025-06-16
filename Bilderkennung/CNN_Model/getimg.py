import time
import cv2 as cv
from screentext import ScreenText
from fpstracker import FPSTracker
from serialcomm import comm
import inference as RCNN
from capthread import VideoCaptureAsync
class Object():
    def __init__(self, x1, y1, x2, y2, label, confidence):
        self.x1 = x1
        self.y1 = y1
        self.x2 = x2
        self.y2 = y2
        self.label = label
        self.confidence = confidence
    def __str__(self):
        getlabeltext = lambda x:"Glass" if x==1 else "Hand"
        return f"Object: {getlabeltext(self.label)} (({self.x1}, {self.y1}), ({self.x2}, {self.y2})), {self.confidence}"

#check whether array contains objects with a specific label
def checkforObject(objects, searchlabel):
    for o in objects:
        if o.label == searchlabel:
            return True
    return False

def drawvisuals(img, x1, y1, x2, y2, text):
    cv.rectangle(img, (x1,y1), (x2,y2), (0,255,0), 2)
    cv.putText(img, text, (x1+5,y1+15), cv.FONT_HERSHEY_SIMPLEX,
            0.5, (0, 255, 0), 1, cv.LINE_AA)
    
def update_fps():
    fps = fpstracker.updateFPS()
    fps_text.set_Text(f"FPS: {fps:.0f}")
    fps_text.set_Position((10,20))
    fps_text.set_Color((0,0,255))
    fps_text.showText(frame)

#get arduino
arduino = comm("COM5")
fps_text = ScreenText()
fpstracker = FPSTracker()
try:
    model = RCNN.initmodel()
except FileNotFoundError:
    print("weights file not found, place in CNN_Model/ folder")
    exit()

objects = []
waitingforUNO = False
runtime = 0
score_threshold = 0.9

# Kamera öffnen (0 für die erste erkannte Kamera)
cap_thread = VideoCaptureAsync(0)

if not cap_thread.cap.isOpened():
    print("Error: capture not open")

while True:
    ret, frame = cap_thread.read()

    if not ret:
        print("Error: no frame")
        continue
        #break
    
    #pre-process the image
    frame = cv.resize(frame, (640, 480))#make sure every frame has the same size, otherwise dependent on camera
    #frame = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)#apply grayscale, color not needed for shape detection, reduces size
    
    if not waitingforUNO:
        #pass img to RCNN model
        start_time = time.time()
        output = RCNN.checkimg(model, frame)
        runtime = time.time() - start_time
        print(f"Time: {runtime}")
        print(f"Output:\n{output}")
        prediction = output[0]
        
        #if cup is found, get cup coordinates
        for box,  label, score, in zip(prediction['boxes'], prediction['labels'], prediction['scores']):
            if score.item()>=score_threshold:
                x1, y1, x2, y2 = box.int().tolist()
                objects.append(Object(x1, y1, x2, y2, label.item(), score.item()))
                text = objects[len(objects)-1].__str__()
                drawvisuals(frame, x1, y1, x2, y2, text)
        
        #pass coordinate to Arduino if no hand in img
        if checkforObject(objects, 1) and not checkforObject(objects, 2):
            #get coordinates of the center of the first cup
            xmid = int(objects[0].x1) + 0.5*(objects[0].x2-objects[0].x1)
            ymid = int(objects[0].y1) + 0.5*(objects[0].y2-objects[0].y1)
            arduino.passtoSerial(xmid, ymid)
            waitingforUNO = True
    else:
        if arduino.checkresponse()!=None:
            waitingforUNO = False
            objects.clear()

    #show pre-processed frame
    cv.imshow('Livebild', frame)
            
    if cv.waitKey(10) & 0xFF == ord('q'): 
        break

cap_thread.stop()
cv.destroyAllWindows()