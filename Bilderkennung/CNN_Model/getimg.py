import time
import cv2 as cv
from screentext import ScreenText
from fpstracker import FPSTracker
from serialcomm import passtoSerial ; from serialcomm import checkresponse
import inference as RCNN

class Object():
    def __init__(self, x1, y1, x2, y2, label):
        self.x1 = x1
        self.y1 = y1
        self.x2 = x2
        self.y2 = y2
        self.label = label
    def __str__(self):
        getlabeltext = lambda x:"Glass" if x==1 else "Hand"
        return f"Object: {getlabeltext(self.label)} ({self.x1}, {self.y1}, {self.x2}, {self.y2})"

def checkforObject(objects, searchlabel):
    for o in objects:
        if o.label == searchlabel:
            return True
    return False

def drawvisuals(img, x1, y1, x2, y2, text):
    cv.rectangle(img, (x1,y1), (x2,y2), (0,255,0), 2)
    cv.putText(img, text, (x1+5,y1+15), cv.FONT_HERSHEY_SIMPLEX,
            0.5, (0, 255, 0), 1, cv.LINE_AA)
    

fps_text = ScreenText()
fpstracker = FPSTracker()
model = RCNN.initmodel()
objects = []
waitingforUNO = False
runtime = 0
score_threshold = 0.5
# Kamera öffnen (0 für die erste erkannte Kamera)
cap = cv.VideoCapture(0, cv.CAP_DSHOW)

if not cap.isOpened():
    print("Error: capture not open")

while True:
    ret, frame = cap.read() #ret is a bool for checking output

    if not ret:
        print("Error: no frame")
        break
    
    #actually pre-process the image
    frame = cv.resize(frame, (640, 480))#make sure every frame has the same size, otherwise dependent on camera
    #frame = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)#apply grayscale, color not needed for shape detection, reduces size
    
    #implement frames-per-second counter
    fps = fpstracker.updateFPS()
    fps_text.set_Text(f"FPS: {fps:.0f}")
    fps_text.set_Position((10,20))
    fps_text.set_Color((0,0,255))
    fps_text.showText(frame)
    
    if not waitingforUNO:
        #implement MLM
        start_time = time.time()
        output = RCNN.checkimg(model, frame)
        runtime = time.time() - start_time
        print(runtime)
        print(output)
        prediction = output[0]
        #if cup is found, get cup coordinates
        for box,  label, score, in zip(prediction['boxes'], prediction['labels'], prediction['scores']):
            if score.item()>=score_threshold:
                x1, y1, x2, y2 = box.int().tolist()
                objects.append(Object(x1, y1, x2, y2, label.item()))
                getlabeltext = lambda x:"Glass" if x==1 else "Hand"
                text = f"{getlabeltext(label.item())} ({score:.2f})"
                drawvisuals(frame, x1, y1, x2, y2, text)
        #pass coordinate to Arduino
        if checkforObject(objects, 1) and not checkforObject(objects, 2):
            #get coordinates of the center of the cup
            xmid = int(objects[0].x1) + 0.5*(objects[0].x2-objects[0].x1)
            ymid = int(objects[0].y1) + 0.5*(objects[0].y2-objects[0].y1)
            passtoSerial(xmid, ymid)
            waitingforUNO = True
    else:
        if checkresponse():
            waitingforUNO = False
            objects.remove(objects[0])

    #show pre-processed frame
    cv.imshow('Livebild', frame)
    
    #go through all backlog frames
    for _ in range(int(30*runtime)):
        ret, frame = cap.read()
    runtime = 0

    if cv.waitKey(10) & 0xFF == ord('q'): 
        break

cap.release()
cv.destroyAllWindows()