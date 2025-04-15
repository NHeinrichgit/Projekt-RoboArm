import cv2 as cv
from screentext import ScreenText
from fpstracker import FPSTracker

# Kamera öffnen (0 für die erste erkannte Kamera)
cap = cv.VideoCapture(0)

if not cap.isOpened():
    print("Error: capture not open")

fps_text = ScreenText()
fpstracker = FPSTracker()

while True:
    ret, frame = cap.read() #ret is a bool for checking output

    if not ret:
        print("Error: no frame")
        break
    
    #actually pre-process the image
    frame = cv.resize(frame, (640, 480))#make sure every frame has the same size, otherwise dependent on camera
    frame = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)#apply grayscale, color not needed for shape detection, reduces size
    frame = cv.GaussianBlur(frame, (5, 5), 0)#apply gaussian blur to reduce noise
    frame = cv.Canny(frame, 50, 150)#detect edges using canny

    #implement frames-per-second counter
    fps = fpstracker.updateFPS()
    fps_text.set_Text(f"FPS: {fps:.0f}")
    fps_text.set_Position((10,20))
    fps_text.set_Color((255,255,255))
    fps_frame = fps_text.showText(frame)
    
    #implement MLM
    
    #if cup is found, get cup coordinates
    
    #pass coordinate to Arduino
    
    #show pre-processed frame
    cv.imshow('Livebild', fps_frame)
    
    if cv.waitKey(10) & 0xFF == ord('q'): 
        break

cap.release()
cv.destroyAllWindows()