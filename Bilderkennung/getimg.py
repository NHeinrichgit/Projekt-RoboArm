import cv2 as cv
import time

#manage framerate
target_fps = 10
frame_interval = 1.0 / target_fps
last_time = time.time()

# Kamera öffnen (0 für die erste erkannte Kamera)
cap = cv.VideoCapture(0)

if not cap.isOpened():
    print("Error: capture not open")

while True:
    ret, frame = cap.read() #ret is a bool for checking output

    if not ret:
        print("Error: no frame")
        break

    text = "fps: " + str(target_fps)
    position = (10, 20)  # Position for the text (x, y)
    font = cv.FONT_HERSHEY_SIMPLEX  # Font type
    font_scale = 0.7  # Font size
    color = (0, 255, 0)  # Green text (in BGR format)
    thickness = 1  # Thickness of the text
    line_type = cv.LINE_AA  # Anti-aliased line type

    current_time = time.time()
    if current_time > last_time + frame_interval:
        cv.putText(frame, text, position, font, font_scale, color, thickness, line_type)
        cv.imshow('Livebild', frame)
        last_time = time.time()

    #waitkey checks for key press and returns key id
    #&0xFF only takes last two bytes
    #ord(q) gives ASCII value of q
    if cv.waitKey(10) & 0xFF == ord('q'): 
        break

cap.release()
cv.destroyAllWindows()
