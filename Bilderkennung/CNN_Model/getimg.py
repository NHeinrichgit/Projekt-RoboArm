import os
import cv2 as cv
#from screentext import ScreenText
#from fpstracker import FPSTracker
import inference as RCNN
# Kamera öffnen (0 für die erste erkannte Kamera)
cap = cv.VideoCapture(0)

if not cap.isOpened():
    print("Error: capture not open")

#fps_text = ScreenText()
#fpstracker = FPSTracker()

imagecounter = 0

model = RCNN.initmodel()

while True:
    ret, frame = cap.read() #ret is a bool for checking output

    if not ret:
        print("Error: no frame")
        break
    
    #actually pre-process the image
    frame = cv.resize(frame, (640, 480))#make sure every frame has the same size, otherwise dependent on camera
    #frame = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)#apply grayscale, color not needed for shape detection, reduces size
    
    imagecounter += 1
    #implement frames-per-second counter
    """ fps = fpstracker.updateFPS()
    fps_text.set_Text(f"FPS: {fps:.0f}")
    fps_text.set_Position((10,20))
    fps_text.set_Color((255,255,255))
    fps_frame = fps_text.showText(frame) """
    
    #implement MLM
    if(imagecounter%100):
        output = RCNN.checkimg(model, frame)
        print(output)
        prediction = output[0]
        score_threshold = 0.9
        #if cup is found, get cup coordinates
        for box, score, label in zip(prediction['boxes'], prediction['labels'], prediction['scores']):
            if score.item()>=score_threshold:
                x1, y1, x2, y2 = box.int().tolist()
                cv.rectangle(frame, (x1,y1), (x2,y2), (0,255,0), 2)
                text = f"{label.item()} ({score:.2f})"
                cv.putText(frame, text, (x1,y1-10), cv.FONT_HERSHEY_SIMPLEX,
                        0.5, (0, 255, 0), 1, cv.LINE_AA)
    #pass coordinate to Arduino
    
    #show pre-processed frame
    
    cv.imshow('Livebild', frame)
    
    if cv.waitKey(10) & 0xFF == ord('q'): 
        break

cap.release()
cv.destroyAllWindows()